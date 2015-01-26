#include "kernel.h"
#define IP_UMV "IP_UMV"

int pipepcp[2],pipeplp[2];
//int **pipeio; // pipeio[IDdisp][envio]

/*int cantidadIO;
t_IO *dispositivosIO;*/

// cuando llegue programa le damos su id y lo metemos en diccionario con su socket
/*Diccionarios Globales*/
t_dictionary *var_compartidas;
t_dictionary *semaforos;
t_dictionary *dispIO;

// diccionario o lista de CPU's

/* Variables Globales*/
int puertoUMV;
int puertoProg;
int puertoCPU;
int quantum;
int socketBusqueda;
int tam_stack;
int grado_multi;
char *ipUMV;
int retardo_CPU;
t_log *log_kernel;
t_log *log_estadoListas;
pthread_mutex_t mutexNew;
pthread_mutex_t mutexReady;
pthread_mutex_t mutexBloqIO;
pthread_mutex_t mutexBloqSem;
pthread_mutex_t mutexExec;
pthread_mutex_t mutexExit;
pthread_mutex_t mutexListaProgramas;
pthread_mutex_t mutexArchivoLog;

/* Listas Globales*/
t_list *listaProgramas;  // pcb + socket
t_list *listaCPU;  // puede no ser global 
t_list *new;
t_queue *ready;
t_list *exec;   // puede no ser global
t_list *exitList;



void crearPipes(void) {
	pipe(pipepcp);
	pipe(pipeplp);
	/*pipeio = malloc(sizeof(int *) * cantidadIO);
	int dispIO;
	for (dispIO = 0; dispIO < cantidadIO; dispIO ++) {
		pipeio[dispIO] = malloc(2 * sizeof(int));
		pipe(pipeio[dispIO]);
	}
	*/
}

void crearThreads(void) {
	log_info(log_kernel, "KERNEL: Se crean los hilos de PLP y PCP.");
	pthread_t threadID;
	pthread_create(&threadID, NULL, &pcp, NULL);
	pthread_detach(threadID);
	pthread_create(&threadID, NULL, &plp, NULL);
	pthread_detach(threadID);
	/*int dispositivo;
	for (dispositivo = 0; dispositivo < cantidadIO; dispositivo ++) {
		pthread_create(&threadID, NULL, &io, dispositivosIO + dispositivo);
		pthread_detach(threadID);
	}*/
}

t_archConfig *leer_archivo_configuracion(char *path){

	t_config *archivo;
	archivo = config_create(path);
	char**array_sem=malloc(sizeof(char**));
	t_archConfig *config_kernel = malloc(sizeof(t_archConfig));
	config_kernel->puertoUMV = config_get_int_value(archivo, PUERTO_UMV);
	config_kernel->puertoPrograma=config_get_int_value(archivo, PUERTO_PROG);
	config_kernel->quantum = config_get_int_value(archivo, QUANTUM);
	config_kernel->ip_umv = config_get_string_value(archivo,IP_UMV);
	config_kernel->retardo_CPU =config_get_int_value(archivo,RETARDO_CPU);
	config_kernel->gradoMulti =config_get_int_value(archivo,MULTIPROGRAMACION);
	config_kernel->puertoCPU = config_get_int_value(archivo,PUERTO_CPU);
	config_kernel->tamaStack = config_get_int_value(archivo,STACK);
	config_kernel->nivel_log = config_get_int_value(archivo, NIVEL_LOG);
	array_sem = config_get_array_value(archivo,SEMAFOROS);
	char**array_sem_valores = config_get_array_value(archivo,VALOR_SEMAFORO);
	char**array_var_globales = config_get_array_value(archivo, VAR_GLOBALES);
	char**array_id_io = config_get_array_value(archivo,ID_HIO);
	char**array_io_valores = config_get_array_value(archivo,HIO);
	config_kernel->dispositivosIO = obtener_diccionario_dispositovos(array_io_valores,array_id_io);
	config_kernel->semaforos = obtener_diccionario_semaforos(array_sem,array_sem_valores);
	config_kernel->varGlobales = obtener_diccionario_var_globales(array_var_globales);

	free(array_id_io);
	free(array_io_valores);
	free(array_sem);
	free(array_var_globales);
	free(array_sem_valores);


	return config_kernel;
}

t_dictionary* obtener_diccionario_dispositovos(char**valores_io,char** id_io){

	t_dictionary *dispositivos;
	dispositivos = dictionary_create();
	int valor=0;
	int retardo;

	while(id_io[valor]!=NULL){
		//pipe(pipeIO);
		t_dataDispositivos *estructura = malloc(sizeof(t_dataDispositivos));
		estructura->id_IO = strdup(id_io[valor]);
		retardo = atoi(valores_io[valor]);
		estructura->retardoInicial=retardo;
		estructura->progBloq = list_create();
		estructura->flag_uso = 0;
		/*estructura->pipeIO = malloc(sizeof(int*) * 2);*/
		//estructura->pipeIO = NULL;
		dictionary_put(dispositivos,id_io[valor],estructura);
		/*
		t_hilos_param*data=malloc(sizeof(t_hilos_param));
		data->id=malloc(strlen(id_io[valor]));
		data->data=estructura;
		data->id=id_io[valor];
		*/
		pthread_t threadIO_id;
		pthread_create(&threadIO_id,NULL,(void*)io,(void*)estructura);
		pthread_detach(threadIO_id);
		valor++;
	}
return dispositivos;
}


t_dictionary* obtener_diccionario_semaforos(char**id_semaforos,char** valor_semaforo){

	t_dictionary *semaforos;
	semaforos = dictionary_create();
	int valor=0;
	while(id_semaforos[valor]!=NULL){
		t_dataSemaforo *estructura=malloc(sizeof(t_dataSemaforo));
		int valor_sem=atoi(valor_semaforo[valor]);
		estructura->id_Sem = strdup(id_semaforos[valor]);
		estructura->valor=valor_sem;
		estructura->progBloq=queue_create();
		dictionary_put(semaforos,id_semaforos[valor],estructura);
		valor++;
	}
	return semaforos;
}


t_dictionary* obtener_diccionario_var_globales(char** var_globales){

	t_dictionary *var_compartidas = dictionary_create();
	int i = 0;
	while(var_globales[i] != NULL){
		t_dataVarCompartidas *estVarComp = malloc(sizeof(t_dataVarCompartidas));
		estVarComp->valorVariable = 0;
		dictionary_put(var_compartidas, var_globales[i], estVarComp);
		i++;
	}
	return var_compartidas;
}

void *liberar_t_dataDispositivos(t_dataDispositivos* data){

	free(data->id_IO);
	free(data);
	return NULL;
}
void *liberar_t_dataSemaforo(t_dataSemaforo*data){
	queue_clean_and_destroy_elements(data->progBloq, free);
	free(data);

return NULL;
}

void imprimirListas(){
	extern t_list *new;
	extern t_queue *ready;
	extern t_dictionary *dispIO;
	extern t_dictionary *semaforos;
	extern t_list *exec;
	extern t_list *exitList;
	extern t_list *listaCPU;
	char *buffer = string_new();


	pthread_mutex_lock(&mutexNew);
	pthread_mutex_lock(&mutexReady);
	pthread_mutex_lock(&mutexBloqIO);
	pthread_mutex_lock(&mutexBloqSem);
	pthread_mutex_lock(&mutexExec);
	pthread_mutex_lock(&mutexExit);



	string_append(&buffer, "Cola de Nuevos:\t");
	void imprimirNew(t_nodoNew *nodoNew){
		string_append(&buffer, string_itoa(nodoNew->PCB->pid));
		string_append(&buffer, "[");
		string_append(&buffer, string_itoa(nodoNew->peso));
		string_append(&buffer, "]\t|\t");
	}
	list_iterate(new, (void *)imprimirNew);
	pthread_mutex_unlock(&mutexNew);

	string_append(&buffer, "\n");

	string_append(&buffer, "Cola de Listos:\t");
	void imprimirReady(t_nodoReady *nodoReady){
		string_append(&buffer, string_itoa(nodoReady->idPCB->pid));
		string_append(&buffer, "\t|\t");
	}
	list_iterate(ready->elements, (void *)imprimirReady);
	pthread_mutex_unlock(&mutexReady);


	string_append(&buffer, "\n");


	string_append(&buffer, "Lista de Bloqueados por Dispositivo:\n");
	void imprimirBloqIO(t_nodoBloqIO *nodoBloqIO){
		string_append(&buffer, string_itoa(nodoBloqIO->PCB->pid));
		string_append(&buffer, "\t|\t");
	}
	void imprimirDicIO(char *idIO, t_dataDispositivos *dataDisp){
		string_append(&buffer, "\t");
		string_append(&buffer, dataDisp->id_IO);
		string_append(&buffer, ":\t");
		list_iterate(dataDisp->progBloq, (void *)imprimirBloqIO);
		string_append(&buffer, "\n");
	}
	dictionary_iterator(dispIO, (void *)imprimirDicIO);
	pthread_mutex_unlock(&mutexBloqIO);


	string_append(&buffer, "Lista de Bloqueados por Semaforo:\n");
	void imprimirBloqSem(t_PCB *PCB){
		string_append(&buffer, string_itoa(PCB->pid));
		string_append(&buffer, "\t|\t");
	}
	void imprimirDicSem(char *idSem, t_dataSemaforo *dataSem){
		string_append(&buffer, "\t");
		string_append(&buffer, dataSem->id_Sem);
		string_append(&buffer, ":[");
		string_append(&buffer, string_itoa(dataSem->valor));
		string_append(&buffer, "]\t");
		list_iterate(dataSem->progBloq->elements, (void *)imprimirBloqSem);
		string_append(&buffer, "\n");
	}
	dictionary_iterator(semaforos, (void *)imprimirDicSem);
	pthread_mutex_unlock(&mutexBloqSem);


	string_append(&buffer, "Lista de Ejecucion:\t");
	void imprimirExec(t_nodoExec *nodoExec){
		if (nodoExec->qRestante > 0){
			string_append(&buffer, string_itoa(nodoExec->idPCB->pid));
			string_append(&buffer, "[");
			string_append(&buffer, string_itoa(nodoExec->qRestante));
			string_append(&buffer, "]\t|\t");
		}
	}
	list_iterate(exec, (void *)imprimirExec);
	pthread_mutex_unlock(&mutexExec);

	string_append(&buffer, "\n");

	string_append(&buffer, "Lista de Salida:\t");
	void imprimirExit(t_nodoExit *nodoExit){
		string_append(&buffer, string_itoa(nodoExit->pcb->pid));
		string_append(&buffer, "\t|\t");
	}
	list_iterate(exitList, (void *)imprimirExit);
	pthread_mutex_unlock(&mutexExit);

	string_append(&buffer, "\nCantidad de CPU: ");
	string_append(&buffer, string_itoa(list_size(listaCPU)));
	string_append(&buffer,"\n---------------------------------------------------------------------------------\n");

	log_info(log_estadoListas, "Estado de Sistema: \n%s", buffer);
	free(buffer);
}

/*t_nodoReady*nodoPrueba = list_get(ready->elements, 0);
		log_info(log_kernel, "Primer elemento!!: %d", nodoPrueba->idPCB->pid);*/

/*void imprimirReady(t_nodoNew *nodoNew){
	t_nodoReady *nodoReady;
	extern char*buffer;

}*/



int main (int argc, char ** argv) {
	t_archConfig * t_kernel = leer_archivo_configuracion(PATH);

	/*Inicializo Listas*/
	listaProgramas = list_create();
	listaCPU = list_create();
	new = list_create();
	ready = queue_create();
	exec = list_create();
	exitList = list_create();

	/*Inicializo Variables Globales*/
	var_compartidas = t_kernel->varGlobales;
	semaforos = t_kernel->semaforos;
	dispIO = t_kernel->dispositivosIO;
	puertoUMV = t_kernel->puertoUMV;
	puertoProg = t_kernel->puertoPrograma;
	puertoCPU = t_kernel->puertoCPU;
	grado_multi = t_kernel->gradoMulti;
	ipUMV = t_kernel->ip_umv;
	retardo_CPU = t_kernel->retardo_CPU;
	tam_stack = t_kernel->tamaStack;
	quantum = t_kernel->quantum;

	//cantidadIO = dictionary_size(dispIO);

	pthread_mutex_init(&mutexNew, NULL);
	pthread_mutex_init(&mutexReady, NULL);
	pthread_mutex_init(&mutexBloqIO, NULL);
	pthread_mutex_init(&mutexBloqSem, NULL);
	pthread_mutex_init(&mutexExec, NULL);
	pthread_mutex_init(&mutexExit, NULL);
	pthread_mutex_init(&mutexListaProgramas, NULL);
	pthread_mutex_init(&mutexArchivoLog, NULL);


	log_kernel = log_create("log_kernel.log", "log_kernel.log", 0, (t_log_level) t_kernel->nivel_log);
	log_info(log_kernel, "KERNEL: Empieza el archivo de Log.");

	log_estadoListas = log_create("log_estadoListas.log", "log_estadoListas.log", 0, LOG_LEVEL_INFO);
	log_info(log_estadoListas,"-------------------------\nEmpieza el Log de Estado de Sistema Kernel.");


	crearPipes();
	crearThreads();

	/*for (i = 0; i < dictionary_size(var_compartidas); i++){
		printf("Diccionario de variables, Key: %s\tValor: %d.\n", dictionary_get())
	}*/
	while (1) sleep(-1); // duerme para siempre
	return 0;
}
