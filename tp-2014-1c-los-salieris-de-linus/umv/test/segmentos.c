#include <stdio.h>
#include <libio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include "commons.h"

#define fconfig "configuracion.txt"

typedef struct nodo_usado_struct {
	int pid;
	int base;
	int tamanio;
	int dmr;
	struct nodo_usado_struct * siguiente;
} nodo_segmento;

char *umv_dmr;

//int main(int, char **);
int reservarMallocInicial(void);
int escribirSegmento(int, int, int, char *);
int reservarSegmento(int, int);
int obtener_bloque_ff(nodo_segmento *);
int obtener_bloque_wf(nodo_segmento *);
void compactar(void);
nodo_segmento *buscar_nodo(int, int);

void cargarConfiguracion(void);

nodo_segmento *lsu = NULL;
int (* obtener_bloque) (nodo_segmento *) = obtener_bloque_ff;
int TOPE;
int RETARDO;
int PUERTO;

void cargarConfiguracion(void) {
	t_config *conf = config_create(fconfig);
	if (!config_has_property(conf, "MEMORIA"))
		//no esta definido el parametro memoria!
	TOPE = config_get_int_value(conf, "MEMORIA");
	if (!config_has_property(conf, "RETARDO"))
		//no esta definido el parametro retardo!
	RETARDO = config_get_int_value(conf, "RETARDO");
	if (!config_has_property(conf, "PUERTO"))
		//no esta definido el parametro puerto!
	PUERTO = config_get_int_value(conf, "PUERTO");
	config_destroy(conf);
}

void compactar (void) { //faltan semaforos
	nodo_segmento *nodo_aux;
	if (lsu){ //Si lsu no es null
		int posicion_actual = lsu->tamanio;
		nodo_aux = lsu->siguiente;
		if (lsu->dmr != 0) {  //Si el primer segmento no empieza en la 1er posicion
			memcpy(umv_dmr,(lsu->dmr)+umv_dmr,lsu->tamanio);
			lsu->dmr = 0; //Como se lo mueve al principio, su base virtual es cero
			posicion_actual = lsu->tamanio;  //La prox posicion disponible es su tamanio
		}
		while(nodo_aux) { //Nos adentramos en la lista lsu
			if (posicion_actual != nodo_aux->dmr){ //si el segmento no esta en donde deberia estar (En la 1er pos libre)
				memcpy(posicion_actual+umv_dmr,(nodo_aux->dmr)+umv_dmr,nodo_aux->tamanio);
				nodo_aux->dmr = posicion_actual; //Actualizamos su base virtual, ahora esta en la que era la 1er pos libre
			}
			posicion_actual += nodo_aux ->tamanio; //le reasignamos la 1er pos libre a posicion_actual
			nodo_aux = nodo_aux->siguiente;
		}
	}
}

int obtener_bloque_ff (nodo_segmento * nodo) {
	if (nodo->tamanio > TOPE) return -1; //si el tamanio del nodo a insertar es mayor que el espacio disponible en memoria por configuracion
	if (!lsu) { //si lsu no posee nodos
		lsu = nodo;
		nodo->dmr = 0;
		nodo->siguiente = NULL;
		return 0;
	}
	if (nodo->tamanio <= lsu->dmr) {
		// en este caso, el nodo entrante es de menor tamanio que el nodo que estaba primero
		// (podria darse si la lista ya estaba en uso y se libero al nodo que estaba primero)
		nodo->dmr = 0;
		nodo->siguiente = lsu;
		lsu = nodo;
		return 0;
	}
	nodo_segmento *nodo_lista;
	nodo_segmento *nodo_anterior;
	unsigned mem_disp = 0;
	unsigned fin_anterior = 0;
	unsigned espacio;
	for (nodo_lista = lsu; nodo_lista; nodo_anterior = nodo_lista, nodo_lista = nodo_lista->siguiente) {
		espacio = nodo_lista->dmr - fin_anterior; //siempre calcula la diferencia entre el siguiente (que posee el nodo_lista) y el anterior
		if (espacio >= nodo->tamanio) { //si hay espacio!
			nodo->dmr = fin_anterior;
			nodo_anterior->siguiente = nodo; //inserto ordenado por DMR
			nodo->siguiente = nodo_lista;
			return fin_anterior;
		}
		fin_anterior = nodo_lista->dmr + nodo_lista->tamanio;
		mem_disp += espacio;
	}
	// puede ser que se pueda insertar al final
	// si llega hasta aca es porque no pude insertarlo antes
	espacio = TOPE - fin_anterior;
	if (espacio >= nodo->tamanio) {
		nodo->dmr = fin_anterior;
		nodo_anterior->siguiente = nodo;
		nodo->siguiente = NULL;
		return fin_anterior;
	}
	mem_disp += espacio;
	// no hubo lugar
	if (mem_disp >= nodo->tamanio) {
		compactar();
		return obtener_bloque_ff(nodo);
	}
	return -1;
}

int obtener_bloque_wf (nodo_segmento *nodo) {
	// ...
	return 1;
}

int reservarSegmento(int pid, int tamanio) {
	nodo_segmento *nodo = malloc(sizeof(nodo_segmento));
	if (nodo == 0) return -1;
	else {
		srand(time(0));
		int base = (rand() & 0x7ff) << 20; // 0,05% chance de colision, corregir
		nodo->base = base;
		nodo->tamanio = tamanio;
		nodo->pid = pid;
		int estado = obtener_bloque(nodo);
		if (estado == -1)
			// no hay memoria, se debe sacar los nodos que sean de este programa y avisar al PLP del fallo
		return base;
	}
}
int escribirSegmento(int pid, int direccion_virtual, int tamanio,char *buffer) {
	nodo_segmento *nodo = buscar_nodo(pid, direccion_virtual);
	unsigned limite,desplazamiento;
	if (!nodo)
		//no existe el nodo
	limite = nodo->base + nodo->tamanio;
	// limite: primera direccion virtual que no es valida
	if ((direccion_virtual + tamanio) > limite)
		// esta tratando de escribir fuera de su segmento!
	desplazamiento = direccion_virtual - nodo->base;
	char *direccion_fisica = (umv_dmr + nodo->dmr) + desplazamiento;
	memcpy(direccion_fisica, buffer, tamanio);
	return 1;
}
int reservarMallocInicial() {
	umv_dmr = malloc(TOPE);
	if (umv_dmr) return 1;
	else return -1;
}

nodo_segmento *buscar_nodo (int pid, int direccion_virtual) {
	nodo_segmento *nodo;
	for (nodo = lsu; nodo; nodo = nodo->siguiente) {
		if (nodo->pid != pid) continue;
		if (direccion_virtual < nodo->base) continue;
		if (direccion_virtual >= (nodo->base + nodo->tamanio)) continue;
		return nodo;
	}
	return NULL;
}

char *leer(int pid, int direccion_virtual, int tamanio) {
	nodo_segmento * nodo = buscar_nodo(pid, direccion_virtual);
	unsigned limite, desplazamiento;
	if (!nodo)
		//no existe nodo
	limite = nodo -> base + nodo -> tamanio;
	// limite: primera direccion virtual que no es valida
	if ((direccion_virtual + tamanio) > limite)
		//tratando de leer mas alla de su segmento
	desplazamiento = direccion_virtual - nodo->base;
	char *direccion_fisica = (umv_dmr + nodo->dmr) + desplazamiento;
	char *buffer = malloc(tamanio);
	memcpy(buffer, direccion_fisica, tamanio);
	return buffer;
}

void borrarPrograma(int pid){
	nodo_segmento * nodo_aux, * nodo_anterior, *nodo_a_liberar;
	nodo_aux = lsu;
	nodo_anterior = lsu;
	while(nodo_aux){
		if (lsu->pid == pid){  //Caso Base. Si el Primer nodo de la lista contiene el PID
			nodo_a_liberar = lsu; //LSU es el primer nodo, por lo que es el que queremos liberar
			lsu = lsu->siguiente; //LSU ahora debe apuntar al 2do nodo de la lista. El 1ero se liberará.
			free(nodo_a_liberar);
		}
		else {
			if (nodo_aux->pid == pid){ //Seguimos recorriendo la lista. Encontramos el PID en nodo_aux
				nodo_a_liberar = nodo_aux;
				nodo_aux = nodo_aux->siguiente; //nodo_aux debe apuntar al sig nodo del que se liberará.
				nodo_anterior->siguiente = nodo_aux; //nodo_anterior debe apuntar al nodo anterior del que se liberará.
				free(nodo_a_liberar);
			} else  {
				nodo_anterior = nodo_aux; //Ahora nodo_anterior sera el aux y seguimos recorriendo a la lista
				nodo_aux = nodo_aux->siguiente; //nodo_aux sera el siguiente nodo a evaluar
			}
		}
	}
}

void destruirSegmento(int pid,  int base){
	nodo_segmento * nodo_aux, * nodo_anterior, *nodo_a_liberar;
	nodo_aux = lsu;
	nodo_anterior = lsu;
	while(nodo_aux){
		if (lsu->pid == pid && (lsu->base == base)){  //Caso Base. Si el Primer nodo de la lista contiene el PID
			nodo_a_liberar = lsu; //LSU es el primer nodo, por lo que es el que queremos liberar
			lsu = lsu->siguiente; //LSU ahora debe apuntar al 2do nodo de la lista. El 1ero se liberará.
			free(nodo_a_liberar);
		} else {
			if ((nodo_aux->pid == pid) && (nodo_aux->base == base)) {
				nodo_a_liberar = nodo_aux;
				nodo_aux = nodo_aux->siguiente;
				nodo_anterior->siguiente = nodo_aux;
				free(nodo_a_liberar);
			} else  {
				nodo_anterior = nodo_aux; //Ahora nodo_anterior sera el aux y seguimos recorriendo a la lista
				nodo_aux = nodo_aux->siguiente; //nodo_aux sera el siguiente nodo a evaluar
			}
		}
	}
}
/*
int main(int argc, char **argv) {
	return 0;
}*/
