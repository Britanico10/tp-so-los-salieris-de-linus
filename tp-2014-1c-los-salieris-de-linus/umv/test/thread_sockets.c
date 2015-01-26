#include "umv.h"

int crear_socket_listen (void) {
  int socketListen = socket(AF_INET, SOCK_STREAM, 0);
	if (socketListen < 0) return -1;
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
  int *newSocket = malloc(sizeof(int)),estado;
  *newSocket = accept(socketListen, NULL, 0);
  mensaje_t mensaje_actual;
  estado = recibir_mensaje(*newSocket,&mensaje_actual);
  if (estado>=0) {
    switch (mensaje_actual.codigo) {
      case 1: pthread_create(&thread, NULL, thread_plpsocket, newSocket); break; //codigo operacion = 1 handshake PLP
      case 2: pthread_create(&thread, NULL, thread_cpusocket, newSocket); break; //codigo operacion = 2 handshake CPU
      default: //si alguien mas trata de hacer handshake le tiramos un error!
        enviar_mensaje(*newSocket, (mensaje_t) {.codigo = 0, .cantidad = 0, .tam_extra = 0, .parametros = NULL, .extra = NULL});
        destruir_mensaje(&mensaje_actual);
        break;
    }
  }
  pthread_detach(thread);
}

void *thread_plpsocket (void *newSocket) {
  int *nSocket = newSocket;
  mensaje_t mensaje_actual;
  int estado;
  while ((estado = recibir_mensaje(*nSocket, &mensaje_actual)) >= 0) {
    if (!estado) continue;
    if ((mensaje_actual.codigo != 3) || (mensaje_actual.codigo != 4)) {
      enviar_mensaje(*nSocket, (mensaje_t) {.codigo = 0, .cantidad = 0, .tam_extra = 0, .parametros = NULL, .extra = NULL});
      destruir_mensaje(&mensaje_actual);
      continue;
    }
    switch (mensaje_actual.codigo) {
    	case 3: //lala preparar mensaje de respuesta/ejecutar operacion
    		break;
    	case 4: //lolo
    		break;
    	default: break;
    }
  }
  close(*nSocket);
  return NULL;
}

void *thread_cpusocket (void *newSocket) {
  int *nSocket = newSocket;
  mensaje_t mensaje_actual;
  int estado;
  while ((estado = recibir_mensaje(*nSocket, &mensaje_actual)) >= 0) {
    if (!estado) continue;
    if ((mensaje_actual.codigo != 5) || (mensaje_actual.codigo != 6)) {
      enviar_mensaje(*nSocket, (mensaje_t) {.codigo = 0, .cantidad = 0, .tam_extra = 0, .parametros = NULL, .extra = NULL});
      destruir_mensaje(&mensaje_actual);
      continue;
    }
    switch (mensaje_actual.codigo) {
      case 5: //lala preparar mensaje de respuesta/ejecutar operacion
        break;
      case 6: //lolo
        break;
      default: break;
    }
  }
  close(*nSocket);
  return NULL;
}