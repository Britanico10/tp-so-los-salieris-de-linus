#ifndef PROGRAMA_H_
#define PROGRAMA_H_
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "commons.h"
#include <commons/log.h>
#include "mensajes.h"
#include "codigo_operacion.h"

#define fconfig "configuracion.txt"

void cargarConfiguracion(void);
int leer_archivo(const char *, char **);
void enviar_codigo(char *, int, int);

extern char *IP;
extern int PUERTO;

#endif /* PROGRAMA_H_ */