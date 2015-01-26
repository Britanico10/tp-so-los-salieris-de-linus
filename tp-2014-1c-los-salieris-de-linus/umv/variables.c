#include "umv.h"

pthread_rwlock_t semaforo_lsu;
nodo_segmento *lsu = NULL;
int (* obtener_bloque) (nodo_segmento *) = obtener_bloque_ff;
int TOPE;
int RETARDO;
int PUERTO;
char *umv_dmr;
t_log *archivo_log;