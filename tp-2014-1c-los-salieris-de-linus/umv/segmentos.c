#include "umv.h"
#include <parser/metadata_program.h>

extern nodo_segmento *lsu;
extern int (* obtener_bloque) (nodo_segmento *);
extern int TOPE;
extern int RETARDO;
extern int PUERTO;
extern char *umv_dmr;

extern pthread_rwlock_t semaforo_lsu;

void compactar (void) {
	pthread_rwlock_wrlock(&semaforo_lsu);
	nodo_segmento *nodo_aux;
	if (lsu){ //Si lsu no es null
		int posicion_actual = lsu->tamanio;
		if (lsu->dmr != 0) {  //Si el primer segmento no empieza en la 1er posicion
			memmove(umv_dmr,(lsu->dmr)+umv_dmr,lsu->tamanio);
			lsu->dmr = 0; //Como se lo mueve al principio, su base virtual es cero
		}
		nodo_aux = lsu->siguiente;
		while(nodo_aux) { //Nos adentramos en la lista lsu
			if (posicion_actual != nodo_aux->dmr){ //si el segmento no esta en donde deberia estar (En la 1er pos libre)
				memmove(posicion_actual+umv_dmr,(nodo_aux->dmr)+umv_dmr,nodo_aux->tamanio);
				nodo_aux->dmr = posicion_actual; //Actualizamos su base virtual, ahora esta en la que era la 1er pos libre
			}
			posicion_actual += nodo_aux->tamanio; //le reasignamos la 1er pos libre a posicion_actual
			nodo_aux = nodo_aux->siguiente;
		}
	}
	pthread_rwlock_unlock(&semaforo_lsu);
}

int obtener_bloque_wf (nodo_segmento *nodo) {
	//Verificamos que el nodo que se quiere insertar no posea un tamanio mayor a la memoria disponible, por configuracion
	if (nodo->tamanio > TOPE) return -1;
	//Verificamos si lsu aun no posee nodos. Entonces este seria el primero y directamente es insertado al principio
	if (!lsu) {
		lsu = nodo;
		nodo->dmr = 0;
		nodo->siguiente = NULL;
		return 0;
	}
	unsigned mayor_espacio_disponible = 0;
	nodo_segmento *nodo_aux, *nodo_anterior;
	nodo_segmento *nodo_max = NULL;
	unsigned mem_disp = 0;
	unsigned fin_anterior = 0;
	unsigned espacio;
	nodo_aux = lsu;
	nodo_anterior = NULL;
	while(nodo_aux) {
		espacio = nodo_aux->dmr - fin_anterior;  //siempre calcula el espacio entre el anterior y el mismo
		if (espacio > mayor_espacio_disponible) {
			mayor_espacio_disponible = espacio;
			nodo_max = nodo_anterior;
		}
  		fin_anterior = nodo_aux->dmr + nodo_aux->tamanio;
		nodo_anterior = nodo_aux;
		nodo_aux = nodo_aux->siguiente;
		mem_disp += espacio;
	}
	if (fin_anterior != TOPE) {
		espacio = TOPE - fin_anterior;
		if (espacio > mayor_espacio_disponible) {
			mayor_espacio_disponible = espacio;
			nodo_max = nodo_anterior;
		}
		mem_disp += espacio;
	}
	if (mayor_espacio_disponible >= nodo->tamanio) {
		if (nodo_max) {
			nodo -> dmr = nodo_max -> dmr + nodo_max -> tamanio;
			nodo -> siguiente = nodo_max -> siguiente;
			nodo_max -> siguiente = nodo;
			return nodo -> dmr;
		} else {
			nodo -> dmr = 0;
			nodo -> siguiente = lsu;
			lsu = nodo;
			return 0;
		}
	}
	/*Si aun no salio por un return
	*significa que el tamanio del nodo no entra en el mayor espacio libre
	*Si la memoria disponible es suficiente, entonces hay que compactar y volver a empezar*/
	if (nodo -> tamanio <= mem_disp) {
		compactar();
		return obtener_bloque_wf(nodo);
	}
	//Si llega a este punto es que no se compacto, ya que ni haciendo eso habia lugar para el nodo
	return -1;
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
		pthread_rwlock_unlock(&semaforo_lsu);
		compactar();
		pthread_rwlock_wrlock(&semaforo_lsu);
		return obtener_bloque_ff(nodo);
	}
	return -1;
}

unsigned conseguirBaseRandom(int pid, int tamanio) {
  unsigned base = ((rand() & 0xffff) | (rand() << 16)) & 0xffffffffU;
	if (!validar_base(pid, tamanio, base))
		return conseguirBaseRandom(pid, tamanio);
	return base;
}

int validar_base (int pid, int tamanio, unsigned base) {
	nodo_segmento * nodo;
	for (nodo = lsu; nodo; nodo = nodo -> siguiente) {
		if (nodo -> pid != pid) continue;
		if ((base >= (nodo -> base)) && (base < (nodo -> base + nodo -> tamanio))) return 0;
		if (((base + tamanio) > (nodo -> base)) && ((base + tamanio) <= (nodo -> base + nodo -> tamanio))) return 0;
	}
	return 1;
}

int reservarSegmento(int pid, int tamanio) {
	pthread_rwlock_wrlock(&semaforo_lsu);
	nodo_segmento *nodo = malloc(sizeof(nodo_segmento));
	if (nodo == 0) {
		pthread_rwlock_unlock(&semaforo_lsu);
		return -1;
	}
	else {
		int base = conseguirBaseRandom(pid,tamanio);
		nodo->base = base;
		nodo->tamanio = tamanio;
		nodo->pid = pid;
		int estado = obtener_bloque(nodo);
		if (estado == -1) {
			pthread_rwlock_unlock(&semaforo_lsu);
			return -1;
		}
		pthread_rwlock_unlock(&semaforo_lsu);
		return base;
	}
}

int escribirSegmento(int pid, int direccion_virtual, int tamanio,char *buffer) {
	pthread_rwlock_rdlock(&semaforo_lsu);
	nodo_segmento *nodo = buscar_nodo(pid, direccion_virtual);
	unsigned limite,desplazamiento;
	if (!nodo) { //segmentation fault
		pthread_rwlock_unlock(&semaforo_lsu);
		return -1;
	}
	limite = nodo->base + nodo->tamanio;
	// limite: primera direccion virtual que no es valida
	if ((direccion_virtual + tamanio) > limite) { //segmentation fault
		pthread_rwlock_unlock(&semaforo_lsu);
		return -1;
	}
	desplazamiento = direccion_virtual - nodo->base;
	char *direccion_fisica = (umv_dmr + nodo->dmr) + desplazamiento;
	memcpy(direccion_fisica, buffer, tamanio);
	pthread_rwlock_unlock(&semaforo_lsu);
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



char *leerSegmento(int pid, int direccion_virtual, int tamanio) {
	//t_log* logger = log_create(fileLeida,"LEERSEGMENTO",true,LOG_LEVEL_INFO);
	pthread_rwlock_rdlock(&semaforo_lsu);
	nodo_segmento * nodo = buscar_nodo(pid, direccion_virtual);
	unsigned limite, desplazamiento;
	if (!nodo) { //el nodo no existe --> segmentation fault
		pthread_rwlock_unlock(&semaforo_lsu);
		return NULL;
	}
	limite = nodo->base + nodo->tamanio;
	// limite: primera direccion virtual que no es valida
	if ((direccion_virtual + tamanio) > limite) { //segmentation fault
		pthread_rwlock_unlock(&semaforo_lsu);
		return NULL;
	}
	desplazamiento = direccion_virtual - nodo->base;
	char *direccion_fisica = (umv_dmr + nodo->dmr) + desplazamiento;
	char *buffer = malloc(tamanio);
	memcpy(buffer, direccion_fisica, tamanio);
	pthread_rwlock_unlock(&semaforo_lsu);
	return buffer;
}

nodo_segmento * buscar_nodos_de_un_programa(int pid) {
	nodo_segmento * nodo_aux, * nodo_a_programa, * nodo_dato, * ultimo_nodo;
	pthread_rwlock_rdlock(&semaforo_lsu);
	nodo_aux = lsu;
	nodo_a_programa = NULL;
	ultimo_nodo = NULL;
	while(nodo_aux){
		if (nodo_aux->pid == pid) {
			nodo_dato = malloc(sizeof(nodo_segmento));
			memcpy(nodo_dato,nodo_aux,sizeof(nodo_segmento));
			nodo_dato->siguiente = NULL;
			if (nodo_a_programa == NULL) ultimo_nodo = nodo_a_programa = nodo_dato;
			else
				ultimo_nodo = ultimo_nodo->siguiente = nodo_dato;
		}
		nodo_aux = nodo_aux->siguiente;
	}
	pthread_rwlock_unlock(&semaforo_lsu);
	return nodo_a_programa;
}

void borrarPrograma(int pid){
	pthread_rwlock_wrlock(&semaforo_lsu);
	nodo_segmento * nodo_aux, * nodo_anterior, *nodo_a_liberar;
	nodo_aux = lsu;
	nodo_anterior = lsu;
	while(nodo_aux){
		if (lsu->pid == pid){  //Caso Base. Si el primer nodo de la lista contiene el PID
			nodo_a_liberar = lsu; //LSU es el primer nodo, por lo que es el que queremos liberar
			lsu = lsu->siguiente; //LSU ahora debe apuntar al 2do nodo de la lista. El 1ero se liberará.
			nodo_aux = nodo_aux->siguiente;
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
		if (!lsu) break;
	}
	pthread_rwlock_unlock(&semaforo_lsu);
}

void destruirSegmento(int pid,  int base){
	pthread_rwlock_wrlock(&semaforo_lsu);
	nodo_segmento * nodo_aux, * nodo_anterior, *nodo_a_liberar;
	nodo_aux = lsu;
	nodo_anterior = lsu;
	while(nodo_aux){
		if ((lsu->pid == pid) && (lsu->base == base)){  //Caso Base. Si el Primer nodo de la lista contiene el PID
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
		if (!lsu) break;
	}
	pthread_rwlock_unlock(&semaforo_lsu);
}

void dumpEstructuras(void){
	nodo_segmento * nodo_aux;
	unsigned num_esp_disp = 0, total_espacio = 0;
	pthread_rwlock_rdlock(&semaforo_lsu);
	unsigned espacio = 0, fin_anterior = 0;
	if (lsu) nodo_aux = lsu;
	else {
		puts("Todavia no hay segmentos creados");
		pthread_rwlock_unlock(&semaforo_lsu);
		return;
	}
	puts("PID\tBASE\t\tTAMANIO\t\tDMR");
	// error cuando hay varios espacios!
	puts("---------------------------------------------------------------");
	while(nodo_aux) {
		if (fin_anterior != nodo_aux->dmr) {
			espacio = nodo_aux->dmr - fin_anterior;
			total_espacio += espacio;
			printf("ESPACIO DISPONIBLE #%d\t%u\t\t%u\n", ++num_esp_disp, espacio, fin_anterior);
		}
		printf("%u\t%u\t%u\t\t%u\n",nodo_aux->pid, nodo_aux->base,nodo_aux->tamanio,nodo_aux->dmr);
		fin_anterior = nodo_aux->dmr + nodo_aux->tamanio;
		nodo_aux = nodo_aux->siguiente;
	}
	if ((TOPE - fin_anterior) > 0)
		printf("ESPACIO DISPONIBLE #%d\t%u\t\t%u\n", ++num_esp_disp, TOPE - fin_anterior, fin_anterior);
	puts("---------------------------------------------------------------");
	printf("TOTAL ESPACIO DISPONIBLE: %ubytes\n", total_espacio + TOPE - fin_anterior);
	puts("---------------------------------------------------------------");
	pthread_rwlock_unlock(&semaforo_lsu);
}

void dumpEstructurasPrograma(int pid){
	//Muestras el contenido de las estructuras de cierto Programa
	nodo_segmento * nodo_aux;
	pthread_rwlock_rdlock(&semaforo_lsu);
	nodo_aux = buscar_nodos_de_un_programa(pid);
	if (!nodo_aux){
		puts("Todavia no hay segmentos creados para dicho programa");
		pthread_rwlock_unlock(&semaforo_lsu);
		return;
	}
	else {
		puts("PID\tBASE\t\tTAMANIO\t\tDMR");
		puts("---------------------------------------------------------------");
		while(nodo_aux) {
			printf("%u\t%u\t%u\t\t%u\n",nodo_aux->pid, nodo_aux->base,nodo_aux->tamanio,nodo_aux->dmr);
			nodo_aux = nodo_aux->siguiente;
		}
		puts("---------------------------------------------------------------");
		pthread_rwlock_unlock(&semaforo_lsu);
	}
	free(nodo_aux);
}

void dumpMemPrincipalContenido(int offset, int tamanio){
	//validar que este dentro de memoria y no afuera
	pthread_rwlock_rdlock(&semaforo_lsu);
	char * resultado = malloc(tamanio);
	memcpy(resultado,umv_dmr+offset,tamanio);
	mostrar_hexadecimal(resultado, tamanio, offset);
	pthread_rwlock_unlock(&semaforo_lsu);
	free(resultado);
}
