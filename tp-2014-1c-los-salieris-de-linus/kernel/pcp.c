#include "kernel.h"


int CPUEsperando;
int programasEsperando;
fd_set setPCP;
int fdMax;
extern t_log *log_kernel;
extern pthread_mutex_t mutexReady;
extern pthread_mutex_t mutexExit;
extern pthread_mutex_t mutexListaProgramas;


void aceptar_conexion_CPU(int socket, t_list *lista){
  int nuevoFichero;
  size_t tamanio = sizeof(struct sockaddr_in);
  struct sockaddr_in extDirec;
  t_nodoCPU *nodoCPU = malloc(sizeof(t_nodoCPU));
  mensaje_t handshake, respuesta;
  extern fd_set setPCP;
  extern int fdMax;
  extern int retardo_CPU;

  if ((nuevoFichero = accept(socket, (struct sockaddr*)&extDirec, (socklen_t*)&tamanio)) == -1){
    perror("Error al aceptar conexion. \n");
    log_error(log_kernel, "PCP: No se pudo aceptar la conexion.");
  } else {
	  recibir_mensaje(nuevoFichero, &handshake);
	  if(handshake.codigo == HANDSHAKE_CPU){
		  FD_SET(nuevoFichero, &setPCP);
		  if (nuevoFichero > fdMax){
			  fdMax = nuevoFichero;
		  }
		  nodoCPU->socketCPU = nuevoFichero;
		  nodoCPU->estado = 0;
		  list_add(lista, nodoCPU);
		  log_info(log_kernel, "PCP: Se conecto una CPU.", nuevoFichero);
		  log_debug(log_kernel, "PCP: Socket de CPU conectada: %d.", nuevoFichero);
		  destruir_mensaje(&handshake);


		  respuesta.codigo = HANDSHAKE_CPU_RET;
		  respuesta.cantidad = 1;
		  respuesta.parametros = malloc(sizeof(unsigned));
		  respuesta.parametros[0] = retardo_CPU;
		  respuesta.tam_extra = 0;
		  respuesta.extra = strdup("");
		  enviar_mensaje(nuevoFichero, respuesta);
		  destruir_mensaje(&respuesta);

	  } else{
		  //log_warning(log_kernel, "PCP: Se conecto algo que no era una CPU (o el Handshake esta mal).");
		  close(nuevoFichero);
	  }
  }
}


void analizarRespuesta(mensaje_t mensaje, int socket){
	extern int CPUEsperando;
	extern int programasEsperando;
	switch(mensaje.codigo){
		case SYS_IMPR:
			/*Enviar al programa el valor a imprimiEnviar al programa la cadena a imprimir*/
			/* Buscar el id y pasarlo al PLP -> FALTA*/
			enviar_imprimir(string_itoa(mensaje.parametros[0]), 0, socket);
			log_info(log_kernel, "PCP: CODIGO RECIBIDO: SYS CALL Imprimir Valor.");
			log_debug(log_kernel, "PCP: \n\tNumero Codigo: %d.\n\tValor: %d.\n", mensaje.codigo, mensaje.parametros[0]);
			break;
		case SYS_IMPR_TEXTO:
			/*Enviar al programa la cadena a imprimir*/
			enviar_imprimir(mensaje.extra, 1, socket);
			log_info(log_kernel, "PCP: CODIGO: SYS CALL Imprimir Texto.");
			log_debug(log_kernel, "PCP: \n\tNumero Codigo: %d.\n\tTexto: %s.\n", mensaje.codigo, mensaje.extra);
			break;
		case SYS_OBTENER_VALOR:
			/* Obtener_Valor(mensaje,t_dictionary *var_compartidas,socket)
			 * Revisar diccionario de variables compartidas,
			 * y enviar el valor de la solicitada. Si no esta, enviar error
			 * */
			log_info(log_kernel, "PCP: CODIGO: SYS CALL Obtener Valor de Variable Compartida.");
			log_debug(log_kernel, "PCP: \n\tNumero Codigo: %d.\n\tVariable Compartida: %s.\n", mensaje.codigo, mensaje.extra);
			obtenerValor(mensaje.extra, socket);
			break;
		case SYS_ASIGNAR_VALOR: //tengo que devolver algo?
			/*Asignar Valor(socket,t_dictionary diccionario,mensaje)
			 * Revisar diccionario de variables compartidas, asignar el valor que mando la CPU
			 * */
			log_info(log_kernel, "PCP: CODIGO: SYS CALL Asignar Valor de Variable Compartida.");
			log_debug(log_kernel, "PCP: \n\tNumero Codigo: %d.\n\tVariable Compartida: %s\tValor: %d\n", mensaje.codigo, mensaje.extra, mensaje.parametros[0]);

			asignarValor(mensaje.extra, mensaje.parametros[0], socket); //No se que me devuelve dictionary_put si no encuentra el key

			break;
		case SYS_SIGNAL:
			/*hacer_signal(semaforos,sem,ready)
			 * Sumar valor de semaforo indicado (en el diccionario de semaforos)
			 * if (semaforo <= 0): DESBLOQUEO
			 * 	Pasar de BLOQ a READY
			 * 	Sumar READY_OK
			 * 	Planificar()
			 * 	*/
			log_info(log_kernel, "PCP: CODIGO: SYS CALL Signal de Semaforo.");
			log_debug(log_kernel, "PCP: \n\tNumero Codigo: %d.\n\tSemaforo: %s\n", mensaje.codigo, mensaje.extra);

			if (signal_semaforo(mensaje.extra, socket)){
				programasEsperando++;
				while ((programasEsperando > 0) && (CPUEsperando > 0)){ //if o while?
					planificarExec();
				}
			}
			break;
		case SYS_WAIT:
			/*hacer_wait(valorsemaforo,t_dictionary*semaforos,t_lisst *exec)
			 * Restar valor de semaforo indicado (en el diccionario de semaforos)
			 * if (semaforo < 0)
			 * 	sacar PCB de EXEC y mandarlo a la lista de BLOQ del semaforo
			 * 	sumar CPU_OK
			 * 	planificar()
			 * 	else
			 * 	*/
			log_info(log_kernel, "PCP: CODIGO: SYS CALL Wait de Semaforo.");
			log_debug(log_kernel, "PCP: \n\tNumero Codigo: %d.\n\tSemaforo: %s\n", mensaje.codigo, mensaje.extra);

			wait_semaforo(mensaje.extra, socket);
			/*if (wait_semaforo(mensaje.extra, socket)){			//OJO QUE SI SE BLOQUEO YO ESTOY SACANDOLO DE EXIT (DEBERIA ACTUALIZAR LA PCB! con la CPU)
				cambiarEstado(socket);							//Pongo disponible una CPU
				CPUEsperando++;
				while ((programasEsperando > 0) && (CPUEsperando > 0)){ //if o while?
					planificarExec();
				}
			}*/
			break;
		case SYS_IO:
			/* enviar_a_IO(unidades, idDisp)
			 * Sacar de EXEC el PCB y agregar PCB junto con el retardo a la cola de programas BLOQ en el diccionario de dispositivos
			 * Avisarle al HILO (??? como, pipes, semaforo?) signal(IO_OK)
			 * Sumar CPU_OK
			 * Planificar()
			 * */

			log_info(log_kernel, "PCP: CODIGO: SYS CALL Bloqueo por IO.");
			log_debug(log_kernel, "PCP: \n\tNumero Codigo: %d.\n\tDispositivo de IO: %s, Retardo: %d.\n", mensaje.codigo, mensaje.extra, mensaje.parametros[0]);
			encolarPCBenIO(mensaje.parametros[0], mensaje.extra, socket);
			/*CPUEsperando++;
			while ((programasEsperando > 0) && (CPUEsperando > 0)){
				planificarExec();
			}*/
			break;

		case FIN_INSTRUCCION:
			//analizar_quantum_cpu(mensaje);
			/*Actualiza PCB, de la lista de PCB
			 * Revisa la cantidad de quantum que le queda a el proceso (quantum restante en la lista de exec)
			 * Si es mayor a 0, le vuelve a mandar a la misma cpu el mismo proceso
			 *
			 * Si es 0, saca el PCB de lista EXEC y lo manda a Ready, suma Ready_OK y suma CPU_OK
			 * planificar()
			 *
			 * */
			log_info(log_kernel, "PCP: CODIGO: Fin de Instruccion.");
			log_debug(log_kernel, "PCP: \n\tNumero Codigo: %d.\n\tPrograma: %d\n", mensaje.codigo, mensaje.parametros[0]);
			actualizarPCB(mensaje);
			analizarQuantumRestante(socket, mensaje.parametros[9]);
			break;
		case FIN_PROGRAMA:
			/* terminar_ejecucion_programa(listaPCB, estadoVariables)
			 * 2. CPU termina la ejecucion del programa
			 * Con lo que le llega del CPU (PCB, estado final de variables)
			 * Actualiza la PCB
			 * Manda al programa el estado final de sus variables
			 * Saca el PCB de EXEC, lo pasa a lista de EXIT, suma CPU_OK
			 * planificar()
			 * */
			log_info(log_kernel, "PCP: CODIGO: Fin de Programa.");
			log_debug(log_kernel, "PCP: \n\tNumero Codigo: %d.\n", mensaje.codigo);
			//actualizarPCB(mensaje);
			finPrograma(socket, mensaje.extra);
			break;
		case SEGMENTATION_FAULT:
			/*Segmentation_fault(exec,exitsocketUMV)
			 *
			 * Excepcion: Segmentation Fault
			 * Se fija en la lista de EXEC que programa tiene asignado esa CPU
			 * Le notifica al programa su excepcion (desconecta su socket),
			 * lo saca de EXEC y lo pasa a lista de EXIT, SUMA CPU_OK
			 * le pide a la UMV la destruccion de sus segmentos --> lo hace el PLP
			 * planificar()
			 * */
			log_info(log_kernel, "PCP: CODIGO: Segmentation Fault.");
			log_debug(log_kernel, "PCP: \n\tNumero Codigo: %d.\n", mensaje.codigo);
			seg_fault(socket, SEGMENTATION_FAULT);
			break;
		case STACK_OVERFLOW:
			log_info(log_kernel, "PCP: CODIGO: Stack Overflow.");
			log_debug(log_kernel, "PCP: \n\tNumero: %d.\n", mensaje.codigo);
			stack_fault(socket, 0);
			break;
		case STACK_UNDERFLOW:
			log_info(log_kernel, "PCP: CODIGO: Stack Underflow.");
			log_debug(log_kernel, "PCP:\n\tNumero: %d.\n", mensaje.codigo);
			stack_fault(socket, 1);
			break;
		default:
			//log_warning(log_kernel, "PCP: No se reconoce el CODIGO recibido: %d.\n", mensaje.codigo);
			break;
	}

}

void encolarPCBenIO(int uniRetardo, char *idDisp, int socketCPU){
	extern t_list *exec;
	int _is_socketCPU(t_nodoExec *nodoExec) {
		return ((nodoExec->socketCpu) == (socketCPU));
	}
	t_nodoExec *nodoBuscado = list_find(exec, (void*) _is_socketCPU);
	nodoBuscado->qRestante = 0;

	t_dataDispositivos *dispositivo = buscarDispIO(idDisp);

	t_PCB *PCBaBloquear = nodoBuscado->idPCB;
	t_nodoBloqIO *nodoBloqIO = malloc(sizeof(t_nodoBloqIO));
	nodoBloqIO->PCB = PCBaBloquear;
	nodoBloqIO->estado = 0;
	nodoBloqIO->retardoIO = uniRetardo;
	list_add(dispositivo->progBloq, nodoBloqIO);
	imprimirListas();

	bloquearProgramaIO(uniRetardo, dispositivo, PCBaBloquear);
}

void bloquearProgramaIO(int uniRetardo, t_dataDispositivos *dispositivo, t_PCB *PCBaBloquear){
	//t_PCB *PCBaBloquear = sacarPCBdeExec(socket);
	//t_dataDispositivos *dispositivo = buscarDispIO(idDisp);

	t_mensaje_pipe *mensajeParaIO = malloc(sizeof(t_mensaje_pipe));
	mensajeParaIO->codigo = SYS_BLOQ_IO;
	mensajeParaIO->numero = uniRetardo;
	mensajeParaIO->idPCB = PCBaBloquear->pid;
	mensajeParaIO->texto = strdup("");

	cambiarEstadoProg(PCBaBloquear, 3);
	write(dispositivo->pipeIO, mensajeParaIO, sizeof(t_mensaje_pipe));
	dispositivo->flag_uso = 1;
}

void desbloquearPCB_IO(char *idDisp, int idPCB){
	log_info(log_kernel, "PCP: Entra en desbloquear. Disp: %s, IDPCB: %d.\n", idDisp, idPCB);
	t_dataDispositivos *dispositivo = buscarDispIO(idDisp);
	t_nodoBloqIO *nodoBloq = desbloquearPrograma(dispositivo, idPCB);
	t_PCB *PCBaDesbloquear = nodoBloq->PCB;
	planificarReady(PCBaDesbloquear);

	if ((!list_is_empty(dispositivo->progBloq)) && (dispositivo->flag_uso == 0)){
		t_nodoBloqIO *nodoABloquear = list_get(dispositivo->progBloq, 0);
		bloquearProgramaIO(nodoABloquear->retardoIO, dispositivo, nodoABloquear->PCB);
		dispositivo->flag_uso = 1;
	}else{
		dispositivo->flag_uso = 0;
		free(nodoBloq);
	}
}

t_nodoBloqIO *desbloquearPrograma(t_dataDispositivos *disp, int idPCB){

	int _buscarPorPCB(t_nodoBloqIO *nodoBloq) {
		return ((nodoBloq->PCB->pid) == (idPCB));
	}
	t_nodoBloqIO *nodoBuscado = list_remove_by_condition(disp->progBloq, (void *)_buscarPorPCB);
	return nodoBuscado;
}

void asignarPipe(char *idIO, int pipe){
	t_dataDispositivos *dataDisp = buscarDispIO(idIO);
	dataDisp->pipeIO = pipe;
}

t_dataDispositivos *buscarDispIO(char *idIO){
	extern t_dictionary *dispIO;
	t_dataDispositivos *dataDisp = dictionary_get(dispIO, idIO);
	return dataDisp;
}

void cambiarEstadoProg(t_PCB *PCB, int nuevoEstado){
	extern t_list *listaProgramas;
	t_nodoPrograma *nodoProgBuscado;

	int _is_PCB(t_nodoPrograma *nodoProg) {
		return (nodoProg->idPrograma) == (PCB->pid);
	}
	pthread_mutex_lock(&mutexListaProgramas);
	nodoProgBuscado = list_find(listaProgramas, (void*) _is_PCB);
	nodoProgBuscado->estado = nuevoEstado;
	pthread_mutex_unlock(&mutexListaProgramas);
}

void planificarExec(){
	//t_nodoReady* nodo = malloc(sizeof(t_nodoReady));
	extern t_queue *ready;
	extern int quantum;
	extern t_list *exec;
	extern int programasEsperando;
	extern int CPUEsperando;


	pthread_mutex_lock(&mutexReady);
	t_nodoReady *nodo = queue_pop(ready);
	pthread_mutex_unlock(&mutexReady);

	programasEsperando--;
	t_PCB * PCBaPlanificar = nodo->idPCB; //en realidad no es el id, es un t_pcb
	t_nodoExec *nodoEjecucion = malloc(sizeof(t_nodoExec));
	nodoEjecucion->idPCB = PCBaPlanificar;
	nodoEjecucion->qRestante = quantum;
	nodoEjecucion->socketCpu = buscarCPUDisponible();
	list_add(exec, nodoEjecucion);
	cambiarEstadoProg(PCBaPlanificar, 2);
	CPUEsperando--;
	imprimirListas();

	/*Mando a la CPU a que ejecute*/
	ejecutar(PCBaPlanificar, nodoEjecucion->socketCpu);

	free(nodo); // el malloc lo hice en planificarReady;
}

void ejecutar(t_PCB *PCB, int socketCPU){
	//int i;
	mensaje_t mensaje;
	mensaje.codigo = EJECUTAR_INSTRUCCION;
	mensaje.extra = NULL;
	mensaje.tam_extra = 0;
	mensaje.cantidad = 9;
	unsigned *PCBdesarmada = malloc(sizeof(unsigned)*mensaje.cantidad);
	//mensaje.parametros = malloc(sizeof(unsigned)*mensaje.cantidad);

	PCBdesarmada[0] = PCB->pid;
	PCBdesarmada[1] = PCB->segmento_codigo;
	PCBdesarmada[2] = PCB->segmento_stack;
	PCBdesarmada[3] = PCB->pointer_stack;
	PCBdesarmada[4] = PCB->indice_codigo;
	PCBdesarmada[5] = PCB->indice_etiquetas;
	PCBdesarmada[6] = PCB->pc;
	PCBdesarmada[7] = PCB->tam_contexto_actual;
	PCBdesarmada[8] = PCB->tam_indice_etiquetas;
	mensaje.parametros = PCBdesarmada;

	/*i = */enviar_mensaje(socketCPU, mensaje);

	log_debug(log_kernel, "PCP: Envio a ejecutar el programa: %d a la CPU: %d.", PCB->pid, socketCPU);
	destruir_mensaje(&mensaje);
}

int buscarCPUDisponible(void){
	t_nodoCPU *cpuBuscada;
	extern t_list *listaCPU;

	int _is_CPU(t_nodoCPU *nodoCPU) {
		return ((nodoCPU->estado) == (0));
	}
	cpuBuscada = list_find(listaCPU, (void*) _is_CPU);
	cambiarEstado(cpuBuscada->socketCPU);

	return cpuBuscada->socketCPU;
}

void seg_fault(int sock, int cod){
	extern int CPUEsperando;
	extern int programasEsperando;

	cambiarEstado(sock);
	planificarExit(sock, cod, "------Segmentation Fault------");
	CPUEsperando++;
	while ((programasEsperando > 0) && (CPUEsperando > 0)){
		planificarExec();
	}
}

void stack_fault(int sock, int fault){
	extern int CPUEsperando;
	extern int programasEsperando;

	if (fault){
		planificarExit(sock, 0, "------ STACK UNDERFLOW ------");
	}else{
		planificarExit(sock, 0, "------ STACK OVERFLOW ------");
	}
	CPUEsperando++;
	while ((programasEsperando > 0) && (CPUEsperando > 0)){
		planificarExec();
	}
}

void finPrograma(int socket, char *mensajeTerminacion){
	/*Manda al programa el estado final de sus variables
	 *
	 * Saca el PCB de EXEC, lo pasa a lista de EXIT, suma CPU_OK
	 * planificar()
	 * */
	extern int programasEsperando;
	extern int CPUEsperando;

	cambiarEstado(socket);
	planificarExit(socket, 1, mensajeTerminacion);
	CPUEsperando++;
	while((programasEsperando > 0) && (CPUEsperando > 0)){
		planificarExec();
	}

}

void actualizarPCB(mensaje_t mensaje){
	int idPrograma = mensaje.parametros[0];
	extern t_list *listaProgramas;

	int _is_idPCB(t_nodoPrograma *nodoProg) {
		return (nodoProg->idPrograma) == (idPrograma);
	}

	pthread_mutex_lock(&mutexListaProgramas);
	t_nodoPrograma *nProg = list_find(listaProgramas, (void*) _is_idPCB);

	nProg->PCB->pointer_stack = mensaje.parametros[3];
	nProg->PCB->pc = mensaje.parametros[6];
	nProg->PCB->tam_contexto_actual = mensaje.parametros[7];
	pthread_mutex_unlock(&mutexListaProgramas);
	// 0pid, 1segmento_codigo, 2segmento_stack, 3stack_pointer, 4indice_codigo, 5indice_etiqueta, 6pc, 7tam_contexto_actual, 8tam_indice_etiquetas
	// no hace falta que actualize tudo
}



void analizarQuantumRestante(int socket, int estadoCPU){
	extern t_list *exec;
	extern int CPUEsperando;
	extern int programasEsperando;

	int _is_socketCPU(t_nodoExec *nodoE) {
		return (nodoE->socketCpu) == (socket);
	}
	t_nodoExec *nodoExecBuscado = list_find(exec, (void *) _is_socketCPU);
	nodoExecBuscado->qRestante--;


	if (estadoCPU == 1){
		nodoExecBuscado->qRestante = 0;
	}
	if (nodoExecBuscado->qRestante >= 0){
		log_debug(log_kernel, "\tPCP: QUANTUM RESTANTE: %d.\n", nodoExecBuscado->qRestante);
		if (nodoExecBuscado->qRestante > 0){
			/*Tiene que ejecutar el mismo programa en la misma CPU*/
			ejecutar(nodoExecBuscado->idPCB, socket);
		}
		else if(nodoExecBuscado->qRestante == 0){
		/*El programa vuelve a Ready, sumo programas esperando y CPU esperando y planifico*/
		t_PCB *PCB = sacarPCBdeExec(socket);
		cambiarEstado(socket);
		CPUEsperando++;
		if (estadoCPU == 1){
			eliminarCPU(socket);
			CPUEsperando--;
			FD_CLR(socket, &setPCP);
		}
		planificarReady(PCB);
		}
	} else{
		//El programa ya esta en Bloq porque la bloqueo un semaforo o un IO
		cambiarEstado(socket);
		sacarPCBdeExec(socket);
		CPUEsperando++;
		while((CPUEsperando > 0) && (programasEsperando > 0)){
			planificarExec();
		}

	}
	imprimirListas();
}

/*void desconectarCPU(int socketCPU){
	extern t_list *listaCPU;
	int _is_CPU(t_nodoCPU *nodoCPU) {
		return (nodoCPU->socketCPU) == (socketCPU);
	}
	t_nodoCPU *nCPU = list_remove_by_condition(listaCPU, (void*) _is_CPU);
}*/

void planificarReady(t_PCB *pcb){
	extern t_queue *ready;
	extern int programasEsperando;
	extern int CPUEsperando;
	t_nodoReady * nReady = malloc(sizeof(t_nodoReady));
	nReady->idPCB = pcb;

	pthread_mutex_lock(&mutexReady);
	queue_push(ready, nReady);
	pthread_mutex_unlock(&mutexReady);

	cambiarEstadoProg(pcb, 1);
	programasEsperando ++;
	imprimirListas();
	while((programasEsperando > 0) && (CPUEsperando > 0)){
		planificarExec();
	}

}

void enviar_imprimir(char *mensaje, int codigo, int socketCPU){
	extern int pipeplp[2];
	t_mensaje_pipe *mensajePipe = malloc(sizeof(t_mensaje_pipe));
	mensajePipe->texto = malloc(strlen(mensaje)); //por ahi esta de mas
	mensajePipe->texto = mensaje;
	t_PCB *PCB = buscarPCBporIDenExec(socketCPU);
	mensajePipe->idPCB = PCB->pid;
	if (codigo == 0){
		mensajePipe->codigo = SYS_IMPR_PIPE;
	}
	else{
		mensajePipe->codigo = SYS_IMPR_TEXTO_PIPE;
	}
	write(pipeplp[1], mensajePipe, sizeof(t_mensaje_pipe));
	log_info(log_kernel, "PCP: Mando al PLP el codigo IMPRIMIR.");
	log_debug(log_kernel, "PCP: \n\tCodigo: %d.\tTexto: %s.\tPID: %d.", mensajePipe->codigo, mensajePipe->texto, mensajePipe->idPCB);

	/*free(mensajePipe->texto);
	free(mensajePipe);*/ //Lo deberia hacer el PLP
}

void asignarValor(char *idVarGlobalFalso, int valor, int socket){
	extern t_dictionary *var_compartidas;

	mensaje_t mensajeRespuesta;
	//memcpy(get_valor, &valor, sizeof(int));						// esto?
	//dictionary_put(var_compartidas, idVarGlobal, (void *)valor);	//hace falta? :)
	char * idVarGlobal = strchr(idVarGlobalFalso, '\n') ? strndup(idVarGlobalFalso, strchr(idVarGlobalFalso, '\n') - idVarGlobalFalso) : strdup(idVarGlobalFalso);
	if (!(dictionary_has_key(var_compartidas, idVarGlobal))){
		mensajeRespuesta.codigo = VARIABLE_NO_EXISTE;
		mensajeRespuesta.cantidad = 0;
		mensajeRespuesta.parametros = NULL;
		mensajeRespuesta.extra = NULL;
		mensajeRespuesta.tam_extra = 0;
		enviar_mensaje(socket, mensajeRespuesta);
		log_info(log_kernel, "PCP: \tNo se pudo asignar el valor, la variable compartida %s no existe.\n", idVarGlobal);
	}else {
		mensajeRespuesta.codigo = SYS_ASIGNAR_OK;

		t_dataVarCompartidas *estVarComp = dictionary_get(var_compartidas, idVarGlobal);
		estVarComp->valorVariable = valor;
		//dictionary_put(var_compartidas, idVarGlobal, estVarComp);

		mensajeRespuesta.cantidad = 1;
		mensajeRespuesta.parametros = malloc(sizeof(int));
		mensajeRespuesta.parametros[0] = estVarComp->valorVariable;
		mensajeRespuesta.extra = strdup("");
		mensajeRespuesta.tam_extra = 0;
		enviar_mensaje(socket, mensajeRespuesta);
		destruir_mensaje(&mensajeRespuesta);
		log_info(log_kernel, "PCP: \tSe pudo asignar el valor a Var. Compartida: %s\t Valor: %d.\n", idVarGlobal, estVarComp->valorVariable);
	}
}

void obtenerValor(char *idVarGlobalFalso,int socket){
	extern t_dictionary *var_compartidas;
	char * idVarGlobal = strchr(idVarGlobalFalso, '\n') ? strndup(idVarGlobalFalso, strchr(idVarGlobalFalso, '\n') - idVarGlobalFalso) : strdup(idVarGlobalFalso);

	mensaje_t respuesta;
	respuesta.tam_extra = 0;
	respuesta.extra = NULL;
	if (!(dictionary_has_key(var_compartidas, idVarGlobal))){
		respuesta.codigo = VARIABLE_NO_EXISTE;
		/*int i = 0;
		valorAux = &i;*/
		log_info(log_kernel, "PCP: \tValor Obtenido de Variable Compartida %s: NULO, no existe.\n", idVarGlobal);
		respuesta.cantidad = 0;
		respuesta.parametros = NULL;
		enviar_mensaje(socket, respuesta);
	}else{
		//dictionary_put(var_compartidas, idVarGlobal, (void*) i);
		t_dataVarCompartidas *estVarComp = (t_dataVarCompartidas *) dictionary_get(var_compartidas, idVarGlobal);
		respuesta.cantidad = 1;
		respuesta.codigo = SYS_VALOR_OBTENIDO;
		log_info(log_kernel, "PCP: \tValor Obtenido de Variable Compartida %s: %d.\n", idVarGlobal, estVarComp->valorVariable);
		respuesta.parametros = malloc(sizeof(int));
		respuesta.parametros[0] = estVarComp->valorVariable;
		respuesta.extra = strdup("");
		enviar_mensaje(socket, respuesta);
		destruir_mensaje(&respuesta);
	}
	//memcpy(respuesta.parametros, param, sizeof(int));				//Revisar
	//respuesta.parametros[0] = &param;
}

void wait_semaforo(char *idSemFalso, int socketCPU){
	extern t_dictionary *semaforos;
	extern int CPUEsperando;
	extern int programasEsperando;
	extern t_list *exec;
	mensaje_t respuesta;
	respuesta.cantidad = 0;
	respuesta.parametros = NULL;
	respuesta.tam_extra = 0;
	respuesta.extra = NULL;

	char * idSem = strchr(idSemFalso, '\n') ? strndup(idSemFalso, strchr(idSemFalso, '\n') - idSemFalso) : strdup(idSemFalso);

	if ((dictionary_has_key(semaforos, idSem))){
		t_dataSemaforo *data = (t_dataSemaforo *)dictionary_get(semaforos, idSem);
		data->valor--;

		if (data->valor < 0){ // bloqueo
			/*mensaje_t mensajeConPCB;							//Despues de un wait si o si me manda un fin_instruccion
			recibir_mensaje(socketCPU, &mensajeConPCB);			//Este recibir me lo detecta el select?? no quiero que pase eso
			actualizarPCB(mensajeConPCB);						*//*Otra opcion es poner el quantum restante en negativo
			y cuando hago un fin_instruccion me fijo si es negativo quiere decir que ya lo bloquee y no tengo que pasarlo a ready*/
			int _is_socketCPU(t_nodoExec *nodoExec) {
				return ((nodoExec->socketCpu) == (socketCPU));
			}
			t_nodoExec *nodoBuscado = list_find(exec, (void*) _is_socketCPU);
			nodoBuscado->qRestante = -1;

			//t_PCB *pcbBloq = buscarPCBporIDenExec(socketCPU);	//Busco PCB de exec
			queue_push(data->progBloq, nodoBuscado->idPCB);				//La Bloqueo
			cambiarEstadoProg(nodoBuscado->idPCB, 4);
			//dictionary_put(semaforos, idSem, data);				//Hace falta?
			imprimirListas();
			log_info(log_kernel, "PCP: \tHubo un programa que tuvo que ser bloqueado.\tPrograma: %d.\n", nodoBuscado->idPCB->pid);

		}
		respuesta.codigo = SYS_WAIT;
		enviar_mensaje(socketCPU, respuesta);

	}else {
		log_info(log_kernel, "PCP: \tEl Semaforo: %s no esta definido.\n", idSem);
		respuesta.codigo = SEMAFORO_NO_EXISTE;
		enviar_mensaje(socketCPU, respuesta);

	}
}

void cambiarEstado(int socketCPU){
	extern t_list *listaCPU;
	int _is_CPU(t_nodoCPU *nodoCPU) {
		return (nodoCPU->socketCPU) == (socketCPU);
	}
	t_nodoCPU *nCPU = list_find(listaCPU, (void*) _is_CPU);
	nCPU->estado = !(nCPU->estado);
}

int signal_semaforo(char *idSemaforoFalso, int socketCPU){
	extern t_dictionary *semaforos;
	mensaje_t respuesta;
	respuesta.cantidad = 0;
	respuesta.parametros = NULL;
	respuesta.tam_extra = 0;
	respuesta.extra = NULL;
	char * idSemaforo = strchr(idSemaforoFalso, '\n') ? strndup(idSemaforoFalso, strchr(idSemaforoFalso, '\n') - idSemaforoFalso) : strdup(idSemaforoFalso);

	if ((dictionary_has_key(semaforos, idSemaforo))){
		t_dataSemaforo *data = (t_dataSemaforo *) dictionary_get(semaforos, idSemaforo); // el key de semaforos es un char*?
		data->valor++;
		respuesta.codigo = SYS_WAIT;
		enviar_mensaje(socketCPU, respuesta);

		if (!queue_is_empty(data->progBloq)){
			desbloquearPCB_sem(idSemaforo);
			log_info(log_kernel, "PCP: \tHubo un programa que tuvo que ser desbloqueado.");
			return 1;
		}
	}else{
		respuesta.codigo = SEMAFORO_NO_EXISTE;
		enviar_mensaje(socketCPU, respuesta);
		log_info(log_kernel, "PCP: \tEl Semaforo: %s no esta definido.\n", idSemaforo);
	}
	return 0;
}

void desbloquearPCB_sem(char *idSem){
	extern t_dictionary *semaforos;
	extern t_queue *ready;
	t_nodoReady *nodoR = malloc(sizeof(t_nodoReady));
	t_dataSemaforo *data = (t_dataSemaforo *)dictionary_get(semaforos, idSem); //es al pedo hacer get y set de esto?
	nodoR->idPCB = queue_pop(data->progBloq);

	log_info(log_kernel, "PCP: \tPrograma desbloqueado: %d.\n", nodoR->idPCB->pid);

	//dictionary_put(semaforos, idSem, data); hace falta?
	pthread_mutex_lock(&mutexReady);
	queue_push(ready, nodoR);
	pthread_mutex_unlock(&mutexReady);
	imprimirListas();
	cambiarEstadoProg(nodoR->idPCB, 1);
}

t_PCB *buscarPCBporIDenExec(int socket){
	extern t_list *exec;
	int _is_PCB(t_nodoExec *nodoExec) {
		return (nodoExec->socketCpu) == (socket);
	}
	t_nodoExec *nodoExec = list_find(exec, (void *) _is_PCB);
	return nodoExec->idPCB;
}

t_PCB *sacarPCBdeExec(int socket){
	extern t_list *exec;

	int _is_PCB(t_nodoExec *nodoExec) {
		return (nodoExec->socketCpu) == (socket);
	}
	t_nodoExec *nodoPCBBuscado = list_remove_by_condition(exec, (void *) _is_PCB);

	t_PCB *PCB = nodoPCBBuscado->idPCB;


	free(nodoPCBBuscado); //malloc en planificarExec
	return PCB;
}

void planificarExit(int socketCPU, int codigo, char *texto){
	/* Puedo usar un mensaje para saber que tipo de finalizacion fue y guardar los datos ahi
	 * Sacar de exec
	 * Pasar a exit
	 * Decirle al PLP que hay un programa en exit
	 * */
	extern t_list *exitList;
	extern int pipeplp[2];
	t_PCB *PCBaPlanificar = sacarPCBdeExec(socketCPU);
	t_nodoExit *nExit = malloc(sizeof(t_nodoExit));
	t_mensaje_pipe *mensajePipe = malloc(sizeof(t_mensaje_pipe));

	nExit->pcb = PCBaPlanificar;
	//nExit->mensajeDePCP->codigo_a_enviar = codigo;
	//strcpy(nExit->mensajeDePCP->texto, texto);
	//nExit->mensajeDePCP->texto = strdup(texto);
	pthread_mutex_lock(&mutexExit);
	list_add(exitList, nExit);
	pthread_mutex_unlock(&mutexExit);

	cambiarEstadoProg(PCBaPlanificar, 5);
	imprimirListas();

	mensajePipe->codigo = COD_EXIT_PIPE;
	mensajePipe->texto = strdup(texto);
	mensajePipe->idPCB = PCBaPlanificar->pid;
	log_info(log_kernel, "PCP: \tPrograma Terminado con PID: %d \t || Motivo: %s \n -Mando a PLP a eliminarlo-", nExit->pcb->pid, texto);
	write(pipeplp[envio], mensajePipe, sizeof(t_mensaje_pipe));
}

void eliminarCPU(int socketCPU){
	//t_nodoCPU *nodoCPUBuscado = malloc(sizeof(t_nodoCPU));
	extern t_list *listaCPU;

	int _is_CPU(t_nodoCPU *nodoCPU) {
		return (nodoCPU->socketCPU) == (socketCPU);
	}
	t_nodoCPU *nodoCPUBuscado = list_remove_by_condition(listaCPU, (void*) _is_CPU);

	free(nodoCPUBuscado); //malloc en aceptar_conexion
}


void desconectarPrograma(int idPCB){
	extern t_list *listaProgramas;

	int _buscarNodoPrograma(t_nodoPrograma *nodoProg) {
		return ((nodoProg->idPrograma) == (idPCB));
	}
	//t_nodoPrograma *nodoBuscado = list_find(listaProgramas, (void*) _buscarNodoPrograma);

	//sacarDeLista(nodoBuscado->PCB, nodoBuscado->estado); SE COMPLICO

	//nodoBuscado->estado = 5;

}

void sacarDeLista(t_PCB *PCB, int listaActual){
	switch(listaActual){
		case 1: //Esta en Ready: le pongo el estado en 5 y chequeo antes de planificarExec
			break;
		case 2: //Esta en Exec: le pongo el estado en 5 y chequeo antes de planificarExec -- Posible error cuando la cpu me devuelva algo que requiera del programa
			break;
		case 3:
			break;
		case 4:
			break;
		default:
			break;
	}
}


void *pcp(void *basura) {
	log_info(log_kernel, "PCP: Empiezo.\n");
	fd_set tempPCP;
	int socketListen;
	int i;

	mensaje_t mensaje;

	extern fd_set setPCP;
	extern int fdMax;
	extern int puertoCPU;
	extern int pipepcp[2];
	extern int pipeplp[2];
	extern int CPUEsperando;
	extern int programasEsperando;
	extern t_list* listaCPU;
	/*extern t_list* listaPCB;
	extern t_list* exec;
	extern t_list* exitList;
	extern t_queue *ready;
	extern int ** pipeio;
	extern int quantum;*/

	CPUEsperando = 0;
	programasEsperando = 0;

	FD_ZERO(&setPCP);
	FD_ZERO(&tempPCP);

	/*Creo socket de escucha de Programas*/
	socketListen = crear_socket_listen(puertoCPU);
	log_info(log_kernel, "PCP: Se crea el socket de escucha para CPU");
	log_debug(log_kernel, "PCP: Sockets de escucha para CPU en el puerto: %d, socket: %d.\n", puertoCPU, socketListen);

	FD_SET(socketListen, &setPCP);
	FD_SET(pipepcp[recepcion], &setPCP);

	/*Obtengo mayor de los 2 sockets y ponerlo como fdMax*/
	if (socketListen > pipeplp[recepcion]){
		fdMax = socketListen;
	} else {
	    fdMax = pipeplp[recepcion];
	}

	for(;;){
		tempPCP = setPCP;
	    if (select (fdMax+1, &tempPCP, NULL, NULL, NULL) == -1){
	    	perror("Error en el select. Hilo PCP.\n");
	    }
	    for (i = 0; i <= fdMax; i++){
	      if(FD_ISSET(i, &tempPCP)){
	    	  if ((i == pipepcp[recepcion]) || (i == socketListen)){
	    		  if (i == pipepcp[recepcion]){
	    			  log_debug(log_kernel, "PCP: Mensaje en el Pipe del PLP o E/S, desde el socket: %d.\n", i);
	    			  /*Llega un mensaje del PLP o de algun IO*/

	    			  t_mensaje_pipe *codigoMensajeRecibido = malloc(sizeof(t_mensaje_pipe));
	    			  read(pipepcp[recepcion], codigoMensajeRecibido, sizeof(t_mensaje_pipe));
	    			  //log_info(log_kernel, "Codigo de Mensaje: %d.\n", codigoMensajeRecibido->codigo);
	    			  switch(codigoMensajeRecibido->codigo){
	    			  	  case PROG_READY_PIPE:
	    			  		  log_info(log_kernel, "PCP: \tHay un nuevo programa en Ready. CODIGO: %d.\n", codigoMensajeRecibido->codigo);
	    			  		  programasEsperando++;
	    			  		  while((programasEsperando > 0) && (CPUEsperando > 0)){
	    			  			  planificarExec();
	    			  		  }
	    			  		  free(codigoMensajeRecibido);
	    			  		  break;
	    			  	  case IO_TERMINA:
	    			  		  codigoMensajeRecibido->texto[codigoMensajeRecibido->numero] = '\0';
	    			  		  log_info(log_kernel, "PCP: \tUn dispositivo de IO termino de atender un Programa. CODIGO: %d \tDISPOSITIVO: %s.\n", codigoMensajeRecibido->codigo, codigoMensajeRecibido->texto);
	    			  		  desbloquearPCB_IO(codigoMensajeRecibido->texto, codigoMensajeRecibido->idPCB);
	    			  		  /*programasEsperando++;
	    			  		  log_info(log_kernel, "\tLa cantidad de programas esperando es: %d.\n", programasEsperando);
	    			  		  log_info(log_kernel, "\tLa cantidad de CPU esperando es: %d.\n", CPUEsperando);
	    			  		  while((programasEsperando > 0) && (CPUEsperando > 0)){
	    			  			  planificarExec();
	    			  		  }*/
	    			  		  free(codigoMensajeRecibido->texto);
	    			  		  free(codigoMensajeRecibido);
	    			  		  break;
	    			  	  case ELIMINAR_PROGRAMA:
	    			  		  log_info(log_kernel, "PCP: \tSe desconecto un Programa. CODIGO: %d \tPROGRAMA: %d.\n", codigoMensajeRecibido->codigo, codigoMensajeRecibido->idPCB);
	    			  		  desconectarPrograma(codigoMensajeRecibido->idPCB);
	    			  		  free(codigoMensajeRecibido);
	    			  		  break;
	    			  	  case HANDSHAKE_IO:
	    			  		  asignarPipe(codigoMensajeRecibido->texto, codigoMensajeRecibido->numero);
	    			  		  free(codigoMensajeRecibido->texto);
	    			  		  free(codigoMensajeRecibido);
	    			  		  break;
	    			  	  default: break;
	    			  }
	    		  }
	    		  if (i == socketListen){
	    			  /*Acepto nuevos CPU*/
	    			  aceptar_conexion_CPU(socketListen, listaCPU);
	    			  CPUEsperando++;
	    			  /*Reviso si puedo planificar (si tengo en Ready programas en espera)*/
	    			  while((programasEsperando > 0) && (CPUEsperando > 0)){
	    				  planificarExec();
	    			  }
	    		  }
	    	  } else{
	    		  /*Un CPU viejo quiere hablarme*/
	    		  log_debug(log_kernel, "PCP: Mensaje de la CPU en socket: %d.\n", i);
	    		  if(recibir_mensaje(i, &mensaje)==-1){
	    			  log_info(log_kernel, "PCP: CPU desconectada, se elimina de la lista y se cierra su conexion.\n");
	    			  int j;
	    			  for(j = 0; j < list_size(listaCPU); j++){
	    				  t_nodoCPU *nodoCPU = list_get(listaCPU,j);
	    				  if(nodoCPU->socketCPU == i){
	    					  if (nodoCPU->estado == 1){												//Se fija si la CPU tiene un programa asignado
	    						  planificarExit(nodoCPU->socketCPU, COD_EXIT_PIPE, "-----Desconexion de mi CPU-----");		//Busca y saca de exit, Notificacion al programa (desconexion), planifica exit, le avisa a PLP que hay un nuevo en exit
	    						  }
	    					  else {
	    						  CPUEsperando--;
	    					  }
	    					  eliminarCPU(nodoCPU->socketCPU);											//Elimino CPU de la lista de CPU's, y no planifico Ready, porque no agregue una cpu libre.
	    					  break;
	    				  }else {
	    					  //log_warning(log_kernel, "Se desconecto un socket que no era una CPU, socket: %d.\n", i);
	    				  }
	    			  }
	    			  FD_CLR(i, &setPCP);
	    			  close(i);

	    		  } else{
	    			  analizarRespuesta(mensaje, i);
	    		  }
	    	  }
	      }
	    }
	  }
	return NULL;
}
