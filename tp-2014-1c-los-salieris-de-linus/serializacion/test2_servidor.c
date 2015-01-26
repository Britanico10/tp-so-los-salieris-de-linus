#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "mensajes.h"

int crear_socket_listen (unsigned short puerto) {
  int numero = socket(AF_INET, SOCK_STREAM, 0);
  if (numero < 0) return -1;
  struct sockaddr_in sa;
  sa.sin_family = AF_INET;
  sa.sin_port = htons(puerto);
  sa.sin_addr.s_addr = INADDR_ANY;
  if (bind(numero, (const void *) &sa, sizeof sa) < 0) {
    close(numero);
    return -1;
  }
  if (listen(numero, SOMAXCONN) < 0) {
    close(numero);
    return -1;
  }
  return numero;
}

void * thread_socket (void * ptr) {
  int * nSocket = ptr;
  mensaje_t mensaje_actual;
  int estado;
  while ((estado = recibir_mensaje(*nSocket, &mensaje_actual)) >= 0) {
    if (!estado) continue;
    if ((mensaje_actual.codigo != 1) || (mensaje_actual.cantidad != 2) || mensaje_actual.tam_extra) {
      enviar_mensaje(*nSocket, (mensaje_t) {.codigo = 0, .cantidad = 0, .tam_extra = 0, .parametros = NULL, .extra = NULL});
      destruir_mensaje(&mensaje_actual);
      continue;
    }
    destruir_mensaje(&mensaje_actual);
    
    char *texto_a_enviar = "a = b + 12\n";
    char *texto = malloc(strlen(texto_a_enviar)+1);
    *texto = 0;
    strcat(texto,texto_a_enviar);
    mensaje_actual.parametros = malloc(sizeof(unsigned)*2);
    mensaje_actual.tam_extra = strlen(texto)+1;
    mensaje_actual.extra = texto;
    mensaje_actual.codigo = 2;
    mensaje_actual.cantidad = 2;
    mensaje_actual.parametros[0] = 324;
    mensaje_actual.parametros[1] = 123;
    enviar_mensaje(*nSocket, mensaje_actual);
    free(texto);
    free(mensaje_actual.parametros);
  }
  close(*nSocket);
  return NULL;
}

void aceptar_conexion (int socketListen) {
  pthread_t thread;
  int * ns = malloc(sizeof(int));
  *ns = accept(socketListen, NULL, 0);
  pthread_create(&thread, NULL, thread_socket, ns);
  pthread_detach(thread);
}

int main (void) {
  unsigned short puerto = 1234;
  int socketListen = -1;
  while (socketListen < 0) {
    socketListen = crear_socket_listen(puerto);
    if (socketListen < 0) puerto ++;
  }
  printf("Socket en escucha creado en el puerto %hu\n", puerto);
  while (46) aceptar_conexion(socketListen);
  return 0;
}