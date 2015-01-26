#include <parser/parser.h>
#include "primitivas.h"

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

int PUERTO_UMV;
int PUERTO_KERNEL;

int socketUMV, socketKernel;

char * nombresDiccionario;
unsigned * punterosDiccionario;

pcb PCB_actual;

int estado_programa; // 0: normal, distinto de cero: error/fin
	/*
		1 = finalizado
		2 = segmentation fault
		3 = stack overflow
		4 = error de ejecucion
		5 = error desconocido
		6 = variable compartida inexistente
		7 = etiqueta o nombre de procedimiento/funcion inexistente
		8 = retorno de funcion mas alla del inicio del programa / stack underflow
	*/

AnSISOP_funciones funciones = {
	.AnSISOP_definirVariable = pr_definir_variable,
	.AnSISOP_obtenerPosicionVariable = pr_puntero_variable,
	.AnSISOP_dereferenciar = pr_obtener_valor,
	.AnSISOP_asignar = pr_escribir_valor,
	.AnSISOP_obtenerValorCompartida = pr_obtener_variable_compartida,
	.AnSISOP_asignarValorCompartida = pr_asignar_variable_compartida,
	.AnSISOP_irAlLabel = pr_ir_a,
	.AnSISOP_llamarSinRetorno = pr_ejecutar_procedimiento,
	.AnSISOP_llamarConRetorno = pr_ejecutar_funcion,
	.AnSISOP_finalizar = pr_finalizar,
	.AnSISOP_retornar = pr_retornar,
	.AnSISOP_imprimir = pr_imprimir_numero,
	.AnSISOP_imprimirTexto = pr_imprimir_texto,
	.AnSISOP_entradaSalida = pr_solicitar_IO
};

AnSISOP_kernel funciones_kernel = {
	.AnSISOP_wait = pr_wait,
	.AnSISOP_signal = pr_signal
};