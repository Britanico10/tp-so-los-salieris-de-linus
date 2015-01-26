/*
 * mock_plp.c
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
#define PATH "/home/utnso/Escritorio/productor.ansisop"
#include <parser/metadata_program.h>
#include "mensajes.h"
#include <commons/string.h>

#define DIRECCION INADDR_ANY   //INADDR_ANY representa la direccion de cualquier
							   //interfaz conectada con la computadora
#define PUERTO 5000
#define BUFF_SIZE 1024
int leer_archivo (const char * ruta, char ** resultado);

int main() {

	int socketEscucha, socketNuevaConexion;

	struct sockaddr_in socketInfo;

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

// Aceptar una nueva conexion entrante. Se genera un nuevo socket con la nueva conexion.
	if ((socketNuevaConexion = accept(socketEscucha, NULL, 0)) < 0) {

		perror("Error al aceptar conexion entrante");
		return EXIT_FAILURE;

	}
	char*codigo_literal=malloc(1024);
	int tamano_codigo = leer_archivo(PATH, &codigo_literal);
	printf("%d \n",tamano_codigo);
	t_metadata_program *metadata=metadata_desde_literal(codigo_literal);

	int tam_indice_codigo = sizeof(t_intructions)*metadata->instrucciones_size;
	char*indice=malloc(tam_indice_codigo);

	memcpy(indice,(char*)metadata->instrucciones_serializado,tam_indice_codigo);


	printf ("%d",metadata->instrucciones_serializado[2].offset);
	mensaje_t mensaje;
	mensaje.codigo =123;
	mensaje.cantidad = 2;
	mensaje.tam_extra = tam_indice_codigo;
	unsigned *p = malloc(2*sizeof(int));
	p[0]=123;
	p[1]=0;  //offset
	mensaje.parametros=p;
	mensaje.extra=indice;
	enviar_mensaje(socketNuevaConexion,mensaje);
	destruir_mensaje(&mensaje);



	close(socketEscucha);
	close(socketNuevaConexion);

	return EXIT_SUCCESS;
}

int leer_archivo (const char * ruta, char ** resultado) {
	if (!ruta) return -1;
	FILE * archivo = fopen(ruta, "rb");
	if (!archivo) return -1;
	char lectura[1024];
	char * buffer = NULL;
	int cantidad_total = 0;
	int leido;
	while (1) {
		leido = fread(lectura, 1, 1024, archivo);
		if (leido) {
			buffer = realloc(buffer, cantidad_total + leido);
			memcpy(buffer + cantidad_total, lectura, leido);
			cantidad_total += leido;
		} else {
			leido = ferror(archivo);
			fclose(archivo);
			if (leido) {
				free(buffer);
				return -1;
			}
			*resultado = buffer;
			return cantidad_total;
		}
	}
	return cantidad_total;
}
