/*
 * mock_cpu.c
 *
 *  Created on: 16/06/2014
 *      Author: utnso
 */


#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "mensajes.h"
#include <parser/metadata_program.h>

#define DIRECCION "127.0.0.1"
#define PUERTO 5000
#define BUFF_SIZE 1024

int main() {

	int unSocket;

	struct sockaddr_in socketInfo;


	printf("Conectando...\n");

// Crear un socket:
// AF_INET: Socket de internet IPv4
// SOCK_STREAM: Orientado a la conexion, TCP
// 0: Usar protocolo por defecto para AF_INET-SOCK_STREAM: Protocolo TCP/IPv4
	if ((unSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Error al crear socket");
		return EXIT_FAILURE;
	}

	socketInfo.sin_family = AF_INET;
	socketInfo.sin_addr.s_addr = inet_addr(DIRECCION);
	socketInfo.sin_port = htons(PUERTO);

// Conectar el socket con la direccion 'socketInfo'.
	if (connect(unSocket, (struct sockaddr*) &socketInfo, sizeof(socketInfo))
			!= 0) {
		perror("Error al conectar socket");
		return EXIT_FAILURE;
	}

	printf("Conectado!\n");

	mensaje_t mensaje;



	recibir_mensaje(unSocket,&mensaje);
	//t_intructions* indice = (t_intructions*) mensaje.extra;
	t_intructions *indice = malloc(8);
	char* indicec=malloc(8);
	memcpy(indicec,mensaje.extra+8,8);
	indice = (t_intructions*)indicec;
	printf(" offset %d start %d \n",indice->offset,indice->start);




	close(unSocket);

	return EXIT_SUCCESS;

}
