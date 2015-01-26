#ifndef KERNEL_H_
#define KERNEL_H_

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
#include <sys/select.h>
#include <parser/metadata_program.h>
#include <parser/parser.h>
#include <parser/sintax.h>
#include "commons.h"
#include "mensajes.h"
#include "codigo_operacion.h"

#define PATH "config_kernel.config"//temporal
#define envio 1
#define recepcion 0

#define PUERTO_PROG "PUERTO_PROG"
#define PUERTO_CPU "PUERTO_CPU"
#define QUANTUM "QUANTUM"
#define RETARDO_CPU "RETARDO_CPU"
#define MULTIPROGRAMACION "MULTIPROGRAMACION"
#define PUERTO_UMV "PUERTO_UMV"
#define STACK "STACK"
#define VAR_GLOBALES "VAR_GLOBALES"
#define ID_HIO "ID_HIO"
#define VALOR_SEMAFORO "VALOR_SEMAFORO"
#define HIO "HIO"
#define SEMAFOROS "SEMAFOROS"
#define NIVEL_LOG "NIVEL_LOG"


typedef struct {
	int puertoPrograma;
	int puertoCPU;
	int puertoUMV;
	int quantum;
	int retardo_CPU;
	int gradoMulti;
	int tamaStack;
	char *ip_umv;
	int nivel_log;
	t_dictionary *semaforos;
	t_dictionary *dispositivosIO;
	t_dictionary *varGlobales;
} t_archConfig;

typedef struct {
	unsigned codigo;
	char *texto;
	unsigned idPCB;
	int numero; //retardo
} __attribute__((packed)) t_mensaje_pipe;


typedef struct {
	char *id_Sem;
	int valor;
	t_queue *progBloq;
} t_dataSemaforo;

typedef struct {
	char *id_IO;
	int retardoInicial;
	int pipeIO;
	t_list *progBloq;
	unsigned flag_uso;
} t_dataDispositivos;

typedef struct {
	int valorVariable;
} t_dataVarCompartidas;

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
} t_PCB;

typedef struct {
	t_PCB * PCB;
	int peso;
} t_nodoNew;

typedef struct {
	t_PCB *idPCB;
} t_nodoReady;

typedef struct {
	t_PCB *idPCB;
	int qRestante;
	int socketCpu; 				// es el socket de la CPU, para el caso de que se desconecte y poder buscar el programa que se esta ejecutando al que hay que eliminar
} t_nodoExec;

typedef struct {
	int socketCPU;
	int estado; 			// 0: disponible, 1: en uso
} t_nodoCPU;

typedef struct {
	t_PCB *PCB;
	int socket;
	int idPrograma;		//IMPORTANTE
	int estado; 		//0: New, 1: Ready, 2: Exec, 3: BloqIO, 4:BloqSemaforo, 5: Exit
} t_nodoPrograma;

typedef struct {
	t_PCB *PCB;
	int retardoIO;
	int estado;		//esta activo o no el programa??
} t_nodoBloqIO;


typedef struct{
	t_PCB*pcb;
}t_nodoExit;



typedef struct {
	// ...nombre, unidad de retardo, id
} t_IO;

int main(int, char **);

void crearPipes(void);
void crerThreads(void);
void *pcp(void*);
void *plp(void*);
void crearThreads(void);
void crearPipes(void);
void *io(t_dataDispositivos*);
t_archConfig *cargarConfiguracion(void);
t_dictionary* obtener_diccionario_dispositovos(char**,char**);
t_dictionary* obtener_diccionario_semaforos(char**,char**);
t_dictionary* obtener_diccionario_var_globales(char**);
void imprimirListas(void);

int crear_socket_listen(int);
void enviar_tam_segmento(int unSocket,int tamano, int id);
void mandar_destruir_prog(int socket,int pid);
int enviar_Segmento(int socket,char *buffer, unsigned base, int tamano);
void imprimir_lista(t_list*lista);
int previo_menor(t_nodoNew* previo,t_nodoNew* posterior);
void destruir_Programa(t_PCB*pcb, int unSocket);
void *liberar_nodoPrograma(t_nodoPrograma* data);
int aceptar_conexion(int socket, t_list *lista, int idProg);
int conectarse(const char * ip, int puerto);
void conectar_socket(int puerto, char* direccion,int socket);
int obtener_socket();

void eliminar_de_new(unsigned );
void mandar_eliminar_prog_pcp(unsigned);
void eliminar_programa_sistema(int );
void pasar_a_ready();
int enviar_codigo(char*,int ,int );
int escribir_segmentos(t_metadata_program*,t_nodoPrograma*,char*);
void nuevo_exit(t_mensaje_pipe *);
void analizar_msg_pipe(void);
void imprimir_en_programa(char*,int socket);
int verificar_tam_etiquetas(int ,int ,int );
void pasar_a_NEW(t_metadata_program* ,t_nodoPrograma*);
void reservar_segmentos(t_PCB *,t_metadata_program*,int ,char*);
void cargar_programa_en_mem(char*,int );
int rta_escritura_UMV(int );


int buscarCPUDisponible(void);
void ejecutar(t_PCB*, int);
void asignarValor(char *, int, int);
void obtenerValor(char *,int);
void enviar_imprimir(char *, int, int);
int signal_semaforo(char *, int);
void desbloquearPCB_sem(char *);
void wait_semaforo(char *, int);
t_PCB *sacarPCBdeExec(int);
void cambiarEstado(int);
void actualizarPCB(mensaje_t);
void analizarQuantumRestante(int, int);
void eliminarCPU(int);
void planificarReady(t_PCB *pcb);
void planificarExec(void);
void planificarExit(int, int, char *);
void finPrograma(int, char *);
void seg_fault(int, int);
void desbloquearPCB_IO(char *, int);
void encolarPCBenIO(int, char *, int);
void bloquearProgramaIO(int, t_dataDispositivos*, t_PCB*);
t_PCB *buscarPCBporIDenExec(int);
void cambiarEstadoProg(t_PCB *, int);
void sacarDeLista(t_PCB *, int);
void stack_fault(int, int);
t_dataDispositivos *buscarDispIO(char *);
t_nodoBloqIO *desbloquearPrograma(t_dataDispositivos *, int);

int crear_socket_listen (int);


#endif /* KERNEL_H_ */	
