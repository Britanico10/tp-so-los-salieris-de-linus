#include "programa.h"

t_log *log_programa;

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
}

void handshake (int socket, unsigned codigo) {
	mensaje_t mensaje;
	mensaje.codigo = codigo;
	mensaje.cantidad = 0;
	mensaje.tam_extra = 0;
	mensaje.extra = NULL;
	mensaje.parametros = NULL;
	enviar_mensaje(socket, mensaje);
}

void enviar_codigo(char *codigo,int unSocket, int tamanio){
	mensaje_t mensaje_enviado,mensaje_recibido;
	int resto = tamanio;
	int i = 0, estado;
	mensaje_enviado.codigo = COMIENZO_CODIGO;
	mensaje_enviado.cantidad = 0;
	mensaje_enviado.tam_extra = 0;
	mensaje_enviado.parametros = NULL;
	mensaje_enviado.extra = NULL;
	enviar_mensaje(unSocket,mensaje_enviado);

	if((estado = recibir_mensaje(unSocket, &mensaje_recibido)) < 0) {
		log_error(log_programa,"No se puede cargar el programa en este momento");
		return;
	}

	while(resto > 1024) {
		mensaje_enviado.codigo = CODIGO;
		mensaje_enviado.cantidad = 0;
		mensaje_enviado.tam_extra = 1024;
		mensaje_enviado.parametros = NULL;
		mensaje_enviado.extra = codigo + i * 1024;
		enviar_mensaje(unSocket,mensaje_enviado);
		if ((estado = recibir_mensaje(unSocket, &mensaje_recibido)) > 0) {
			if (mensaje_recibido.codigo != CODIGO_RECIBIDO)	{
				log_error(log_programa,"Recepcion de codigo desconocido durante la transferencia");
				return;
			}
		}
		else { 
			log_error(log_programa,"No se pudo cargar el codigo del programa"); 
			continue;
		}
		i++;
		resto -= 1024;
	}
	if(resto != 0){
		//Envio de lo que quedo del codigo
		mensaje_enviado.codigo = CODIGO;
		mensaje_enviado.cantidad = 0;
		mensaje_enviado.tam_extra = resto;
		mensaje_enviado.parametros = NULL;
		mensaje_enviado.extra = codigo + i * 1024;
		enviar_mensaje(unSocket,mensaje_enviado);
		resto = 0;
		if ((estado = recibir_mensaje(unSocket, &mensaje_recibido)) > 0) {
			//Aviso de finalizacion de envio del codigo del programa
			mensaje_enviado.codigo = FIN_CODIGO;
			mensaje_enviado.cantidad = 0;
			mensaje_enviado.tam_extra = 0;
			mensaje_enviado.parametros = NULL;
			mensaje_enviado.extra = NULL;
			enviar_mensaje(unSocket,mensaje_enviado);
		} else log_error(log_programa,"Fallo la carga final del programa");
	}
}

int main(int argc, char**argv){
	log_programa = log_create("log_prog.log", "log_prog.log", 0, LOG_LEVEL_INFO);
	char *IP = NULL;
	int PUERTO;
	int unSocket;
	char *codigo;
	log_info(log_programa,"Entro al programa");
	int tamano_codigo = leer_archivo(argv[1], &codigo);
	log_info(log_programa,"Tamanio de codigo: %d\n", tamano_codigo);
	if (tamano_codigo < 0) {
		perror("Codigo invalido.");
		exit(0);
	}
	if ((unSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Error al crear socket");
		exit(0);
	}

	t_config *conf = config_create(fconfig);
	if (!config_has_property(conf, "IP_KERNEL")) exit(0);
	IP = realloc(IP, strlen(config_get_string_value(conf, "IP_KERNEL"))+1);
	IP = config_get_string_value(conf, "IP_KERNEL");
	if (!config_has_property(conf, "PUERTO_KERNEL")) exit(0);
	PUERTO = config_get_int_value(conf, "PUERTO_KERNEL");


	struct sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(PUERTO);
	if (!inet_aton(IP, &sa.sin_addr)) return -1;
		unSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (unSocket < 0) return -1;
	if (connect(unSocket, (const void *) &sa, sizeof sa )< 0) {
	 perror("connect");
	 close(unSocket);
	 return -1;
	}

	config_destroy(conf);

	handshake(unSocket, HANDSHAKE_PROGRAMA);
	log_info(log_programa,"Conectado al Kernel");
	enviar_codigo(codigo, unSocket, tamano_codigo);
	mensaje_t mensaje_recibido;
	int estado;
	while ((estado = recibir_mensaje(unSocket, &mensaje_recibido)) > 0) {
		switch (mensaje_recibido.codigo){
			case IMPRIMIR: 
				mensaje_recibido.extra = realloc(mensaje_recibido.extra, (mensaje_recibido.tam_extra)+1);
				mensaje_recibido.extra[mensaje_recibido.tam_extra] = '\0';
				printf("%s\n",mensaje_recibido.extra);
				free(mensaje_recibido.extra);
				//destruir_mensaje(&mensaje_recibido);
			break;
			case FIN_PROGRAMA:
				log_info(log_programa,"Finalizando..");
				free(codigo); free(IP);
				return 0;
			break;
			default: 
				log_error(log_programa,"El codigo recibido por parte del kernel no es correcto");
			break;
		}
		//free(mensaje_recibido.extra);
	}
	return 0;
}
