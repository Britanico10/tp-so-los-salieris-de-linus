#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "mensajes.h"
#include "codigo_operacion.h"

void mostrar_mensaje_recibido (mensaje_t mensaje) {
  puts("------------NUEVO MENSAJE--------------------------");
  printf("Codigo     : %10u\nParametros : %10u\nDatos extra: %10u\n", mensaje.codigo, mensaje.cantidad, mensaje.tam_extra);
  unsigned parametro;
  for (parametro = 0; parametro < mensaje.cantidad; parametro ++)
    printf("Par#%7u: %10u\n", parametro + 1, mensaje.parametros[parametro]);
  printf("Contenido  : %s\n", mensaje.extra);
  puts("-------------FIN NUEVO MENSAJE---------------------");
  putchar('\n');
}

int crear_socket_listen (unsigned puerto) {
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
  //char * buffer = malloc(1), * respuesta;
  unsigned acum_buffer = 0;
  mensaje_t mensaje_recibido, mensaje_enviado;
  int estado;
  while ((estado = recibir_mensaje(*nSocket, &mensaje_recibido)) >= 0) {
    switch (mensaje_recibido.codigo){
      case HANDSHAKE_CPU: 
        puts("\nSe conecto una cpu");
      break;
      default: mostrar_mensaje_recibido(mensaje_recibido); break;
    }
  }
}

void aceptar_conexion (int socketListen) {
  pthread_t thread;
  int * ns = malloc(sizeof(int));
  *ns = accept(socketListen, NULL, 0);
  pthread_create(&thread, NULL, thread_socket, ns);
  pthread_detach(thread);
}

int main (void) {
  unsigned puerto = 5000;
  int socketListen;
  socketListen = crear_socket_listen(puerto);
  printf("Socket en escucha creado en el puerto %hu\n", puerto);
  while (1) aceptar_conexion(socketListen);
  return 0;
}