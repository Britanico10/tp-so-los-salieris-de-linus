#include "kernel.h"
#include <unistd.h>


/*void bloquearPCB(int idPCB, int retardo){
	t_nodoBloqIO *nodoBloq = malloc(sizeof(t_nodoBloqIO));
}*/

extern int pipepcp[2];
t_log *log_io;

void sleepmili(int milisegundos){
	usleep(milisegundos*1000);
}

void bloquearPCB(int idPCB, int tiempo, char *id_IO, int retardoInicial){
	t_mensaje_pipe *mensaje = malloc(sizeof(t_mensaje_pipe));
	log_info(log_io, "Empieza bloqueo\n");
	sleepmili(tiempo * (retardoInicial));
	//usleep((tiempo * 1000000)* (retardoInicial * 1000));
	mensaje->codigo = IO_TERMINA;
	mensaje->idPCB = idPCB;
	mensaje->texto = strdup(id_IO);
	mensaje->numero = strlen(mensaje->texto);
	write(pipepcp[envio], mensaje, sizeof(t_mensaje_pipe));
	log_info(log_io, "Termina bloqueo, mando mensaje: %d, %d, %s.\n", mensaje->codigo, mensaje->idPCB, mensaje->texto);
	/*free(mensaje->texto);
	free(mensaje);*/
}



void *io(t_dataDispositivos *dataIO) {
	int pipeIO[2];
	char *id = strdup(dataIO->id_IO);
	int retardoInicial = dataIO->retardoInicial;
	log_io = log_create("IO_log.log", "IO_log.log", 0, LOG_LEVEL_INFO);
	log_info(log_io, "Empieza IO: %s.\n", id);
	pipe(pipeIO);
	//t_queue *colaBloq;

	sleep(1);
	t_mensaje_pipe *handshake = malloc(sizeof(t_mensaje_pipe));
	handshake->codigo = HANDSHAKE_IO;
	handshake->texto = strdup(dataIO->id_IO);
	handshake->numero = pipeIO[envio];


	write(pipepcp[envio], handshake, sizeof(t_mensaje_pipe));


	while(1){
		t_mensaje_pipe *mensaje_recibido = malloc(sizeof(t_mensaje_pipe));
		if (read(pipeIO[recepcion], mensaje_recibido, sizeof(t_mensaje_pipe)) == -1){
			perror("-1");
			return 0;
		}
		if (mensaje_recibido->codigo == SYS_BLOQ_IO){
			log_info(log_io, "\t\t%s: bloqueo programa: %d por %d milisegundos.\n", dataIO->id_IO, mensaje_recibido->idPCB, (mensaje_recibido->numero) * (retardoInicial));
			bloquearPCB(mensaje_recibido->idPCB, mensaje_recibido->numero, id, retardoInicial);
		}
		free(mensaje_recibido->texto);
		free(mensaje_recibido);
	}

	return NULL;
}
