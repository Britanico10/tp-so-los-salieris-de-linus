#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "codigo_operacion.h"
#include "mensajes.h"

#define PUERTO 5001

typedef struct pcb_struct {
	unsigned pid;					//Identificador único del Programa en el sistema
	unsigned segmento_codigo;		//Dirección del primer byte en la UMV del segmento de código
	unsigned segmento_stack;		//Dirección del primer byte en la UMV del segmento de stack
	unsigned pointer_stack;			//Dirección del primer byte en la UMV del Contexto de Ejecución Actual
	unsigned indice_codigo;			//Dirección del primer byte en la UMV del Índice de Código
	unsigned indice_etiquetas;		//Dirección del primer byte en la UMV del Índice de Etiquetas
	unsigned pc;					//Número de la próxima instrucción a ejecutar
	unsigned tam_contexto_actual;	//Cantidad de variables (locales y parámetros) del Contexto de Ejecución Actual
	unsigned tam_indice_etiquetas;	//Cantidad de bytes que ocupa el Índice de etiquetas
} t_PCB;

t_PCB *pcbActual;

char * leer_linea(void) {
  char * linea = malloc(1);
  unsigned longitud = 0;
  int lectura;
  for (lectura = getchar(); (lectura != EOF) && (lectura != '\n'); lectura = getchar()) {
    linea = realloc(linea,longitud+2);
    linea[longitud++] = lectura;
  }
  linea[longitud] = 0;
  return linea;
}

void armarPCB(mensaje_t mensaje){
	pcbActual->pid = mensaje.parametros[0];
	//parametros: pid,segmento_codigo,segmento_stack,stack_pointer,indice_codigo,indice_etiqueta,pc,tam_contexto_actual,tam_indice_etiquetas
	pcbActual->segmento_codigo = mensaje.parametros[1];
	pcbActual->segmento_stack = mensaje.parametros[2];
	pcbActual->pointer_stack = mensaje.parametros[3];
	pcbActual->indice_codigo = mensaje.parametros[4];
	pcbActual->indice_etiquetas = mensaje.parametros[5];
	pcbActual->pc = mensaje.parametros[6];
	pcbActual->tam_contexto_actual = mensaje.parametros[7];
	pcbActual->tam_indice_etiquetas = mensaje.parametros[8];
}

unsigned *desarmarPCB(){
	unsigned *PCBdesarmada = malloc(sizeof(unsigned)*9);
	PCBdesarmada[0] = pcbActual->pid;
	PCBdesarmada[1] = pcbActual->segmento_codigo;
	PCBdesarmada[2] = pcbActual->segmento_stack;
	PCBdesarmada[3] = pcbActual->pointer_stack +1;
	PCBdesarmada[4] = pcbActual->indice_codigo;
	PCBdesarmada[5] = pcbActual->indice_etiquetas;
	PCBdesarmada[6] = pcbActual->pc +5;
	PCBdesarmada[7] = pcbActual->tam_contexto_actual +4;
	PCBdesarmada[8] = pcbActual->tam_indice_etiquetas;
	return PCBdesarmada;
}

void mostrar_mensaje_recibido (mensaje_t mensaje) {
  puts("------------NUEVO MENSAJE--------------------------");
  printf("Codigo     : %10u\nParametros : %10u\nDatos extra: %10u\n", mensaje.codigo, mensaje.cantidad, mensaje.tam_extra);
  unsigned parametro;
  for (parametro = 0; parametro < mensaje.cantidad; parametro ++)
    printf("Par#%7u: %10u\n", parametro + 1, mensaje.parametros[parametro]);
  if (mensaje.tam_extra > 0){
	  printf("Contenido  : %s\n", mensaje.extra);
  }

  puts("-------------FIN NUEVO MENSAJE---------------------");
  putchar('\n');
}

void menuObtenerValor(int ns,mensaje_t mensaje_enviado) {
  //el kernel necesita hacer cambio de proceso para indicar el PID antes de escribir?
  char* buffer;
  unsigned estado;
  mensaje_t mensaje_recibido;
  printf("Ingrese nombre de variable: ");
  buffer = leer_linea();

  mensaje_enviado.codigo = SYS_OBTENER_VALOR;
  mensaje_enviado.cantidad = 0;
  mensaje_enviado.tam_extra = strlen(buffer)+1;//aca estamos probando con string con estructura seria distinto
  mensaje_enviado.parametros = NULL;
  mensaje_enviado.extra = buffer;
  enviar_mensaje(ns, mensaje_enviado);
  destruir_mensaje(&mensaje_enviado);
  if ((estado = recibir_mensaje(ns, &mensaje_recibido)) > 0)
    mostrar_mensaje_recibido(mensaje_recibido);
}

void menuGrabarValor(int ns,mensaje_t mensaje_enviado){
  //el kernel necesita hacer cambio de proceso para indicar el PID antes de escribir?
  int valor;
  char* buffer;
  unsigned estado;
  mensaje_t mensaje_recibido;
  mensaje_enviado.parametros = malloc(sizeof(int));
  printf("Ingrese nombre de valor: ");
  buffer = leer_linea();
  printf("Ingrese valor: ");
  scanf("%d", &valor); while (getchar() != '\n');
  
  mensaje_enviado.codigo = SYS_ASIGNAR_VALOR;
  mensaje_enviado.cantidad = 1;
  mensaje_enviado.tam_extra = strlen(buffer)+1;//aca estamos probando con string con estructura seria distinto
  mensaje_enviado.extra = strdup(buffer);
  mensaje_enviado.parametros[0] = valor;
  enviar_mensaje(ns, mensaje_enviado);
  destruir_mensaje(&mensaje_enviado);
  if ((estado = recibir_mensaje(ns, &mensaje_recibido)) > 0)
    mostrar_mensaje_recibido(mensaje_recibido);
}

void menuWait(int ns,mensaje_t mensaje_enviado){
  char* buffer;
  unsigned estado;
  mensaje_t mensaje_recibido;
  printf("Ingrese nombre del semaforo: ");
  buffer = leer_linea();

  mensaje_enviado.codigo = SYS_WAIT;
  mensaje_enviado.cantidad = 0;
  mensaje_enviado.tam_extra = strlen(buffer)+1;//aca estamos probando con string con estructura seria distinto
  mensaje_enviado.parametros = NULL;
  mensaje_enviado.extra = buffer;
  enviar_mensaje(ns, mensaje_enviado);
  destruir_mensaje(&mensaje_enviado);
}

void menuSignal(int ns,mensaje_t mensaje_enviado){
  char* buffer;
  unsigned estado;
  mensaje_t mensaje_recibido;
  printf("Ingrese nombre del semaforo: ");
  buffer = leer_linea();

  mensaje_enviado.codigo = SYS_SIGNAL;
  mensaje_enviado.cantidad = 0;
  mensaje_enviado.tam_extra = strlen(buffer)+1;//aca estamos probando con string con estructura seria distinto
  mensaje_enviado.parametros = NULL;
  mensaje_enviado.extra = buffer;
  enviar_mensaje(ns, mensaje_enviado);
  destruir_mensaje(&mensaje_enviado);
}

void menuEntradaSalida(int ns,mensaje_t mensaje_enviado){
  char * dispositivo;
  unsigned estado,duracion;
  mensaje_t mensaje_recibido;
  printf("Ingrese dispositivo: ");
  dispositivo = leer_linea();
  printf("Ingrese duracion: ");
  scanf("%d", &duracion); while (getchar() != '\n');

  mensaje_enviado.codigo = SYS_IO;
  mensaje_enviado.cantidad = 1;
  mensaje_enviado.tam_extra = strlen(dispositivo)+1;
  mensaje_enviado.extra = dispositivo;
  mensaje_enviado.parametros = &duracion;
  enviar_mensaje(ns, mensaje_enviado);
  //destruir_mensaje(&mensaje_enviado);
}

void menuImprimir(int ns,mensaje_t mensaje_enviado){
  unsigned numero;
  unsigned estado;
  mensaje_t mensaje_recibido;
  printf("Ingrese un numero: ");
  scanf("%d", &numero); while (getchar() != '\n');

  mensaje_enviado.codigo = SYS_IMPR;
  mensaje_enviado.cantidad = 1;
  mensaje_enviado.tam_extra = 0;
  mensaje_enviado.parametros = &numero;
  mensaje_enviado.extra = NULL;
  enviar_mensaje(ns, mensaje_enviado);
  //destruir_mensaje(&mensaje_enviado);
}

void menuImprimirTexto(int ns,mensaje_t mensaje_enviado){
  char * buffer;
  unsigned estado;
  mensaje_t mensaje_recibido;
  printf("Ingrese mensaje: ");
  buffer = leer_linea();

  mensaje_enviado.codigo = SYS_IMPR_TEXTO;
  mensaje_enviado.cantidad = 0;
  mensaje_enviado.tam_extra = strlen(buffer)+1;
  mensaje_enviado.parametros = NULL;
  mensaje_enviado.extra = buffer;
  enviar_mensaje(ns, mensaje_enviado);
  //destruir_mensaje(&mensaje_enviado);
}

void menuFinInstruccion(int ns,mensaje_t mensaje_enviado){
  unsigned estado;
  //deberia armar el PCB!
  mensaje_t mensaje_recibido;
  mensaje_enviado.codigo = FIN_INSTRUCCION;
  mensaje_enviado.cantidad = 9;
  mensaje_enviado.tam_extra = 0;
  mensaje_enviado.extra = NULL;
  mensaje_enviado.parametros = desarmarPCB();
  enviar_mensaje(ns, mensaje_enviado);
  //destruir_mensaje(&mensaje_enviado);
  if ((estado = recibir_mensaje(ns, &mensaje_recibido)) > 0) {
      if (mensaje_recibido.codigo == EJECUTAR_INSTRUCCION){
      	armarPCB(mensaje_recibido);
      	mostrar_mensaje_recibido(mensaje_recibido);
      }
      else puts("Me llego un mensaje que no entiendo!");
  }
}

void menuFinalizar(int ns,mensaje_t mensaje_enviado){
  unsigned estado;
  mensaje_t mensaje_recibido;
  char *texto = strdup("Estado de Variables: caca.");
  mensaje_enviado.codigo = FIN_PROGRAMA;
  mensaje_enviado.cantidad = 9;
  mensaje_enviado.tam_extra = strlen(texto);
  mensaje_enviado.extra = strdup(texto);
  mensaje_enviado.parametros = desarmarPCB();
  enviar_mensaje(ns, mensaje_enviado);
  if ((estado = recibir_mensaje(ns, &mensaje_recibido)) > 0) {
	  if (mensaje_recibido.codigo == EJECUTAR_INSTRUCCION){
		  armarPCB(mensaje_recibido);
		  mostrar_mensaje_recibido(mensaje_recibido);
      }
	  else puts("Me llego un mensaje que no entiendo!");
  }
  //destruir_mensaje(&mensaje_enviado);
}



int main (int argc, char ** argv) {
  mensaje_t mensaje_enviado,mensaje_recibido;
  int estado;
  unsigned option;
  /*if (argc != 2) {
    fprintf(stderr, "Introducir el numero de puerto por parametro\n");
    return 1;
  }
  unsigned short puerto;
  if (sscanf(argv[1], "%hu", &puerto) != 1) {
    fprintf(stderr, "Numero de puerto incorrecto\n");
    return 1;
  }*/
  int ns = socket(AF_INET, SOCK_STREAM, 0);
  if (ns < 0) {
    perror("socket");
    return 1;
  }
  struct sockaddr_in sa;
  sa.sin_family = AF_INET;
  sa.sin_port = htons(PUERTO);
  inet_aton("127.0.0.1", &sa.sin_addr);
  if (connect(ns, (const void *) &sa, sizeof sa) < 0) {
    perror("connect");
    close(ns);
    return 1;
  }

  pcbActual = malloc(sizeof(t_PCB));
  mensaje_enviado.codigo = HANDSHAKE_CPU;
  mensaje_enviado.cantidad = 0;
  mensaje_enviado.tam_extra = 0;
  mensaje_enviado.extra = NULL;
  mensaje_enviado.parametros = NULL;
  enviar_mensaje(ns, mensaje_enviado);
  puts("Handshake enviado.");

  if ((estado = recibir_mensaje(ns, &mensaje_recibido)) > 0) {
    if (mensaje_recibido.codigo == EJECUTAR_INSTRUCCION){
    	armarPCB(mensaje_recibido);
    	mostrar_mensaje_recibido(mensaje_recibido);
    }
    else puts("Me llego un mensaje que no entiendo!");
  }

  while (1) {
    puts("**************************");
    puts("1.\tobtener valor");
    puts("2.\tgrabar valor");
    puts("3.\twait");
    puts("4.\tsignal");
    puts("5.\tentrada/salida");
    puts("6.\timprimir");
    puts("7.\timprimirTexto");
    puts("8.\tfin de instruccion");
    puts("9.\tfinalizar programa");
    puts("10.\tcerrar conexion");
    puts("11.\tmostrar ultimo mensaje recibido");
    puts("**************************");
    printf("Ingrese numero de operacion: ");
    scanf("%d",&option);
    while (getchar() != '\n');
    switch (option) {
      case 1: menuObtenerValor(ns,mensaje_enviado); break;
      case 2: menuGrabarValor(ns,mensaje_enviado); break;
      case 3: menuWait(ns,mensaje_enviado); break;
      case 4: menuSignal(ns,mensaje_enviado); break;
      case 5: menuEntradaSalida(ns,mensaje_enviado); break;
      case 6: menuImprimir(ns,mensaje_enviado); break;
      case 7: menuImprimirTexto(ns,mensaje_enviado); break;
      case 8: menuFinInstruccion(ns,mensaje_enviado); break;
      case 9: menuFinalizar(ns,mensaje_enviado); break;
      case 10: puts("Cerrar conexion!"); close(ns); exit(0); break;
      case 11: mostrar_mensaje_recibido(mensaje_recibido); break;
    }
  }
}
