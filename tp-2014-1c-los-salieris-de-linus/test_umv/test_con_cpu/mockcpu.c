#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>
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

void menuCambiarProceso(int ns, mensaje_t mensaje_enviado){
	mensaje_t mensaje_recibido;
	unsigned pid;
	unsigned estado;
	int *p = malloc(1*sizeof(unsigned));
	p[0] = pid;
	printf("Ingrese PID: ");
	scanf("%d", &pid);
	mensaje_enviado.parametros = p;
  	mensaje_enviado.codigo = CAMBIO_PROCESO;
  	mensaje_enviado.cantidad = 1;
  	mensaje_enviado.tam_extra = 0;
  	mensaje_enviado.extra = NULL;
  	enviar_mensaje(ns, mensaje_enviado);
  	//destruir_mensaje(&mensaje_enviado);
  	if ((estado = recibir_mensaje(ns, &mensaje_recibido)) > 0)
    	mostrar_mensaje_recibido(mensaje_recibido);
}
void menuEscribirSegmento(int ns,mensaje_t mensaje_enviado){
  	int pid,base,offset;
  	char* buffer;
  	unsigned estado;
  	mensaje_t mensaje_recibido;
  	printf("Ingrese PID: ");
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

void menuLeerSegmento(int ns, mensaje_t mensaje_enviado){
	mensaje_t mensaje_recibido;
	int pid,base,offset,tamanio;
	unsigned estado;
	printf("Ingrese PID: ");
	scanf("%d", &pid); while (getchar() != '\n');
	printf("Ingrese base: ");
	scanf("%d", &base); while (getchar() != '\n');
	printf("Ingrese desplazamiento: ");
	scanf("%d", &offset); while (getchar() != '\n');
	printf("Ingrese tamanio: ");
	scanf("%d", &tamanio); while (getchar() != '\n');
	mensaje_enviado.codigo = LEER_SEGMENTO;
  	mensaje_enviado.cantidad = 3;
  	mensaje_enviado.tam_extra = 0;
  	mensaje_enviado.extra = NULL;
	int *p = malloc(3*sizeof(unsigned));
	p[0] = base;
	p[1] = offset;
	p[2] = tamanio;
	mensaje_enviado.parametros = p;
	enviar_mensaje(ns, mensaje_enviado);
	destruir_mensaje(&mensaje_enviado);
	if ((estado = recibir_mensaje(ns, &mensaje_recibido)) > 0) mostrar_mensaje_recibido(mensaje_recibido);
}


int main (int argc, char ** argv){
	mensaje_t mensaje_enviado;
	unsigned option;

	/*if (argc != 2) {
    fprintf(stderr, "Introducir el numero de puerto por parametro\n");
    return 1;
  	}
  	unsigned short puerto;
  	if (sscanf(argv[1], "%hu", &puerto) != 1) {
    	fprintf(stderr, "Numero de puerto incorrecto\n");
    	return 1;
  	}
*/
  	//Crear socket
	int ns = socket(AF_INET, SOCK_STREAM, 0);
	 if (ns < 0) {
	  perror("socket");
	  return 1;
	 }

	//Conectarse con UMV
	struct sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(8887);
	inet_aton("127.0.0.1", &sa.sin_addr);
	if (connect(ns, (const void *) &sa, sizeof sa) < 0) {
	  perror("connect");
	  close(ns);
	  return 1;
	}

	//Enviar Handshake
	mensaje_enviado.codigo = HANDSHAKE_CPU;
	mensaje_enviado.cantidad = 0;
	mensaje_enviado.tam_extra = 0;
	mensaje_enviado.extra = NULL;
	mensaje_enviado.parametros = NULL;
	enviar_mensaje(ns, mensaje_enviado);

	//Menu
	while (1) {
	  puts("**************************");
	  puts("**Siempre realizar cambio de proceso primero**");
	  puts("1.\tCambiar de proceso");
	  puts("2.\tEscribir segmento");
	  puts("3.\tLeer segmento");
	  puts("4.\tTerminar ejecucion");
	  puts("**************************");
	  printf("Ingrese numero de operacion: ");
	  scanf("%d",&option);
	  while (getchar() != '\n');
	    switch (option) {
	      case 1: menuCambiarProceso(ns, mensaje_enviado); break;
	      case 2: menuEscribirSegmento(ns, mensaje_enviado); break;
	      case 3: menuLeerSegmento(ns, mensaje_enviado); break;
	      case 4: puts("Fin de la ejecucion"); close(ns); exit(0); break;
	    }
	  }
}