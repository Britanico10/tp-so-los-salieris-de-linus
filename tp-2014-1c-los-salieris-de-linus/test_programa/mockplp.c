#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "mensajes.h"
#include "codigo_operacion.h"

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

void mostrar_mensaje_recibido (mensaje_t mensaje) {
  puts("------------NUEVO MENSAJE--------------------------");
  printf("Codigo     : %10u\nParametros : %10u\nDatos extra: %10u\n", mensaje.codigo, mensaje.cantidad, mensaje.tam_extra);
  unsigned parametro;
  for (parametro = 0; parametro < mensaje.cantidad; parametro ++)
    printf("Par#%7u: %10u\n", parametro + 1, mensaje.parametros[parametro]);
  //printf("Contenido  : %s\n", mensaje.extra);
  puts("-------------FIN NUEVO MENSAJE---------------------");
  putchar('\n');
}


void * thread_socket (void * ptr) {
  int * nSocket = ptr;
  //char * buffer = malloc(1), * respuesta;
  unsigned acum_buffer = 0;
  mensaje_t mensaje_recibido, mensaje_enviado;
  int estado;
  while ((estado = recibir_mensaje(*nSocket, &mensaje_recibido)) >= 0) {
    switch (mensaje_recibido.codigo){
      case HANDSHAKE_PROGRAMA: 
        puts("\nSe conecto un programa nuevo");
      break;
      case COMIENZO_CODIGO:
        puts("\nSe anuncio comienzo de transferencia de codigo");
        mensaje_enviado.codigo = CODIGO_RECIBIDO;
        mensaje_enviado.cantidad = 0;
        mensaje_enviado.tam_extra = 0;
        mensaje_enviado.parametros = NULL;
        mensaje_enviado.extra = NULL;
        enviar_mensaje(*nSocket,mensaje_enviado);
      break;
      case CODIGO:
        mostrar_mensaje_recibido(mensaje_recibido);
        printf("%s\n", mensaje_recibido.extra);
        destruir_mensaje(&mensaje_recibido);
        mensaje_enviado.codigo = CODIGO_RECIBIDO;
        mensaje_enviado.cantidad = 0;
        mensaje_enviado.tam_extra = 0;
        mensaje_enviado.parametros = NULL;
        mensaje_enviado.extra = NULL;
        enviar_mensaje(*nSocket,mensaje_enviado);
      break;
      case FIN_CODIGO:
        puts("\nTermino la transferencia de codigo.");
      break;
      default:
        enviar_mensaje(*nSocket,(mensaje_t) {.codigo = 0, .cantidad = 0, .tam_extra = 0, .parametros = NULL, .extra = NULL});
        puts("\nSe recibio un mensaje desconocido.");
      break;
    }
  }
  close(*nSocket);
  puts("\nSe desconecto el programa!");
  exit(0);
  return NULL;
}

void menuEscribirMensaje(int ns){
  mensaje_t mensaje_enviado;
  char* buffer;
  printf("Ingrese contenido a imprimir en el programa: ");
  buffer = leer_linea();
  mensaje_enviado.codigo = IMPRIMIR;
  mensaje_enviado.cantidad = 0;
  mensaje_enviado.tam_extra = strlen(buffer)+1;
  mensaje_enviado.parametros = NULL;
  mensaje_enviado.extra = buffer;
  enviar_mensaje(ns, mensaje_enviado);
  puts("Ya envie un mensaje para imprimir");
  destruir_mensaje(&mensaje_enviado);
}

void menuFinalizarPrograma(int ns){
  mensaje_t mensaje_enviado;
  mensaje_enviado.codigo = FIN_PROGRAMA;
  mensaje_enviado.cantidad = 0;
  mensaje_enviado.tam_extra = 0;
  mensaje_enviado.parametros = NULL;
  mensaje_enviado.extra = NULL;
  enviar_mensaje(ns, mensaje_enviado);
  puts("Ya envie el codigo FIN_PROGRAMA");
}

void * consola (void * ptr) {
  int * ns = ptr;
  unsigned option;
   while (1) {
    puts("**************************");
    puts("1.\tEscribir Mensaje");
    puts("2.\tFinalizar programa");
    puts("3.\tTerminar ejecucion");
    puts("**************************");
    printf("Ingrese numero de operacion: ");
    scanf("%d",&option); while (getchar() != '\n');
    switch (option) {
      case 1: menuEscribirMensaje(*ns); break;
      case 2: menuFinalizarPrograma(*ns); break;
      case 3: puts("Fin de la ejecucion"); close(ns); exit(0); break;
    }
  }
}

void aceptar_conexion (int socketListen) {
  pthread_t thread;
  int * ns = malloc(sizeof(int));
  *ns = accept(socketListen, NULL, 0);
  pthread_create(&thread, NULL, thread_socket, ns);
  pthread_detach(thread);
  pthread_create(&thread, NULL, consola, ns);
  pthread_detach(thread);
}

int main (void) {
  unsigned short puerto = 6000;
  int socketListen;
  socketListen = crear_socket_listen(puerto);
  printf("Socket en escucha creado en el puerto %hu\n", puerto);
  while (1) aceptar_conexion(socketListen);
  return 0;
}
