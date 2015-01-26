#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "mensajes.h"
/* Manda un mensaje con dos parametros y sin texto */
void mandar_mensaje (int nSocket, unsigned n1, unsigned n2) {
  mensaje_t mensaje;
  mensaje.codigo = 1;
  mensaje.cantidad = 2;
  mensaje.tam_extra = 0;
  mensaje.extra = NULL;
  unsigned p[2];
  p[0] = n1;
  p[1] = n2;
  mensaje.parametros = p;
  enviar_mensaje(nSocket, mensaje);
}

void mostrar_mensaje_recibido (mensaje_t mensaje) {
  printf("Codigo     : %10u\nParametros : %10u\nDatos extra: %10u\n", mensaje.codigo, mensaje.cantidad, mensaje.tam_extra);
  unsigned parametro;
  for (parametro = 0; parametro < mensaje.cantidad; parametro ++)
    printf("Par#%7u: %10u\n", parametro + 1, mensaje.parametros[parametro]);
  printf("Contenido  : %s\n", mensaje.extra);
  putchar('\n');
}

int main (int argc, char ** argv) {
  if (argc != 2) {
    fprintf(stderr, "Introducir el numero de puerto por parametro\n");
    return 1;
  }
  unsigned short puerto;
  if (sscanf(argv[1], "%hu", &puerto) != 1) {
    fprintf(stderr, "Numero de puerto incorrecto\n");
    return 1;
  }
  int ns = socket(AF_INET, SOCK_STREAM, 0);
  if (ns < 0) {
    perror("socket");
    return 1;
  }
  struct sockaddr_in sa;
  sa.sin_family = AF_INET;
  sa.sin_port = htons(puerto);
  inet_aton("127.0.0.1", &sa.sin_addr);
  if (connect(ns, (const void *) &sa, sizeof sa) < 0) {
    perror("connect");
    close(ns);
    return 1;
  }
  mandar_mensaje(ns, 12, 9);
  mensaje_t mensaje;
  recibir_mensaje(ns, &mensaje);
  mostrar_mensaje_recibido(mensaje);
  destruir_mensaje(&mensaje);
  mandar_mensaje(ns, 532, 11);
  recibir_mensaje(ns, &mensaje);
  mostrar_mensaje_recibido(mensaje);
  destruir_mensaje(&mensaje);
  close(ns);
  return 0;
}