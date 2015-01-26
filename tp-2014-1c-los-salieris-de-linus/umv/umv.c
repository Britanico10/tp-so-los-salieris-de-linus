#include "umv.h"

extern nodo_segmento *lsu;
extern int (* obtener_bloque) (nodo_segmento *);
extern int TOPE;
extern int RETARDO;
extern int PUERTO;
extern char *umv_dmr;
extern t_log *archivo_log;

void cargarConfiguracion(void) {
	t_config *conf = config_create(fconfig);
	if (!config_has_property(conf, "MEMORIA"))
		exit(0);
	TOPE = config_get_int_value(conf, "MEMORIA");
	if (!config_has_property(conf, "RETARDO"))
		exit(0);
	RETARDO = config_get_int_value(conf, "RETARDO");
	if (!config_has_property(conf, "PUERTO"))
		exit(0);
	PUERTO = config_get_int_value(conf, "PUERTO");
	config_destroy(conf);
}

int main(int argc, char ** argv) {
	int socketListen;
	archivo_log=log_create("log.txt","umv",0,LOG_LEVEL_INFO);
	cargarConfiguracion();
	log_info(archivo_log,"Configuracion leida y cargada");
	if (reservarMallocInicial() == -1) {
		puts("Fallo reserva del malloc inicial");
		log_info(archivo_log,"Fallo carga inicial del malloc");
		exit(EXIT_FAILURE);
	}
	log_info(archivo_log,"Se reservo el espacio inicial de %d bytes",TOPE);
	pthread_rwlock_init(&semaforo_lsu,NULL);
	log_info(archivo_log,"Se inicializo los semaforos");
	pthread_t thread_consola;
	pthread_create(&thread_consola, NULL, consola, NULL);
	log_info(archivo_log,"Inicio la ejecucion de la consola");
	socketListen = crear_socket_listen();
	if (socketListen < 0) {
		perror("Fallo creacion de socket en escucha");
		return 1;
	}
	log_info(archivo_log,"Socket en escucha creado en el puerto %d", PUERTO);
	while (1) aceptar_conexion(socketListen);
	/* todo esto debe pasar al cerrarse la UMV, por ahora no llega hasta aca!*/
	pthread_join(thread_consola,0);
	free(umv_dmr);
	pthread_rwlock_destroy(&semaforo_lsu);
	exit(EXIT_SUCCESS);
}
