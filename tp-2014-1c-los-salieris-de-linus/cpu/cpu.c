#include "cpu.h"
#include <parser/metadata_program.h>

int notificacion_signal;
int retardoCPU;

int conectarse(const char * ip, int puerto) {
  struct sockaddr_in sa;
  sa.sin_family = AF_INET;
  sa.sin_port = htons(puerto);
  if (!inet_aton(ip, &sa.sin_addr)) return -1;
  int ns = socket(AF_INET, SOCK_STREAM, 0);
  if (socket < 0) return -1;
  if (connect(ns, (const void *) &sa, sizeof sa) < 0) {
    perror("connect");
    close(ns);
    return -1;
  }
  return ns;
}

void mandar_mensaje_finalizacion (pcb PCB) {
  char * variables = malloc(2);
  unsigned longitud = 0;
  *variables = 0;
  const char * var_actual = nombresDiccionario;
  const unsigned * ptr_actual = punterosDiccionario;
  char valor[25];
  int primera = 1;
  unsigned valor_variable;
  while (*var_actual) {
    valor_variable = pr_obtener_valor(*ptr_actual);
    sprintf(valor, "%s%c = %u", primera ? "" : ", ", *var_actual, valor_variable);
    primera = 0;
    longitud += strlen(valor);
    variables = realloc(variables, longitud + 2);
    strcat(variables, valor);
    var_actual ++;
    ptr_actual ++;
  }
  strcat(variables, "\n");
  mensaje_t mensaje = {.codigo = FIN_PROGRAMA, .cantidad = 0, .parametros = NULL, .extra = variables, .tam_extra = longitud + 2};
  enviar_mensaje(socketKernel, mensaje);
  free(variables);
}

void mandar_segmentation_fault (){
	mensaje_t mensaje = {.codigo = SEGMENTATION_FAULT, .cantidad = 0, .parametros = NULL, .extra = NULL, .tam_extra = 0};
	enviar_mensaje(socketKernel, mensaje);
}

pcb recuperar_pcb (mensaje_t mensaje) {
  return (pcb) {
    .pid = mensaje.parametros[0],
    .segmento_codigo = mensaje.parametros[1],
    .segmento_stack = mensaje.parametros[2],
    .pointer_stack = mensaje.parametros[3],
    .indice_codigo = mensaje.parametros[4],
    .indice_etiquetas = mensaje.parametros[5],
    .pc = mensaje.parametros[6],
    .tam_contexto_actual = mensaje.parametros[7],
    .tam_indice_etiquetas = mensaje.parametros[8]
  };
}

int construirDiccionario (pcb PCB) {
  if (!PCB.tam_contexto_actual) {
    nombresDiccionario = malloc(1);
    *nombresDiccionario = 0;
    punterosDiccionario = NULL;
    return 1;
  }
  char * datosPila = malloc(PCB.tam_contexto_actual * 5);
  int estado = leer_datos(PCB.pointer_stack, 0, PCB.tam_contexto_actual * 5, datosPila);
  if (estado <= 0) return estado;
  nombresDiccionario = malloc(PCB.tam_contexto_actual + 1);
  punterosDiccionario = malloc(sizeof(unsigned) * PCB.tam_contexto_actual);
  unsigned posicion;
  for (posicion = 0; posicion < PCB.tam_contexto_actual; posicion ++) {
    nombresDiccionario[posicion] = datosPila[5 * posicion];
    punterosDiccionario[posicion] = PCB.pointer_stack + 5 * posicion + 1; //desempaquetar_32(datosPila + 5 * posicion + 1);
  }
  nombresDiccionario[PCB.tam_contexto_actual] = 0;
  return PCB.tam_contexto_actual;
}

void destruirDiccionario (void) {
  free(nombresDiccionario);
  free(punterosDiccionario);
}

char * obtener_siguiente_instruccion (pcb PCB) {
  char datos_posicionales[8];
  mensaje_t mensaje;
  mensaje.codigo = LEER_SEGMENTO;
  mensaje.parametros = malloc(3 * sizeof(unsigned));
  mensaje.parametros[0] = PCB.indice_codigo;
  mensaje.parametros[1] = 8 * PCB.pc;
  mensaje.parametros[2] = 8;
  mensaje.cantidad = 3;
  mensaje.tam_extra = 0;
  mensaje.extra = NULL;
  enviar_mensaje(socketUMV, mensaje);
  destruir_mensaje(&mensaje);
  int estado = recibir_mensaje(socketUMV, &mensaje);
  if (!estado) exit(1); //desconecto UMV
  switch (mensaje.codigo) {
    case SEGMENTATION_FAULT:
      destruir_mensaje(&mensaje);
      perror("Segmentation fault al intentar obtener la siguiente instruccion");
      exit(0);
      break;
    case DATOS_SEGMENTO:
      memcpy(datos_posicionales, mensaje.extra, mensaje.tam_extra);
      destruir_mensaje(&mensaje);
      break;
    default:
      destruir_mensaje(&mensaje);
      handshake(socketUMV, 0);
      perror("Recibe un mensaje desconocido por parte de la UMV al solicitar siguiente instruccion");
      exit(0);
      break;
  }
  
  t_intructions *t_indice=(t_intructions*)datos_posicionales;
  unsigned inicio, longitud;
  inicio = t_indice->start;
  longitud = t_indice->offset;

  char * instruccion = malloc(longitud + 1);
  instruccion[longitud] = 0;
  estado = leer_datos(PCB.segmento_codigo, inicio, longitud, instruccion);
  if (estado <= 0) {
    free(instruccion);
    return NULL;
  }
  return instruccion;
}

void mandar_PCB_actualizado (void) {
  mensaje_t mensaje;
  unsigned parametros[10];
  mensaje.codigo = FIN_INSTRUCCION;
  mensaje.cantidad = 10;
  mensaje.tam_extra = 0;
  mensaje.extra = NULL;
  mensaje.parametros = parametros;
  parametros[0] = PCB_actual.pid;
  parametros[1] = PCB_actual.segmento_codigo;
  parametros[2] = PCB_actual.segmento_stack;
  parametros[3] = PCB_actual.pointer_stack;
  parametros[4] = PCB_actual.indice_codigo;
  parametros[5] = PCB_actual.indice_etiquetas;
  parametros[6] = PCB_actual.pc;
  parametros[7] = PCB_actual.tam_contexto_actual;
  parametros[8] = PCB_actual.tam_indice_etiquetas;
  parametros[9] = notificacion_signal;
  enviar_mensaje(socketKernel, mensaje);
}

void enviar_mensaje_codificado(int socket_cliente, unsigned codigo, int cantidad, int tamanio_extra, char* extra, unsigned * parametros){
  mensaje_t mensaje;
  mensaje.codigo = codigo;
  mensaje.cantidad = cantidad;
  mensaje.tam_extra = tamanio_extra;
  mensaje.extra = extra;
  mensaje.parametros = parametros;
  enviar_mensaje(socket_cliente, mensaje);
}

void handshake(int socket_cliente, unsigned codigo){
  enviar_mensaje_codificado(socket_cliente, codigo, 0, 0, NULL, NULL);
}

void signal_handler(int signal){
  printf("Recibida señal SIGUSR1.\n");
  notificacion_signal = 1;
}

int recibir_cambio_proceso_ok(int socketUMV) {
  mensaje_t msj;
  int rta;
  if ((rta = recibir_mensaje(socketUMV,&msj)) && (msj.codigo == CAMBIO_PROCESO_OK)) return 1;
  else return -1;
}

int main(int argc, char **argv) {
  int estado;
  mensaje_t mensaje_recibido;
  char * IP_UMV = 0;
  char * IP_KERNEL = 0;
  int PUERTO_UMV;
  int PUERTO_KERNEL;

  t_config *conf = config_create(FCONFIG);
  IP_UMV = strdup(config_get_string_value(conf, "IP_UMV"));
  IP_KERNEL = strdup(config_get_string_value(conf, "IP_KERNEL"));
  PUERTO_UMV = config_get_int_value(conf, "PUERTO_UMV");
  PUERTO_KERNEL = config_get_int_value(conf, "PUERTO_KERNEL");
  config_destroy(conf);
  signal(SIGUSR1, signal_handler);
  
  if ((socketKernel = conectarse(IP_KERNEL, PUERTO_KERNEL)) == -1) {
    perror("conexion kernel");
    return 1;
  }
  if ((socketUMV = conectarse(IP_UMV, PUERTO_UMV)) == -1) {
    perror("conexion umv");
    return 1;
  }

  handshake(socketUMV, HANDSHAKE_CPU);
  handshake(socketKernel, HANDSHAKE_CPU);
  mensaje_t respuesta;
  recibir_mensaje(socketKernel, &respuesta);
  if (respuesta.codigo == HANDSHAKE_CPU_RET){
	  retardoCPU = respuesta.parametros[0];
  }
  destruir_mensaje(&respuesta);
  
  while (((estado = recibir_mensaje(socketKernel, &mensaje_recibido)) > 0) || (notificacion_signal)) {
    if (mensaje_recibido.codigo != EJECUTAR_INSTRUCCION) { 
      //mensajes que debe rechazar!
      enviar_mensaje_codificado(socketKernel, 0, 0, 0, NULL, NULL);
      destruir_mensaje(&mensaje_recibido);
      continue;
    }
    if ((mensaje_recibido.cantidad != 9) || mensaje_recibido.tam_extra) {
      enviar_mensaje_codificado(socketKernel, 0, 0, 0, NULL, NULL);
      destruir_mensaje(&mensaje_recibido);
      continue;
    }
    PCB_actual = recuperar_pcb(mensaje_recibido);
    estado_programa = 0;
    destruir_mensaje(&mensaje_recibido);
    //enviar_mensaje_codificado(socketKernel, 0, 0, 0, NULL, NULL);
    mensaje_recibido.codigo = CAMBIO_PROCESO;
    mensaje_recibido.cantidad = 1;
    mensaje_recibido.tam_extra = 0;
    mensaje_recibido.extra = NULL;
    mensaje_recibido.parametros = &PCB_actual.pid;
    enviar_mensaje(socketUMV, mensaje_recibido);
    //destruir_mensaje(&mensaje_recibido);
    recibir_cambio_proceso_ok(socketUMV);
    /*if (recibir_cambio_proceso_ok(socketUMV) > 0) {

    } //sino fallo el cambio de proceso!*/

    estado = construirDiccionario(PCB_actual);
    if (estado <= 0)
      estado_programa = 2; //segmentation fault
    else {
      /*mensaje_recibido.codigo = CAMBIO_PROCESO;
      mensaje_recibido.cantidad = 1;
      mensaje_recibido.tam_extra = 0;
      mensaje_recibido.extra = NULL;
      mensaje_recibido.parametros = &PCB_actual.pid;
      enviar_mensaje(socketUMV, mensaje_recibido);
      //destruir_mensaje(&mensaje_recibido);
      if (recibir_cambio_proceso_ok(socketUMV) > 0) {
        char * instruccion = obtener_siguiente_instruccion(PCB_actual);
        PCB_actual.pc ++;
        printf("Ejecutando instruccion #%d: %s\n", PCB_actual.pc-1, instruccion);
        analizadorLinea(instruccion, &funciones, &funciones_kernel);
        free(instruccion);
      } //sino fallo el cambio de proceso!*/
    	char * instruccion = obtener_siguiente_instruccion(PCB_actual);
    	PCB_actual.pc ++;
    	printf("Ejecutando instruccion #%d: %s\n", PCB_actual.pc-1, instruccion);
    	usleep(retardoCPU*1000);
    	analizadorLinea(instruccion, &funciones, &funciones_kernel);
    	free(instruccion);
    }
    switch (estado_programa) {
      case 0:
        printf("Finalizo la ejecucion de la instruccion con normalidad, envio PCB actualizado.\n");
        mandar_PCB_actualizado();
        break;
      case 1:
        mandar_mensaje_finalizacion(PCB_actual);
        break;
      case 2:
    	mandar_segmentation_fault();
    	break;
      case 3:
    	mandar_segmentation_fault();
    	break;
      default:
    	break;
    }
    destruirDiccionario();
  	  if(notificacion_signal==1){
  	        /*TODO: Me deconecto de todo y finalizo*/
  	        close(socketUMV);
  	        close(socketKernel);
  	        exit(0);
  	      }
  }
  return EXIT_SUCCESS;
}
