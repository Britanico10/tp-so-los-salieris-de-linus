#include <stdio.h>
#include <libio.h>
#include <string.h>
#include <stdlib.h>

void menuCompactar(void);
void menuCrearSegmento(void);
void menuBorrarSegmento(void);
void menuEscribirSegmento(void);
void menuLeerSegmento(void);
void menuDestruirPrograma(void);
void menuRetardo(void);
void menuAlgoritmo(void);
void menuDumpEstructuras(void);
void menuDumpEstructurasPrograma(void);
void menuDumpMemPrincipal(void);
void menuDumpContentMemPrincipal(void);

void menuCompactar(){
	char confirmacion[1];
	printf("CONFIRMAR S/N: ");
	scanf("%s", confirmacion);
	if ((int)(*confirmacion) == 's' || (int)(*confirmacion) == 'S'){
			compactar();
	}
}
void menuCrearSegmento(){
	int pid,base,offset,tamanio;
	char confirmacion[1];
	printf("Ingrese PID: ");
	scanf("%d", &pid);
	printf("Ingrese base: ");
	scanf("%d", &base);
	printf("Ingrese tamanio: ");
	scanf("%d", &tamanio);
	printf("CONFIRMAR S/N: ");
	scanf("%s", confirmacion);
	if ((int)(*confirmacion) == 's' || (int)(*confirmacion) == 'S')
		base = reservarSegmento(pid,tamanio); // Y los demas parametros??
	printf("Base otorgada: %d\n", base);
}
void menuBorrarSegmento(){
	int pid,base;
	char confirmacion[1];
	printf("Ingrese PID: ");
	scanf("%d", &pid);
	printf("Ingrese base: ");
	scanf("%d", &base);
	printf("CONFIRMAR S/N: ");
	scanf("%s", confirmacion);
	if ((int)(*confirmacion) == 's' || (int)(*confirmacion) == 'S'){
			//ejecutar
	}
}
void menuEscribirSegmento(){
	int pid,base,offset,tamanio;
	char* buffer;
	char confirmacion[1];
	printf("Ingrese PID: ");
	scanf("%d", &pid);
	printf("Ingrese base: ");
	scanf("%d", &base);
	printf("Ingrese desplazamiento: ");
	scanf("%d", &offset);
	printf("Ingrese tamanio: ");
	scanf("%d", &tamanio);
	printf("Ingrese contenido: ");
	scanf("%s", buffer);
	printf("CONFIRMAR S/N: ");
	scanf("%s", confirmacion);
	if ((int)(*confirmacion) == 's' || (int)(*confirmacion) == 'S'){
		escribirSegmento(pid,base+offset,tamanio,buffer);
	}
}
void menuLeerSegmento(){
	int pid,base,offset,tamanio;
	char confirmacion[1];
	printf("Ingrese PID: ");
	scanf("%d", &pid);
	printf("Ingrese base: ");
	scanf("%d", &base);
	printf("Ingrese desplazamiento: ");
	scanf("%d", &offset);
	printf("Ingrese tamanio: ");
	scanf("%d", &tamanio);
	printf("CONFIRMAR S/N: ");
	scanf("%s", confirmacion);
	if ((int)(*confirmacion) == 's' || (int)(*confirmacion) == 'S'){
		leer(pid,base+offset,tamanio);
	}
}
void menuDestruirPrograma(){
	int pid;
	char confirmacion[1];
	printf("Ingrese PID: ");
	scanf("%d", &pid);
	printf("CONFIRMAR S/N: ");
	scanf("%s", confirmacion);
	if ((int)(*confirmacion) == 's' || (int)(*confirmacion) == 'S'){
		borrarPrograma(pid);
	}
}
void menuRetardo(){
	int retardo;
	char confirmacion[1];
	printf("Ingrese Retardo: ");
	scanf("%d", &retardo);
	printf("CONFIRMAR S/N: ");
	scanf("%s", confirmacion);
	if ((int)(*confirmacion) == 's' || (int)(*confirmacion) == 'S'){
		//RETARDO = retardo;
	}
}
void menuAlgoritmo(){
	int algoritmo;
	char confirmacion[1];
	printf("Ingrese numero de algoritmo a utilizar. 1 = FirstFit. 2 = WorstFit: ");
	scanf("%d", &algoritmo);
	printf("CONFIRMAR S/N: ");
	scanf("%s", confirmacion);
	if ((int)(*confirmacion) == 's' || (int)(*confirmacion) == 'S'){
		/*if (algoritmo == 1) obtener_bloque = obtener_bloque_ff;
		else if (algoritmo == 2) obtener_bloque = obtener_bloque_wf;
		*/
	}
}
void menuDumpEstructuras(){
	char confirmacion[1];
	printf("CONFIRMAR S/N: ");
	scanf("%s", confirmacion);
	if ((int)(*confirmacion) == 's' || (int)(*confirmacion) == 'S'){

	}
}
void menuDumpEstructurasPrograma(){
	int pid;
	char confirmacion[1];
	printf("Ingrese PID: ");
	scanf("%d", &pid);
	printf("CONFIRMAR S/N: ");
	scanf("%s", confirmacion);
	if ((int)(*confirmacion) == 's' || (int)(*confirmacion) == 'S'){
			//ejecutar
	}
}
void menuDumpMemPrincipal(){
	char confirmacion[1];
	printf("CONFIRMAR S/N: ");
	scanf("%s", confirmacion);
	if ((int)(*confirmacion) == 's' || (int)(*confirmacion) == 'S'){

	}
}
void menuDumpContentMemPrincipal(){
	char confirmacion[1];
	printf("CONFIRMAR S/N: ");
	scanf("%s", confirmacion);
	if ((int)(*confirmacion) == 's' || (int)(*confirmacion) == 'S'){
	}
}

void consola(){
	unsigned option;

	while(1){
		puts("********CONSOLA UMV*********");
		puts("*\tComandos:");
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
		printf("Ingrese numero de operacion: ");
		scanf("%d",&option);
		switch (option) {
			case 1: menuCompactar(); break;
			case 2: menuCrearSegmento(); break;
			case 3: menuBorrarSegmento();break;
			case 4: menuEscribirSegmento();break;
			case 5: menuLeerSegmento();break;
			case 6: menuDestruirPrograma();break;
			case 7: menuRetardo();break;
			case 8: menuAlgoritmo();break;
			case 9: menuDumpEstructuras();break; //Dump Estructuras
			case 10: menuDumpEstructurasPrograma();break;
			case 11: menuDumpMemPrincipal();break; //Dump Memoria Principal
			case 12: menuDumpContentMemPrincipal();break; //Dump Contenido Memoria Principal
			case 13: exit(0);
		}
	}
}

int main(int argc, char ** argv) {
	consola();
	return 0;
}
