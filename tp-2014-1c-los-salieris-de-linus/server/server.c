/*
 * server.c
 *
 *  Created on: 21/04/2014
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
#include <sys/time.h>

#define DIRECCION INADDR_ANY   //INADDR_ANY representa la direccion de cualquier
							   //interfaz conectada con la computadora
#define PUERTO 5000
#define BUFF_SIZE 1024


int main() {

	fd_set read_fs; // descriptores q estan lisots para leer
	fd_set master; //descriptores q q estan actualemnte conectados
	size_t tamanio; // hace positivo a la variable
	int socketEscucha, socketNuevaConexion;
	int nbytesRecibidos;

	int max;


	struct sockaddr_in socketInfo;
	struct sockaddr_in cliente;

	FD_ZERO(&master);
	FD_ZERO(&read_fs);

	char *buffer=malloc(100*sizeof(char));
	int optval = 1;

	// Crear un socket:
	// AF_INET: Socket de internet IPv4
	// SOCK_STREAM: Orientado a la conexion, TCP
	// 0: Usar protocolo por defecto para AF_INET-SOCK_STREAM: Protocolo TCP/IPv4
	if ((socketEscucha = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		return EXIT_FAILURE;
	}

	// Hacer que el SO libere el puerto inmediatamente luego de cerrar el socket.
	setsockopt(socketEscucha, SOL_SOCKET, SO_REUSEADDR, &optval,
			sizeof(optval));

	socketInfo.sin_family = AF_INET;
	socketInfo.sin_addr.s_addr = DIRECCION; //Notar que aca no se usa inet_addr()
	socketInfo.sin_port = htons(PUERTO);

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

	printf("Escuchando conexiones entrantes.\n");

	//arranca select

	FD_SET(socketEscucha,&master);

	max = socketEscucha;

	printf("%d \n",socketEscucha);

	while(1){

		memcpy(&read_fs, &master, sizeof(master));
		int dev_select;
		if( (dev_select = select(max +1, &read_fs,NULL,NULL,NULL))==-1){
			perror("select");

		}
		printf("select = %d \n",dev_select);
		int i;
		for(i=0; i<= max ;i++) //max : cantidad max de sockets
				{
			if(FD_ISSET(i,&read_fs)){
				printf("i = %d \n max = %d \n",i,max);

				if(i==socketEscucha){
								// pasar a una funcion generica aceptar();
								tamanio = sizeof(struct sockaddr_in);

								if ((socketNuevaConexion = accept(socketEscucha, (struct sockaddr*)&cliente,&tamanio)) < 0) {

							 		perror("Error al aceptar conexion entrante");

							 		return EXIT_FAILURE;

							 	}else{

							 		if(socketNuevaConexion >max){

							 			max =socketNuevaConexion;

							 		}
							 		FD_SET(socketNuevaConexion,&master);
							 		printf("nueva conexion de %s desde socket %d \n",inet_ntoa(cliente.sin_addr), socketNuevaConexion);



							 	}//if del accept


							// Recibir hasta BUFF_SIZE datos y almacenarlos en 'buffer'.

					}else
					{

						//verifica si esta en el cojunto de listos para leer
						//pasarlo a una funcion generica

						if ((nbytesRecibidos = recv(i, buffer, BUFF_SIZE,0)) > 0) {

							printf("Mensaje recibido de socket %d: ",i);
							fwrite(buffer, 1, nbytesRecibidos, stdout);
							printf("\n");
							printf("Tamanio del buffer %d bytes!\n", nbytesRecibidos);
							fflush(stdout);


						}else if(nbytesRecibidos == 0) {
							printf("se desconecto el socket %d \n",i);
							FD_CLR(i,&master);
							// aca se tendria q actualizar los maximos.

						}else
						{
							printf("Error al recibir datos\n i= %d\n",i);
							break;
										 		 }

						// con el scoket se podria responder al momento

					}
				}//1er if

				}// for


	}




	close(socketEscucha);
	return EXIT_SUCCESS;

}

/* notas:
 * el socket inicial empieza en 3
 *
 *  */

