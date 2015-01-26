
#include "mensajes_codigo_ansisop.h"


char *recibir_todo(int socket, mensaje_t unMensaje){
	int resto = 0;
	int cantBytesAux = 0;
	char *bufferExtra = malloc(512);
	char*codigo=malloc(unMensaje.parametros[0]+1); //agregado

	if (strlen(unMensaje.extra) < unMensaje.parametros[0]){			//Llego menos lo que deberia llegar
		printf("%d longitdud total\n ",unMensaje.parametros[0]);//agregado

		resto = (unMensaje.parametros[0]) - (strlen(unMensaje.extra));

		printf("%des el resto  \n",resto);//agregado
		memcpy(codigo,unMensaje.extra,strlen(unMensaje.extra)); //agregado

		while(resto > 0) {
			if ((cantBytesAux = recv(socket, bufferExtra, 512, 0)) <= 0){
				perror("Error en el recibimiento de codigo");
			} else {

				printf("%d   buffers EXRAAAA\n",strlen(bufferExtra));

				if(resto<BUFF_LIMITE){
				memcpy(codigo+strlen(codigo),bufferExtra,resto);
				}else{
				memcpy(codigo+strlen(codigo),bufferExtra,512);
				}
				resto = (unMensaje.parametros[0]) - (strlen(codigo));
			}

		}
	}else{
		memcpy(codigo,unMensaje.extra,strlen(unMensaje.extra)+1);
	}

	printf("Mensaje recibido en PLP de socket: %d\n",socket);
	//fwrite(unMensaje.extra, 1, strlen(unMensaje.extra)+1, stdout);
	printf("%s",codigo);
	printf("Tamanio del mensaje: %d bytes!\n", strlen(codigo));
	//fflush(stdout);
	free(bufferExtra);

	return codigo;

}


int mandar_codigo(mensaje_t mensaje,char *codigo,int unsocket){

	char *cod_aux=malloc(strlen(codigo));
	printf("%d longitud del codigo \n",strlen(codigo));

	mensaje.codigo = CODIGO_LITERAL;
	 mensaje.cantidad = 1;
	 mensaje.parametros=malloc(sizeof(unsigned int)*mensaje.cantidad);

	 if (strlen(codigo)<=BUFF_LIMITE){
		 memcpy(mensaje.extra,codigo,strlen(codigo)+1);
	 }else{
		 memcpy(mensaje.extra,codigo,BUFF_LIMITE);
	 }
	 mensaje.tam_extra=strlen(mensaje.extra);
	// int a=strlen(mensaje.extra);
	printf("%s",mensaje.extra);
	 mensaje.parametros[0]=strlen(codigo)+1;
	 enviar_mensaje(unsocket,mensaje);

	 int tamano_cod_restante  = strlen(codigo)-strlen(mensaje.extra);
	 int bytes_mandados = 0;
	 int offset = BUFF_LIMITE;
	 int cw=0;

	 while(tamano_cod_restante > 0){
		 if(tamano_cod_restante<=BUFF_LIMITE){
		 memcpy(cod_aux,codigo + offset, tamano_cod_restante);
		 puts("sdsdsdsd");
		 }
		 else
		 {
		memcpy(cod_aux,codigo + offset, BUFF_LIMITE);
		 }

		 bytes_mandados = send(unsocket,cod_aux,strlen(cod_aux),0);
		 printf("%d bytes madados \n",bytes_mandados);
		 if(bytes_mandados == -1) {
			 perror("imposible enviar");
			 break;
		 }
		tamano_cod_restante = tamano_cod_restante- bytes_mandados;
		offset=offset + BUFF_LIMITE;
		cw++;
	 }

	 free(cod_aux);
	 //printf("%d bytes q tendira q haber enviado\n",mensaje.parametros[0]); //prueba
	 free(mensaje.parametros);


	// printf("%d bytes mandados \n",(bytes_mandados+a));//prueba

	 return 0;
}

