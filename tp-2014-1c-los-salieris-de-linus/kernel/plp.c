#include "kernel.h"
#include <parser/metadata_program.h>
#include "codigo_operacion.h"
#define BAJADA "\n"


extern t_log* log_kernel;
fd_set setPLP;
int idPrograma = 1;
int socketListen, socketUMV;
int grado_actual;

int segmentosReservados;
int estado;
//temporal

extern pthread_mutex_t mutexReady;
extern pthread_mutex_t mutexExit;
extern pthread_mutex_t mutexListaProgramas;
//extern pthread_mutex_t mutexArchivoLog;


//estructura de texto q envi el pcp

void *plp(void *basura) {
  fd_set tempPLP;
  int fdMax;
  char *codigo_literal;
  mensaje_t mensaje_recibido,mensaje_respuesta;
  extern fd_set setPLP;
  extern t_list *listaProgramas;
  extern int puertoProg;
  extern int puertoUMV;
  extern int socketUMV;
  extern char* ipUMV;
  extern int pipeplp[2];
  extern int segmentosReservados;
  extern int grado_actual;
  grado_actual=0;
  segmentosReservados=0;

  int i;
  int estado;
 // int offset=0;
  //int codigo_recibidos=0;
  FD_ZERO(&setPLP);
  FD_ZERO(&tempPLP);
  //log_kernel=log_create("/home/utnso/Escritorio/loggs.text","loggs.text",0,LOG_LEVEL_INFO);


  /*Creo socket de escucha de Programas*/
	socketListen = crear_socket_listen(puertoProg);
	
  /* Conecto a UMV*/

	socketUMV= obtener_socket();
	conectar_socket(puertoUMV,ipUMV,socketUMV);

	mensaje_respuesta.codigo=HANDSHAKE_KERNEL;
	mensaje_respuesta.cantidad=0;
	mensaje_respuesta.extra=NULL;
	mensaje_respuesta.tam_extra=0;
	mensaje_respuesta.parametros=NULL;
	enviar_mensaje(socketUMV,mensaje_respuesta);
	destruir_mensaje(&mensaje_respuesta);

  FD_SET(socketUMV,&setPLP);
  FD_SET(pipeplp[recepcion], &setPLP);
  FD_SET(socketListen, &setPLP);
  /*Obtengo mayor de los 3 sockets y ponerlo como fdMax*/

  if (socketUMV > pipeplp[recepcion]){					/*********Se tiene que verificar entre los 3 (para ver quien es mayor)*********/
    fdMax = socketUMV;
  } else {
    fdMax = pipeplp[recepcion];
  }


  while(1){
    tempPLP = setPLP;
    log_debug(log_kernel, "PLP: Esperando mensajes y conexiones...");
    if (select (fdMax+1, &tempPLP, NULL, NULL, NULL) == -1){
      log_error(log_kernel, "PLP: Error al invocar el select.");
    }


    for ( i = 0; i <= fdMax; i++){
      if(FD_ISSET(i, &tempPLP)){

    	  if(socketUMV ==i ){ /*Se cierra la UMV,en duda*/
            if (recibir_mensaje(socketUMV,&mensaje_recibido) == -1) {
              log_info(log_kernel, "PLP: Se cerro la Memoria Principal (UMV).");
              close(socketUMV);
              FD_CLR(socketUMV, &setPLP);
              exit(0);
            }
          }
          else if(i==pipeplp[recepcion]){

        	  	 analizar_msg_pipe();
          }

          else if(i== socketListen){ /*Acepto nuevos programas*/
            int nuevoFichero = aceptar_conexion(socketListen,listaProgramas, idPrograma);
            if(nuevoFichero !=0){
            FD_SET(nuevoFichero, &setPLP);
                if (nuevoFichero > fdMax) {
                	fdMax = nuevoFichero;
                }
            idPrograma++;
            }
          }
          else if   ((estado = recibir_mensaje(i,&mensaje_recibido))>0 ){
        	  while (estado  > 0) {
        		  // hasta q no termine el while no va tomar otro programa
        		  //analizar_msg_programa
            	switch (mensaje_recibido.codigo) {

                	case COMIENZO_CODIGO:
                		codigo_literal=string_new();
                		mensaje_respuesta.codigo = CODIGO_RECIBIDO;
                		mensaje_respuesta.cantidad=0;
                		mensaje_respuesta.extra=NULL;
                		mensaje_respuesta.parametros=NULL;
                		mensaje_respuesta.tam_extra=0;
                		enviar_mensaje(i,mensaje_respuesta);
                		recibir_mensaje(i,&mensaje_recibido);// ME AGREGABA BASURA AL MENSAJE_RECIBIDO.EXTRA
                		mensaje_recibido.extra = realloc(mensaje_recibido.extra, (mensaje_recibido.tam_extra)+1);
                		mensaje_recibido.extra[mensaje_recibido.tam_extra] = '\0'; // NO TOCAR IMPORTANTE
                		string_trim(&mensaje_recibido.extra);
                		log_info(log_kernel,"PLP: \tMensaje: %s","COMIENZO DE CODIGO");

                		break;

                	case CODIGO:
                		//offset+=mensaje_recibido.tam_extra;
                		string_trim(&mensaje_recibido.extra);
                		//codigo_literal=realloc(codigo_literal,offset);
                		string_append(&codigo_literal,mensaje_recibido.extra);
                		string_append(&codigo_literal,BAJADA);
                		 //memcpy(codigo_literal+codigo_recibidos*1024,mensaje_recibido.extra,mensaje_recibido.tam_extra);

                		 free(mensaje_recibido.extra);

                		 mensaje_respuesta.codigo = CODIGO_RECIBIDO;
                		 mensaje_respuesta.cantidad=0;
                		 mensaje_respuesta.tam_extra=0;
                		 mensaje_respuesta.parametros=NULL;
                		 mensaje_respuesta.extra=NULL;
                		 enviar_mensaje(i,mensaje_respuesta);

                		 log_info(log_kernel,"PLP: Mensaje: %s","LEYENDO CODIGO");
                		 //++codigo_recibidos;
                		 recibir_mensaje(i,&mensaje_recibido);
                		break;

                	case FIN_CODIGO: //
                		//codigo_recibidos=0;

                		//offset=0;
                		cargar_programa_en_mem(codigo_literal,i);
                		free(codigo_literal);
                		estado=0;
                		break;
        	  	}
        	  }
          }else {
    	  eliminar_programa_sistema(i);//usar socket
    	  log_debug(log_kernel, "PLP: Se desconecto Programa, se eliminan sus estructuras en el sistema.");
    	  //buscar pcpb por socket y elimarla y mostrar q programa se cayo
    	  close(i);
    	  FD_CLR(i, &setPLP);

      }//else q recibe de programa ya aceptado

     }//if isset
  } //for principal


}//while(1)
  return 0;
}//main

mensaje_t aviso(int codigo) {
  mensaje_t mensaje;
  mensaje.codigo = codigo;
  mensaje.cantidad = 0;
  mensaje.tam_extra = 0;
  mensaje.parametros = NULL;
  mensaje.extra = NULL;
  return mensaje;
}
// devuelve ESCRIBIR_SEGMENTO OK si se escribieron todos bien.
int escribir_segmentos(t_metadata_program*metadata,t_nodoPrograma*programa,char*codigo_literal){
	extern int socketUMV;
	extern int segmentosReservados;
	extern int grado_actual;
	int escrito = 0;


	//envio codigo
	escrito = enviar_codigo(codigo_literal,socketUMV,programa->PCB->segmento_codigo);
	if(escrito !=ESCRIBIR_SEGMENTO_OK){
		return escrito;
	}

	//envio etiquetas
	if (metadata->etiquetas_size==0){
		estado+=ESCRIBIR_SEGMENTO_OK;
	}else{
	escrito =enviar_Segmento(socketUMV,metadata->etiquetas,
			programa->PCB->indice_etiquetas,metadata->etiquetas_size );

	if (escrito != ESCRIBIR_SEGMENTO_OK){
		return escrito; //sale de la funcion ?
		}
	}
	//envio indice de codigo

	int tam_indice_codigo = sizeof(t_intructions)*metadata->instrucciones_size;
	char*indice=malloc(tam_indice_codigo);
	memcpy(indice,(char*)metadata->instrucciones_serializado,tam_indice_codigo);

	escrito = enviar_Segmento(socketUMV,indice,programa->PCB->indice_codigo,tam_indice_codigo);
	if (escrito != ESCRIBIR_SEGMENTO_OK){
		return escrito; }//sale de la funcion ?

	return escrito;
}


void pasar_a_ready(){
	extern int grado_multi;
	extern t_list*new;
	extern t_queue*ready;
	extern int pipepcp[2];
	extern t_list *listaProgramas;
	extern int grado_actual;

	if((grado_multi>grado_actual) && (list_size(new) > 0)){ 				/*Le agrego el && (list_size(new) > 0) porque si no entraba cuando la lista estaba vacia y hacia un remove y rompia*/
		t_nodoNew*nodo_new_aux=list_remove(new,0); //
		t_nodoReady *t_nodo_ready = malloc(sizeof(t_nodo_ready));
		t_nodo_ready->idPCB=nodo_new_aux->PCB;

		pthread_mutex_lock(&mutexReady);
		queue_push(ready,t_nodo_ready);
		pthread_mutex_unlock(&mutexReady);

		imprimirListas();

		int buscar_pcb_por_id(t_nodoPrograma*programa){
				if(nodo_new_aux->PCB->pid==programa->PCB->pid){
					return 1;
				}
				else{
					return 0;
				}
			}

		pthread_mutex_lock(&mutexListaProgramas);
		t_nodoPrograma* program=list_find(listaProgramas,(void*)buscar_pcb_por_id);
		program->estado=1;
		pthread_mutex_unlock(&mutexListaProgramas);

		/*int sizeN = queue_size(ready);
		log_info(log_kernel, "PLP: Tamanio de Lista de Ready: %d.",sizeN);*/

		//char *num_msg=string_itoa(hay_en_ready);
		t_mensaje_pipe *mensajePipe = malloc(sizeof(t_mensaje_pipe));
		mensajePipe->codigo = PROG_READY_PIPE;
		write(pipepcp[envio], mensajePipe, sizeof(mensajePipe));						/*Acordate de cambiarlo, de mandar un t_mensaje_pipe*/
		grado_actual++;
		free(nodo_new_aux); // toy lierando la pcb? la idea es q no. libero el nodo de la lista de new pero no la pcb
	}
}
//devueve 1 si la UMV  lo pudo escribir
int enviar_codigo(char*codigo,int unSocket,int base){

	mensaje_t mensaje_enviado,mensaje_recibido;

	int resto = strlen(codigo);
	int offset=0;
	int i = 0;
	int final;

	while(resto > 1024) {

		mensaje_enviado.codigo = ESCRIBIR_SEGMENTO;
		mensaje_enviado.cantidad = 2;
		mensaje_enviado.tam_extra = 1024;
		unsigned *p = malloc(2*sizeof(int));
		p[0] = base;
		p[1] = offset;
		mensaje_enviado.parametros =p;
		mensaje_enviado.extra = codigo + i * 1024;
		enviar_mensaje(unSocket,mensaje_enviado);
		free(mensaje_enviado.parametros);
		//recive msg de umv
		recibir_mensaje(unSocket, &mensaje_recibido);

		final =  rta_escritura_UMV( mensaje_recibido.codigo);
		resto -= 1024;
		offset+=1024;
		i++;
	}
	if(resto != 0){
		//Envio de lo que quedo del codigo
		mensaje_enviado.codigo = ESCRIBIR_SEGMENTO;
		mensaje_enviado.cantidad = 2;
		mensaje_enviado.tam_extra = resto;
		unsigned *p = malloc(2*sizeof(int));
		p[0] = base;
		p[1] = offset;
		mensaje_enviado.parametros =p;
		mensaje_enviado.extra = codigo+i*1024 ;
		log_info(log_kernel,"PLP: Texto a enviar a UMV es %s",mensaje_enviado.extra);
		enviar_mensaje(unSocket,mensaje_enviado);
		free(mensaje_enviado.parametros);
		recibir_mensaje(unSocket, &mensaje_recibido);
		final =  rta_escritura_UMV( mensaje_recibido.codigo);}
	return  final;
}
void imprimir_lista(t_list*lista){
	puts("---ELEMENTOS DE NEW---");
	//log_info(archivo, "NEW: %d (%d)");
	void imprimir(t_nodoNew*new){
		unsigned int pid = new->PCB->pid;
		printf("Programa(pid): %d con peso %d",pid,new->peso);
		puts("");
	}
	list_iterate(lista,(void*)imprimir);

	puts("");
	puts("----------------------");
}
void *liberar_nodoPrograma(t_nodoPrograma* data){
	free(data->PCB);
	free(data);
	return NULL;
}

void mandar_destruir_prog(int socket,int pid){
	mensaje_t mensaje;
	mensaje.codigo = DESTRUIR_SEGMENTOS;
	mensaje.cantidad=1;
	unsigned *p = malloc(sizeof(int)*mensaje.cantidad);
	p[0]=pid;
	mensaje.parametros=p;
	mensaje.extra=NULL;
	mensaje.tam_extra=0;
	enviar_mensaje(socket,mensaje);
	free(mensaje.parametros);
}


void enviar_tam_segmento(int unSocket,int tamano, int id){
	mensaje_t mensaje;
	mensaje.codigo = CREAR_SEGMENTO;
	mensaje.cantidad = 2;
	mensaje.tam_extra = 0;
	mensaje.extra = NULL;
	unsigned *p=malloc(sizeof(int)*mensaje.cantidad);
	p[0] = id;
	p[1] = tamano;
	mensaje.parametros = p;
	enviar_mensaje(unSocket,mensaje);
	free(mensaje.parametros);

}

//preguntar a fede si tengo q enviar parametros
//devuelve cod ESCRIBIR_SEGMENTO_OK, MEMORY OVERLOAD O SEGMENTATION FAULT
int enviar_Segmento(int socket,char *buffer, unsigned base, int tamano){
	int estado;
	mensaje_t mensaje,mensaje_recibido;
	mensaje.codigo = ESCRIBIR_SEGMENTO;
	mensaje.cantidad = 2;
	mensaje.tam_extra = tamano;
	unsigned *p = malloc(2*sizeof(int));
	p[0]=base;
	p[1]=0;  //offset
	mensaje.parametros=p;
	mensaje.extra=buffer;
	enviar_mensaje(socket,mensaje);
	destruir_mensaje(&mensaje);
	recibir_mensaje(socket,&mensaje_recibido);
	estado = rta_escritura_UMV(mensaje_recibido.codigo);
	return estado;
}
int rta_escritura_UMV(int codigo){
	int estado=0;
	switch(codigo){
		case SEGMENTATION_FAULT:
			log_info(log_kernel,"PLP: Mensaje :%s \n","UMV NO ESCRIBIO SEGMENTO POR SEGMENTATIO FAULT ");
			estado= SEGMENTATION_FAULT;
			break;
		case MEMORY_OVERLOAD:
			log_info(log_kernel,"PLP: Mensaje :%s \n","UMV NO ESCRIBIO SEGMENTO POR MEMORY OVERLOAD ");
			estado= MEMORY_OVERLOAD;
			break;
		default:
			log_info(log_kernel,"PLP: Mensaje :%s \n","UMV ESCRIBIO EL SEGMENTO ");
			estado= ESCRIBIR_SEGMENTO_OK;
			break;
	}
	return estado;
}


void destruir_Programa(t_PCB*pcb,int unSocket){
//lo busco por pid digo q lo mando a destruir a umv y libero el nodo
extern fd_set setPLP;
extern int socketUMV;
extern t_list* listaProgramas;
	bool encontrar_pcb(t_nodoPrograma* aux){
		if(aux->PCB->pid==pcb->pid){
			return 1;}
			else{ return 0;
		}
	}
	pthread_mutex_lock(&mutexListaProgramas);
	t_nodoPrograma*programa=list_remove_by_condition(listaProgramas, (void*) encontrar_pcb);
	pthread_mutex_unlock(&mutexListaProgramas);
	mandar_destruir_prog(socketUMV,pcb->pid);
	liberar_nodoPrograma(programa);
	close(unSocket);
	FD_CLR(unSocket,&setPLP);
}
int previo_menor(t_nodoNew* previo,t_nodoNew* posterior){

	if(previo->peso < posterior->peso){
	return 1;
	}else
	{
	return 0;
	}
}


int crear_socket_listen (int puerto) {
	int socketEscucha;
	int optval = 1;
	// Crear un socket:
	// AF_INET: Socket de internet IPv4
	// SOCK_STREAM: Orientado a la conexion, TCP
	// 0: Usar protocolo por defecto para AF_INET-SOCK_STREAM: Protocolo TCP/IPv4
	struct sockaddr_in socketInfo;
	if ((socketEscucha = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		return EXIT_FAILURE;
	}

	// Hacer que el SO libere el puerto inmediatamente luego de cerrar el socket.
	setsockopt(socketEscucha, SOL_SOCKET, SO_REUSEADDR, &optval,sizeof(optval));

	socketInfo.sin_family = AF_INET;
	socketInfo.sin_addr.s_addr = INADDR_ANY; //Notar que aca no se usa inet_addr()
	socketInfo.sin_port = htons(puerto);

// Vincular el socket con una direccion de red almacenada en 'socketInfo'.
	if (bind(socketEscucha, (struct sockaddr*) &socketInfo, sizeof(socketInfo))
			!= 0) {

		perror("Error al bindear socket escucha");
		return EXIT_FAILURE;
	}

// Escuchar nuevas conexiones entrantes.
	if (listen(socketEscucha, 10) != 0) {

		perror("Error al poner a escuchar socket");
		return EXIT_FAILURE;

	}
return socketEscucha;
}

int conectarse(const char * ip, int puerto) {
  struct sockaddr_in sa;
  sa.sin_family = AF_INET;
  sa.sin_port = htons(puerto);
  if (!inet_aton(ip, &sa.sin_addr)) return -1;
  int ns = socket(AF_INET, SOCK_STREAM, 0);
  if (socket < 0) return -1;
  if (connect(ns, (const void *) &sa, sizeof sa) < 0) {
    perror("connect");
    close(ns);
    return -1;
  }
  return ns;
}

int aceptar_conexion(int socket, t_list *lista, int idProg){
  int nuevoFichero;

  size_t tamanio;
  struct sockaddr_in extDirec;
  t_nodoPrograma *nodoPrograma=malloc(sizeof(t_nodoPrograma));
  t_PCB *PCB = malloc(sizeof(t_PCB));
  tamanio = sizeof(struct sockaddr_in);
  mensaje_t mensaje;

  if ((nuevoFichero = accept(socket, (struct sockaddr*)&extDirec, &tamanio)) == -1){
    perror("Error al aceptar conexion. \n");
    return 0;
  } else {
	recibir_mensaje(nuevoFichero,&mensaje);
	if(mensaje.codigo== HANDSHAKE_PROGRAMA){
		log_info(log_kernel,"PLP: Acepte un nuevo programa. Socket: %d, con Id: %d.\n", nuevoFichero, idProg);
		log_info(log_kernel,"%s", "HANDSHAKE");
		nodoPrograma->socket = nuevoFichero;
		nodoPrograma->idPrograma = idProg;
		PCB->pid = idProg;
		nodoPrograma->PCB = PCB;

		pthread_mutex_lock(&mutexListaProgramas);
		list_add(lista, nodoPrograma);
		pthread_mutex_unlock(&mutexListaProgramas);

		return nuevoFichero;

	}else{
		log_warning(log_kernel,"PLP: %s","Conexion invalida.");
		return 0;
	}

  }

}
void conectar_socket(int puerto, char* direccion,int socket){
	struct sockaddr_in socketInfo;
	socketInfo.sin_family = AF_INET;
	socketInfo.sin_addr.s_addr = inet_addr(direccion);
	socketInfo.sin_port = htons(puerto);
	// Conectar el socket con la direccion 'socketInfo'.
	if (connect(socket, (struct sockaddr*) &socketInfo, sizeof(socketInfo))!= 0) {
			perror("Error al conectar socket");
			log_error(log_kernel, "PLP: NO ESTA CONECTADA LA UMV");
			//se tendira q eliminar lsa estrucutasr del kernl
			exit(0);
	}else{
		log_debug(log_kernel, "PLP: Conectado a la UMV!");
	}
}

int obtener_socket(){
	int unSocket;
	// Crear un socket:
	// AF_INET: Socket de internet IPv4
	// SOCK_STREAM: Orientado a la conexion, TCP
	// 0: Usar protocolo por defecto para AF_INET-SOCK_STREAM: Protocolo TCP/IPv4
	if ((unSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			perror("Error al crear socket");
			return EXIT_FAILURE;
		}

		return unSocket;
}

void analizar_msg_pipe(void){
	extern int pipeplp[2];
	/*******Leia del pipepcp, por eso quedaba colgado iterando eternamente*********/
	t_mensaje_pipe * mensaje = malloc(sizeof(t_mensaje_pipe));
	read(pipeplp[recepcion],mensaje,sizeof(t_mensaje_pipe));
	log_debug(log_kernel,"PLP: %s","Me llego mensaje en el PIPE.");
	if (mensaje->codigo == COD_EXIT_PIPE){
		nuevo_exit(mensaje);					/*PARAMETRO AGREGADO (Ver abajo)*/
	} else if (mensaje->codigo == SYS_IMPR_PIPE){
		/* buscar en la lista de programas por mensaje.id*/
		imprimir_en_programa(mensaje->texto, mensaje->idPCB);
	}else if (mensaje->codigo == SYS_IMPR_TEXTO_PIPE){
		imprimir_en_programa(mensaje->texto, mensaje->idPCB);
	}
	log_info(log_kernel,"PLP: %s","Mensaje de PIPE tratado.");
	free(mensaje);
}

void imprimir_en_programa(char*texto,int id){
	extern t_list* listaProgramas;
		int buscar_pcb_por_id(t_nodoPrograma*programa){
			if(id==programa->PCB->pid){
				return 1;
			}
			else{
				return 0;
			}
		}
		t_nodoPrograma* program=list_find(listaProgramas,(void*)buscar_pcb_por_id);

		mensaje_t mensaje;
		mensaje.codigo=IMPRIMIR;
		mensaje.cantidad=0;
		mensaje.extra=texto;
		mensaje.tam_extra=strlen(texto);
		mensaje.parametros=NULL;
		enviar_mensaje(program->socket,mensaje);
		free(mensaje.extra);
		//tengo q saber el socket tambien
}

void eliminar_programa_sistema(int socket){

	t_nodoPrograma*nodo_programa;
	extern t_list*listaProgramas;
	int buscar_pcb_por_socket(t_nodoPrograma*programa){
			if(socket==programa->socket){
				return 1;
			}
			else{
				return 0;
			}
		}
	pthread_mutex_lock(&mutexListaProgramas);
	nodo_programa = list_remove_by_condition(listaProgramas,(void*)buscar_pcb_por_socket);
	pthread_mutex_unlock(&mutexListaProgramas);

	if  (nodo_programa->estado==0){
		eliminar_de_new(nodo_programa->PCB->pid);
	}else{
		mandar_eliminar_prog_pcp(nodo_programa->PCB->pid); //ELIMINAR_PROGRAMA
	}
	mandar_destruir_prog(socketUMV, nodo_programa->PCB->pid);
	liberar_nodoPrograma(nodo_programa);
	//arreglar con seba quien los aca de cada lista.

	// lo saco de la lista de programas, faltaria sacarlo de la lista en dnd este
}

void eliminar_de_new(unsigned pid){
	extern t_list* new;
	int buscar_pcb_por_id(t_nodoNew*programa){
		if(pid==programa->PCB->pid){
			return 1;
		}
		else{
			return 0;
		}
	}
	t_nodoNew* NEW=list_find(new,(void*)buscar_pcb_por_id);
	free(NEW);

}

void mandar_eliminar_prog_pcp(unsigned pid){

	extern int pipepcp[2];
	t_mensaje_pipe *mensaje=malloc(sizeof(t_mensaje_pipe));
	mensaje->codigo = ELIMINAR_PROGRAMA;
	mensaje->idPCB=pid;
	write(pipepcp[envio],mensaje,sizeof(t_mensaje_pipe));


}


void nuevo_exit(t_mensaje_pipe *mensaje){			/*Le agregue un parametro para usar el texto (en realidad el parametro puede ser solo el texto) -> porque dejamos de usar el nodoExit->texto*/
		extern t_list *exitList;
		extern t_list*listaProgramas;
		extern int grado_actual;
		//mensaje_t mensaje_respuesta;
		extern fd_set setPLP;  		/*CAMBIO ACA, no decia externo (vos me hiciste cambiarlo)*/


		//lo saco de lista exit

		pthread_mutex_lock(&mutexExit);
		t_nodoExit *nodoExit = list_remove(exitList,0);// 2do parametro cero xq es el 1ro de la lista
		pthread_mutex_unlock(&mutexExit);

		imprimirListas();
		//envio al programa q imprima ***************ACA podes llamar a la funcion de imprimir_en_programa (que no esta hecha), total es lo mismo

/*
		mensaje_respuesta.codigo = IMPRIMIR;
		mensaje_respuesta.extra = strdup(mensaje->texto);				Creo que no tenia el strdup (era otra funcion que me rompia)
		mensaje_respuesta.tam_extra = strlen(mensaje->texto);
		mensaje_respuesta.cantidad = 0;
		mensaje_respuesta.parametros=NULL;
*/
		int pid = nodoExit->pcb->pid;
		bool encontrar_pcb(t_nodoPrograma *aux){
			if((aux->idPrograma) == pid){
				return 1;
			}else{
				return 0;
			}
		}

		log_info(log_kernel,"PLP: Se elimino de la Lista de Programas el Programa con PID: %d",pid);
		imprimir_en_programa(mensaje->texto,pid); //mensaje texto tendria q estar alocado

		pthread_mutex_lock(&mutexListaProgramas);
		t_nodoPrograma *nodoPrograma_aux =list_remove_by_condition(listaProgramas, (void*) encontrar_pcb);
		pthread_mutex_unlock(&mutexListaProgramas);

		mandar_destruir_prog(socketUMV,nodoExit->pcb->pid);

		FD_CLR(nodoPrograma_aux->socket,&setPLP);
		close(nodoPrograma_aux->socket);
		liberar_nodoPrograma(nodoPrograma_aux);
		free(nodoExit);

		grado_actual--;
		pasar_a_ready(grado_actual);

	}
//podria devolver 1 si lo hizo bien
void cargar_programa_en_mem(char*codigo_literal,int socket_programa){
	t_metadata_program *metadata;
	mensaje_t mensaje_respuesta;
	int escritos;
	extern t_list *new;
	extern int estado;
	extern int segmentosReservados;
	extern t_list* listaProgramas;
	t_nodoPrograma *nodo_programa;
	t_PCB*pcb;

	log_info(log_kernel,"PLP: Recibido FIN DE CODIGO");
	log_info(log_kernel,"PLP: Mensaje: \n %s",codigo_literal);

	log_info(log_kernel, "PLP: Tamanio de codigo recibido:%d.", strlen(codigo_literal));
	metadata=metadata_desde_literal(codigo_literal);
	int buscar_pcb_por_socket(t_nodoPrograma*programa){
		if(socket_programa==programa->socket){
			return 1;
		}
		else{
			return 0;
		}
	}

	pthread_mutex_lock(&mutexListaProgramas);
	nodo_programa = list_remove_by_condition(listaProgramas,(void*)buscar_pcb_por_socket);
	pthread_mutex_unlock(&mutexListaProgramas);

	pcb=nodo_programa->PCB;
	pcb->pc=metadata->instruccion_inicio;
	pcb->tam_indice_etiquetas=metadata->etiquetas_size;
	pcb->tam_contexto_actual=0;

	pthread_mutex_lock(&mutexListaProgramas);
	list_add(listaProgramas,nodo_programa);
	pthread_mutex_unlock(&mutexListaProgramas);

	reservar_segmentos(pcb,metadata,socket_programa,codigo_literal);

	//si se pudo reservar, envia los datos de los segmentos
	if(segmentosReservados == 1){

			escritos = escribir_segmentos(metadata,nodo_programa,codigo_literal);
			if (escritos==ESCRIBIR_SEGMENTO_OK){

				pasar_a_NEW(metadata,nodo_programa);
				segmentosReservados=0;  // lo pone en cero para el prox programa
			}else{ //poer razon ada por escritos
				log_warning(log_kernel, "PLP: No se pudieron escribir los segmentos.");
				mensaje_respuesta.codigo=IMPRIMIR;
				mensaje_respuesta.cantidad=0;
				char *texto =strdup("No se pudieron escribir los segmentos");
				mensaje_respuesta.extra=texto;
				mensaje_respuesta.parametros=NULL;
				mensaje_respuesta.tam_extra=strlen(texto);
				enviar_mensaje(socket_programa,mensaje_respuesta);
				free(mensaje_respuesta.extra);
				destruir_Programa(pcb,socket_programa);

			}
				}
}
int verificar_tam_etiquetas(int size,int num_ciclo,int num_array){
	int estado;
	if(num_ciclo==num_array){
		if (size==0){
			estado=1;
			}else
			estado=0;
	}else {
	estado=0;};

return estado;
}

void reservar_segmentos(t_PCB *pcb,t_metadata_program*metadata,int socket_program,char*codigo_literal){
	extern int estado;
	extern int segmentosReservados;
	extern t_list*listaProgramas;
	extern int tam_stack;
	mensaje_t mensaje_recibido;
	extern int socketUMV;
	int num_seg;
	unsigned array_tamanos[4];

	array_tamanos[0] = strlen(codigo_literal);
	array_tamanos[1] = metadata->etiquetas_size;
	array_tamanos[2] = tam_stack;
	array_tamanos[3] = sizeof(t_intructions)*metadata->instrucciones_size; // obtener tamaño del indice de codigo;
	for(num_seg=0;num_seg<=3;num_seg++){ //pregunta si puede reservar segmentos

		if(verificar_tam_etiquetas(array_tamanos[num_seg],num_seg,1)== 1){
			pcb->indice_etiquetas = 0;
			continue;
		}
		enviar_tam_segmento(socketUMV, array_tamanos[num_seg], pcb->pid);
		recibir_mensaje(socketUMV,&mensaje_recibido);

		if (mensaje_recibido.codigo == CREAR_SEGMENTO_OK){
			switch(num_seg){
			case 0:
				pcb->segmento_codigo = mensaje_recibido.parametros[0];
				log_info(log_kernel,"PLP: Mensaje \n Base: %d,",pcb->segmento_codigo);
				break;
			case 3:
				pcb->indice_codigo = mensaje_recibido.parametros[0];
				log_info(log_kernel,"PLP: Mensaje \n Base: %d,",pcb->indice_codigo);
				segmentosReservados =1;
				break;
			case 2:
				pcb->segmento_stack = mensaje_recibido.parametros[0];
				pcb->pointer_stack = mensaje_recibido.parametros[0];
				log_info(log_kernel,"PLP: Mensaje \n Base: %d,",pcb->segmento_stack);
				;
				break;
			case 1:
				pcb->indice_etiquetas=mensaje_recibido.parametros[0];
				log_info(log_kernel,"PLP: Mensaje \n Base: %d,",pcb->indice_etiquetas);
				break;
			default:
				break;

			free(mensaje_recibido.parametros);
				}
		}
		else{
			//msg a programa
			char *texto=strdup("MEMORY OVERLOAD");
			imprimir_en_programa(texto,pcb->pid);
			destruir_Programa(pcb,socket_program);
			estado = 0;

			break;
				}

	}
}
void pasar_a_NEW(t_metadata_program* metadata,t_nodoPrograma*nodo_programa){
	extern t_list*new;
	extern t_list*listaProgramas;
	extern int grado_actual;

	int peso = (5 * metadata->cantidad_de_etiquetas) + (3 * metadata->cantidad_de_funciones) + (metadata->instrucciones_size);
	t_nodoNew *nodoNew=malloc(sizeof(t_nodoNew));
	nodoNew->PCB = nodo_programa->PCB;
	nodoNew->peso = peso;
	nodo_programa->estado=0;
	list_add(new,nodoNew);

	list_sort(new,(void*)previo_menor);
	//imprimir_lista(new);
	imprimirListas();
	pasar_a_ready(grado_actual);
}
