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

menuCrearSegmento(int ns,mensaje_t mensaje_enviado) {
  unsigned pid,tamanio,estado;
  mensaje_t mensaje_recibido;
  printf("Ingrese PID: ");
  scanf("%d", &pid); while (getchar() != '\n');
  printf("Ingrese tamanio: ");
  scanf("%d", &tamanio); while (getchar() != '\n');

  mensaje_enviado.codigo = CREAR_SEGMENTO;
  mensaje_enviado.cantidad = 2;
  mensaje_enviado.tam_extra = 0;
  mensaje_enviado.extra = NULL;
  int *p = malloc(2*sizeof(unsigned));
  p[0] = pid;
  p[1] = tamanio;
  mensaje_enviado.parametros = p;
  enviar_mensaje(ns, mensaje_enviado);
  destruir_mensaje(&mensaje_enviado);
  if ((estado = recibir_mensaje(ns, &mensaje_recibido)) > 0)
    mostrar_mensaje_recibido(mensaje_recibido);
}

void menuEscribirSegmento(int ns,mensaje_t mensaje_enviado){
  //el kernel necesita hacer cambio de proceso para indicar el PID antes de escribir?
  int pid,base,offset;
  char* buffer;
  unsigned estado;
  mensaje_t mensaje_recibido;
  printf("Ingrese PID: "); //esta demas
  scanf("%d", &pid); while (getchar() != '\n');
  printf("Ingrese base: ");
  scanf("%d", &base); while (getchar() != '\n');
  printf("Ingrese desplazamiento: ");
  scanf("%d", &offset); while (getchar() != '\n');
  printf("Ingrese contenido: ");
  buffer = leer_linea();

  mensaje_enviado.codigo = ESCRIBIR_SEGMENTO;
  mensaje_enviado.cantidad = 2;
  mensaje_enviado.tam_extra = strlen(buffer)+1;//aca estamos probando con string con estructura seria distinto
  mensaje_enviado.extra = buffer;
  int *p = malloc(2*sizeof(unsigned));
  p[0] = base;
  p[1] = offset;
  mensaje_enviado.parametros = p;
  enviar_mensaje(ns, mensaje_enviado);
  destruir_mensaje(&mensaje_enviado);
  if ((estado = recibir_mensaje(ns, &mensaje_recibido)) > 0)
    mostrar_mensaje_recibido(mensaje_recibido);
}

void menuDestruirPrograma(int ns,mensaje_t mensaje_enviado){
  unsigned pid;
  unsigned estado;
  mensaje_t mensaje_recibido;
  printf("Ingrese PID: ");
  scanf("%d", &pid); while (getchar() != '\n');

  mensaje_enviado.codigo = DESTRUIR_SEGMENTOS;
  mensaje_enviado.cantidad = 1;
  mensaje_enviado.tam_extra = 0;
  mensaje_enviado.extra = NULL;
  mensaje_enviado.parametros = &pid;
  enviar_mensaje(ns, mensaje_enviado);
  //destruir_mensaje(&mensaje_enviado);
  if ((estado = recibir_mensaje(ns, &mensaje_recibido)) > 0)
    mostrar_mensaje_recibido(mensaje_recibido);
}

int main (int argc, char ** argv) {
  mensaje_t mensaje_enviado;
  unsigned option;
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
  mensaje_enviado.codigo = HANDSHAKE_KERNEL;
  mensaje_enviado.cantidad = 0;
  mensaje_enviado.tam_extra = 0;
  mensaje_enviado.extra = NULL;
  mensaje_enviado.parametros = NULL;
  enviar_mensaje(ns, mensaje_enviado);
  puts("Handshake enviado.");
  while (1) {
    puts("**************************");
    puts("1.\tCrear segmento");
    puts("2.\tEscribir segmento");
    puts("3.\tDestruir programa");
    puts("4.\tTerminar ejecucion");
    puts("**************************");
    printf("Ingrese numero de operacion: ");
    scanf("%d",&option);
    while (getchar() != '\n');
    switch (option) {
      case 1: menuCrearSegmento(ns,mensaje_enviado); break;
      case 2: menuEscribirSegmento(ns,mensaje_enviado); break;
      case 3: menuDestruirPrograma(ns,mensaje_enviado); break;
      case 4: puts("Fin de ejecucion!"); close(ns); exit(0); break;
    }
  }
}