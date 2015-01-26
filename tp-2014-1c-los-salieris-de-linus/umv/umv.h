#ifndef UMV_H_
#define UMV_H_

#include <stdio.h>
#include <unistd.h>
#include <libio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <parser/metadata_program.h>
#include <parser/parser.h>
#include <parser/sintax.h>
#include <commons/log.h>
#include "commons.h"
#include "mensajes.h"
#include "codigo_operacion.h"

#define fconfig "configuracion.txt"

typedef struct nodo_usado_struct {
	int pid;
	int base;
	int tamanio;
	int dmr;
	struct nodo_usado_struct * siguiente;
} nodo_segmento;

/* variables */
extern nodo_segmento *lsu;
extern int (* obtener_bloque) (nodo_segmento *);
extern int TOPE;
extern int RETARDO;
extern int PUERTO;
extern char *umv_dmr;

extern pthread_rwlock_t semaforo_lsu;

int main(int, char **);
/* segmentos */
int reservarMallocInicial(void);
int escribirSegmento(int, int, int, char *);
unsigned conseguirBaseRandom(int,int);
int validar_base(int, int, unsigned);
int reservarSegmento(int, int);
void menu(void);
void *consola(void *);
int obtener_bloque_ff(nodo_segmento *);
int obtener_bloque_wf(nodo_segmento *);
void compactar(void);
void destruirSegmento(int,  int);
char *leerSegmento(int, int, int);
void borrarPrograma(int);
nodo_segmento *buscar_nodo(int, int);
nodo_segmento * buscar_nodos_de_un_programa(int);

/*threads*/
void cargarConfiguracion(void);
int crear_socket_listen(void);
void aceptar_conexion(int);
void *thread_cpu(void *);
void *thread_kernel(void *);
void enviar_respuesta (int *, unsigned);

/*menu*/
void menu(void);
void menuCompactar(void);
void menuCrearSegmento(void);
void menuBorrarSegmento(void);
void menuEscribirSegmento(void);
void menuLeerSegmento(void);
void menuDestruirPrograma(void);
void menuRetardo(void);
void menuAlgoritmo(void);
void menuDumpEstructuras(void);
void menuDumpEstructurasPrograma(void);
void menuDumpMemPrincipal(void);
void menuDumpContentMemPrincipal(void);

void dumpEstructuras(void);
void dumpEstructurasPrograma(int);
void dumpMemPrincipalContenido(int,int);

char * leer_linea(void);
void mostrar_hexadecimal(void *, unsigned, unsigned);
void mostrar_linea_hexadecimal(void *, unsigned, unsigned);

#endif /* UMV_H_ */
