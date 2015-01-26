/*
 * mensajes_codigo_ansisop.h
 *
 *  Created on: 22/05/2014
 *      Author: utnso
 */

#ifndef MENSAJES_CODIGO_ANSISOP_H_
#define MENSAJES_CODIGO_ANSISOP_H_

#define BUFF_LIMITE 512
#define CODIGO_LITERAL 112
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include "mensajes.h"

char *recibir_todo(int socket, mensaje_t unMensaje);
int mandar_codigo(mensaje_t mensaje,char *codigo,int unsocket);


#endif /* MENSAJES_CODIGO_ANSISOP_H_ */
