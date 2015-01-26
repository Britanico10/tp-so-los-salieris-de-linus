#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "mensajes.h"
/* recibe mensaje de codigo 1, y que tengan 2 parametros y sin texto
 y devuelve otro mensaje donde con texto que repite el primer parametro, la cantidad de veces que indica el segundo parametro
*/
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

void *thread_socket (void * ptr) {
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
    unsigned numero, cantidad, x;
    numero = mensaje_actual.parametros[0];
    cantidad = mensaje_actual.parametros[1];
    destruir_mensaje(&mensaje_actual);
    char numero_como_string[20];
    sprintf(numero_como_string, "%u", numero);
    char * numero_repetido = malloc(strlen(numero_como_string) * cantidad + 1);
    *numero_repetido = 0; // inicializa a ""
    for (x = cantidad; x; x --) strcat(numero_repetido, numero_como_string);
    mensaje_actual.codigo = 2;
    mensaje_actual.cantidad = 1;
    mensaje_actual.tam_extra = strlen(numero_como_string) * cantidad + 1;
    mensaje_actual.extra = numero_repetido;
    unsigned tamano = strlen(numero_repetido);
    mensaje_actual.parametros = &tamano; // equivale a un array de 1 posicion
    enviar_mensaje(*nSocket, mensaje_actual);
    free(numero_repetido);
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