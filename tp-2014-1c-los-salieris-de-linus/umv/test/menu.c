

void menu() {
	puts("********CONSOLA UMV*********");
	puts("Comandos:");
	puts("*\t 1.  COMPACTAR");
	puts("*\t 2.  OPERACION CREAR SEGMENTO <pid> <base> <tamaño> -log");
	puts("*\t 3.  OPERACION DESTRUIR SEGMENTO <pid> <base> -log");
	puts("*\t 4.  OPERACION ESCRIBIR <pid> <base> <offset> <tamaño> <buffer> -log");
	puts("*\t 5.  OPERACION LEER <pid> <base> <offset> <tamaño> -log");
	puts("*\t 6.  OPERACION DESTRUIR PROGRAMA <pid> -log");
	puts("*\t 7.  RETARDO <milisegundos>");
	puts("*\t 8.  ALGORITMO <algoritmo>: Cambia de algoritmo de insercion entre FIRST-FIT o WORST-FIT");
	puts("*\t 9.  DUMP ESTRUCTURAS");
	puts("*\t 10. DUMP ESTRUCTURAS PROGRAMA <pid>");
	puts("*\t 11. DUMP MEM_PPAL");
	puts("*\t 12. DUMP CONTENIDO_MEM_PPAL");
	puts("*\t 13. SALIR DEL PROGRAMA");
	puts("********************************");
}

void menuCompactar(){
	char confirmacion;
	puts("CONFIRMAR S/N: ");
	scanf("%s", &confirmacion);
	if (strcmp(confirmacion,'S')) {
			compactar();
	}
}
void menuCrearSegmento(){
	int pid,base,offset,tamanio;
	char confirmacion;
	puts("Ingrese PID: ");
	scanf("%d", &pid);
	puts("Ingrese base: ");
	scanf("%d", &base);
	puts("Ingrese tamanio: ");
	scanf("%d", &tamanio);
	puts("CONFIRMAR S/N: ");
	scanf("%s", &confirmacion);
	if (strcmp(confirmacion,'S')) {
		reservarSegmento(pid,tamanio); // Y los demas parametros??
	}
}
void menuBorrarSegmento(){
	int pid,base;
	char confirmacion;
	puts("Ingrese PID: ");
	scanf("%d", &pid);
	puts("Ingrese base: ");
	scanf("%d", &base);
	puts("CONFIRMAR S/N: ");
	scanf("%s", &confirmacion);
	if (strcmp(confirmacion,'S')) {
			//ejecutar
	}
}
void menuEscribirSegmento(){
	int pid,base,offset,tamanio;
	char* buffer;
	char confirmacion;
	puts("Ingrese PID: ");
	scanf("%d", &pid);
	puts("Ingrese base: ");
	scanf("%d", &base);
	puts("Ingrese desplazamiento: ");
	scanf("%d", &offset);
	puts("Ingrese tamanio: ");
	scanf("%d", &tamanio);
	puts("Ingrese contenido: ");
	scanf("%s", buffer);
	puts("CONFIRMAR S/N: ");
	scanf("%s", &confirmacion);
	if (strcmp(confirmacion,'S')) {
		escribirSegmento(pid,base+offset,tamanio,buffer);
	}
}
void menuLeerSegmento(){
	int pid,base,offset,tamanio;
	char confirmacion;
	puts("Ingrese PID: ");
	scanf("%d", &pid);
	puts("Ingrese base: ");
	scanf("%d", &base);
	puts("Ingrese desplazamiento: ");
	scanf("%d", &offset);
	puts("Ingrese tamanio: ");
	scanf("%d", &tamanio);
	puts("CONFIRMAR S/N: ");
	scanf("%s", &confirmacion);
	if (strcmp(confirmacion,'S')) {
		leer(pid,base+offset,tamanio);
	}
}
void menuDestruirPrograma(){
	int pid;
	char confirmacion;
	puts("Ingrese PID: ");
	scanf("%d", &pid);
	puts("CONFIRMAR S/N: ");
	scanf("%s", &confirmacion);
	if (strcmp(confirmacion,'S')) {
		borrarPrograma(pid);
	}
}
void menuRetardo(){
	int retardo;
	char confirmacion;
	puts("Ingrese Retardo: ");
	scanf("%d", &retardo);
	puts("CONFIRMAR S/N: ");
	scanf("%s", &confirmacion);
	if (strcmp(confirmacion,'S')) {
		RETARDO = retardo;
	}
}
void menuAlgoritmo(){
	int algoritmo;
	char confirmacion;
	puts("Ingrese numero de algoritmo a utilizar. 1 = FirstFit. 2 = WorstFit: ");
	scanf("%d", &algoritmo);
	puts("CONFIRMAR S/N: ");
	sscanf("%s", &confirmacion);
	if (strcmp(confirmacion,'S')) {
		/*if (algoritmo == 1) obtener_bloque = obtener_bloque_ff;
		else if (algoritmo == 2) obtener_bloque = obtener_bloque_wf;
		*/
	}
}
void menuDumpEstructuras(){
	char confirmacion;
	puts("CONFIRMAR S/N: ");
	scanf("%s", &confirmacion);
	if (strcmp(confirmacion,'S')) {

	}
}
void menuDumpEstructurasPrograma(){
	int pid;
	char confirmacion;
	puts("Ingrese PID: ");
	scanf("%d", &pid);
	puts("CONFIRMAR S/N: ");
	scanf("%s", &confirmacion);
	if (strcmp(confirmacion,'S')) {
			//ejecutar
	}
}
void menuDumpMemPrincipal(){
	char confirmacion;
	puts("CONFIRMAR S/N: ");
	scanf("%s", &confirmacion);
	if (strcmp(confirmacion,'S')) {

	}
}
void menuDumpContentMemPrincipal(){
	char confirmacion;
	puts("CONFIRMAR S/N: ");
	scanf("%s", &confirmacion);
	if (strcmp(confirmacion,'S')) {
	}
}
