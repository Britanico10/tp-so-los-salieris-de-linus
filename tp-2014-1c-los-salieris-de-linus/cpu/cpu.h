#ifndef CPU_H_
#define CPU_H_

#include <stdio.h>
#include <libio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "commons.h"
#include "mensajes.h"
#include <commons/log.h>
#include <stdint.h>
#include <commons/log.h>
#include "primitivas.h"
#include "codigo_operacion.h"

#define FCONFIG "configuracion.txt"

#define EXIT_SUCCES 0
#define EXIT_FAILURE 1

typedef struct pcb_struct {
	unsigned pid;					//Identificador único del Programa en el sistema
	unsigned segmento_codigo;		//Dirección del primer byte en la UMV del segmento de código
	unsigned segmento_stack;		//Dirección del primer byte en la UMV del segmento de stack
	unsigned pointer_stack;			//Dirección del primer byte en la UMV del Contexto de Ejecución Actual
	unsigned indice_codigo;			//Dirección del primer byte en la UMV del Índice de Código
	unsigned indice_etiquetas;		//Dirección del primer byte en la UMV del Índice de Etiquetas
	unsigned pc;					//Número de la próxima instrucción a ejecutar
	unsigned tam_contexto_actual;	//Cantidad de variables (locales y parámetros) del Contexto de Ejecución Actual
	unsigned tam_indice_etiquetas;	//Cantidad de bytes que ocupa el Índice de etiquetas
} pcb;

int main(int, char **);

void cargarConfiguracion(void);
int conectarse(const char *, int);
void handshake(int,unsigned);
pcb recuperar_pcb(mensaje_t);
int construirDiccionario(pcb);
void destruirDiccionario(void);
char * obtener_siguiente_instruccion(pcb);
void mandar_PCB_actualizado(void);
void mandar_mensaje_finalizacion(pcb);

unsigned desempaquetar_32(const void *);
void empaquetar_32(unsigned, void *);

int leer_datos(unsigned, unsigned, unsigned, void *);
int escribir_datos(unsigned, unsigned, unsigned, void *);
unsigned obtener_puntero_de_etiqueta(const char *);

extern int socketUMV, socketKernel;

extern char * nombresDiccionario;
extern unsigned * punterosDiccionario;

extern pcb PCB_actual;

extern int estado_programa;

extern AnSISOP_funciones funciones;
extern AnSISOP_kernel funciones_kernel;

#endif /* CPU_H_ */	
