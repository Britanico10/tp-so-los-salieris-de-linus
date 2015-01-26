#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "mensajes.h"

void CPU_handshake (int nSocket) {
  mensaje_t mensaje;
  mensaje.codigo = 2;
  mensaje.cantidad = 0;
  mensaje.tam_extra = 0;
  mensaje.extra = NULL;
  mensaje.parametros = NULL;
  enviar_mensaje(nSocket, mensaje);
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
  CPU_handshake(ns);
  sleep(10);
  close(ns);
  return 0;
}