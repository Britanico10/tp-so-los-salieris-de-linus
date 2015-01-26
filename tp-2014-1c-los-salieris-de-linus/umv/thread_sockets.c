#include "umv.h"

extern nodo_segmento *lsu;
extern int (* obtener_bloque) (nodo_segmento *);
extern int TOPE;
extern int RETARDO;
extern int PUERTO;
extern char *umv_dmr;
extern t_log *archivo_log;
extern pthread_rwlock_t semaforo_lsu;

int ncpu = 0;

int crear_socket_listen (void) {
  int optval = 1;
  int socketListen = socket(AF_INET, SOCK_STREAM, 0);
	if (socketListen < 0) return -1;
  setsockopt(socketListen, SOL_SOCKET, SO_REUSEADDR, &optval,sizeof(optval));
	struct sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(PUERTO);
	sa.sin_addr.s_addr = INADDR_ANY;
	if (bind(socketListen, (const void *) &sa, sizeof sa) < 0) {
		close(socketListen);
		return -1;
	}
	if (listen(socketListen, SOMAXCONN) < 0) {
		close(socketListen);
		return -1;
	}
	return socketListen;
}

void aceptar_conexion (int socketListen) {
  pthread_t thread;
  int *newSocket = malloc(sizeof(int)), estado;
  *newSocket = accept(socketListen, NULL, 0);
  mensaje_t mensaje_actual;
  estado = recibir_mensaje(*newSocket,&mensaje_actual);
  if (estado>0) {
    switch (mensaje_actual.codigo) {
      case HANDSHAKE_KERNEL:
        log_info(archivo_log,"Kernel conectado");
        pthread_create(&thread, NULL, thread_kernel, newSocket);
      break;
      case HANDSHAKE_CPU:
        ncpu++;
        log_info(archivo_log,"CPU %d conectada",ncpu);
        pthread_create(&thread, NULL, thread_cpu, newSocket);
      break;
      default: //si alguien mas trata de hacer handshake le tiramos un error!
        enviar_mensaje(*newSocket, (mensaje_t) {.codigo = 0, .cantidad = 0, .tam_extra = 0, .parametros = NULL, .extra = NULL});
        destruir_mensaje(&mensaje_actual);
        break;
    }
  }
  pthread_detach(thread);
}

void enviar_respuesta (int *socket, unsigned codigo_operacion) {
  enviar_mensaje(*socket, (mensaje_t) {
    .codigo = codigo_operacion,
    .cantidad = 0,
    .tam_extra = 0,
    .parametros = NULL,
    .extra = NULL,
  });
}

void *thread_kernel (void *newSocket) {
  int *nSocket = newSocket;
  mensaje_t mensaje_recibido,mensaje_repuesta;
  int estado;
  unsigned pid, tamanio, base, offset;

  while ((estado = recibir_mensaje(*nSocket, &mensaje_recibido)) >= 0) {
    switch (mensaje_recibido.codigo) {
      case CAMBIO_PROCESO: //recibe pid
        pid = mensaje_recibido.parametros[0];
        log_info(archivo_log,"HILO KERNEL. Cambio de proceso(pid = %d)",pid);
        destruir_mensaje(&mensaje_recibido);
        usleep(RETARDO*1000);
        enviar_respuesta(nSocket,CAMBIO_PROCESO_OK);
      break;
    	case CREAR_SEGMENTO: //recibe: pid, tamanio -> devuelve base
        pid = mensaje_recibido.parametros[0];
        tamanio = mensaje_recibido.parametros[1];
        log_info(archivo_log,"HILO KERNEL: Intento por crear segmento(pid = %d, tamanio = %d)",pid,tamanio);
        destruir_mensaje(&mensaje_recibido);
        if ((base = reservarSegmento(pid,tamanio)) == ((unsigned) -1)) { //Hay memory overload!
          usleep(RETARDO*1000);
          log_info(archivo_log,"HILO KERNEL. Memory overload al intentar crear segmento");
          enviar_respuesta(nSocket, MEMORY_OVERLOAD);
        } else { //se creo el segmento con exito!!
          mensaje_repuesta.codigo = CREAR_SEGMENTO_OK;
          mensaje_repuesta.cantidad = 1;
          mensaje_repuesta.tam_extra = 0;
          mensaje_repuesta.parametros = &base;
          mensaje_repuesta.extra = NULL;
          usleep(RETARDO*1000);
          log_info(archivo_log,"HILO KERNEL. Nuevo segmento: base otorgada = %d",base);
          enviar_mensaje(*nSocket,mensaje_repuesta);
          //destruir_mensaje(&mensaje_repuesta);
        }
      break;
    	case ESCRIBIR_SEGMENTO: //recibe base, offset, contenido
        base = mensaje_recibido.parametros[0];
        offset = mensaje_recibido.parametros[1];
        tamanio = mensaje_recibido.tam_extra;
        char *buffer = malloc(mensaje_recibido.tam_extra);
        memcpy(buffer,mensaje_recibido.extra,mensaje_recibido.tam_extra);
        log_info(archivo_log,"HILO KERNEL. Intento por escribir segmento(base = %d,offset = %d, tamanio = %d)",base,offset,tamanio);
        log_info(archivo_log,"Escribir segmento: contenido: %s",buffer);
        //ojo que tambien debe poder escribir estructuras!
        destruir_mensaje(&mensaje_recibido);
        usleep(RETARDO*1000);
        if ((estado = escribirSegmento(pid, base+offset, tamanio, buffer))<0) { //segmetation fault
          log_info(archivo_log,"HILO KERNEL. Segmentation fault al intentar escribir segmento");
          enviar_respuesta(nSocket, SEGMENTATION_FAULT);
        }
        else { //salio todo bien!
          log_info(archivo_log,"HILO KERNEL. Segmento escrito correctamente");
          enviar_respuesta(nSocket, ESCRIBIR_SEGMENTO_OK);
        }
        free(buffer);
    	break;
      case DESTRUIR_SEGMENTOS: //recibe pid
        pid = mensaje_recibido.parametros[0];
        borrarPrograma(pid);
        destruir_mensaje(&mensaje_recibido);
        usleep(RETARDO*1000);
        enviar_respuesta(nSocket, DESTRUIR_SEGMENTOS_OK);
      break;
      default:
        usleep(RETARDO*1000);
        enviar_mensaje(*nSocket, (mensaje_t) {.codigo = 0, .cantidad = 0, .tam_extra = 0, .parametros = NULL, .extra = NULL});
        destruir_mensaje(&mensaje_recibido);
      break;
    }
  }
  close(*nSocket);
  return NULL;
}

void *thread_cpu (void *newSocket) {
  int *nSocket = newSocket;
  mensaje_t mensaje_recibido,mensaje_repuesta;
  int estado, pid, base, offset, tamanio;
  char *contenido;
  contenido = 0;
  log_info(archivo_log,"\nCPU %d conectado\n", ncpu);

  while ((estado = recibir_mensaje(*nSocket, &mensaje_recibido)) >= 0) {
    
    switch (mensaje_recibido.codigo) {
      case CAMBIO_PROCESO: //recibe pid
        pid = mensaje_recibido.parametros[0];
        destruir_mensaje(&mensaje_recibido);
        usleep(RETARDO*1000);
        enviar_respuesta(nSocket,CAMBIO_PROCESO_OK);
        log_info(archivo_log,"Cambio al proceso %d",pid);
      break;
      case LEER_SEGMENTO: //recibe base, desplazamiento, tamanio (errores: segmetation fault)
        base = mensaje_recibido.parametros[0];
        offset = mensaje_recibido.parametros[1];
        tamanio = mensaje_recibido.parametros[2];
        contenido = leerSegmento(pid,base+offset,tamanio);
        if (contenido == NULL) {
          usleep(RETARDO*1000);
          enviar_respuesta(nSocket, SEGMENTATION_FAULT);
          free(contenido);
          log_info(archivo_log,"Hilo CPU. Segmentation Fault al intentar leer PID %d, BASE %d, TAMANIO %d", pid,base,tamanio);
        }
        else { //salio bien!
          mensaje_repuesta.codigo = DATOS_SEGMENTO;
          mensaje_repuesta.cantidad = 0;
          mensaje_repuesta.tam_extra = tamanio;
          mensaje_repuesta.parametros = NULL;
          mensaje_repuesta.extra = contenido;
          usleep(RETARDO*1000);
          enviar_mensaje(*nSocket, mensaje_repuesta);
          destruir_mensaje(&mensaje_repuesta);
        }
      break;
      case ESCRIBIR_SEGMENTO: //recibe base, desplazamiento, tamanio y contenido (errores: segmetation fault)
        base = mensaje_recibido.parametros[0];
        offset = mensaje_recibido.parametros[1];
        tamanio = mensaje_recibido.tam_extra;
        contenido = malloc(tamanio);
        memcpy(contenido,mensaje_recibido.extra,tamanio);
        destruir_mensaje(&mensaje_recibido);
        usleep(RETARDO*1000);
        if ((estado = escribirSegmento(pid, base+offset, tamanio, contenido))<0){ //segmetation fault
          enviar_respuesta(nSocket, SEGMENTATION_FAULT);
          log_info(archivo_log,"Hilo CPU. Segmentation Fault al intentar escribir PID %d, BASE %d, TAMANIO %d", pid,base,tamanio);
        }
        else //salio todo bien!
          enviar_respuesta(nSocket, ESCRIBIR_SEGMENTO_OK);
        free(contenido);
      break;
      default: 
        usleep(RETARDO*1000);
        enviar_mensaje(*nSocket, (mensaje_t) {.codigo = 0, .cantidad = 0, .tam_extra = 0, .parametros = NULL, .extra = NULL});
        destruir_mensaje(&mensaje_recibido);
      break;
    }
  }
  close(*nSocket);
  return NULL;
}
