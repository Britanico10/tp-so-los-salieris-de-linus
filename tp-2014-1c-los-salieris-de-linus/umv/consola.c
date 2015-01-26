#include "umv.h"

extern nodo_segmento *lsu;
extern int (* obtener_bloque) (nodo_segmento *);
extern int TOPE;
extern int RETARDO;
extern int PUERTO;
extern char *umv_dmr;

char * leer_linea(void) {
	char * linea = malloc(1);
	unsigned longitud = 0;
	int lectura;
	for (lectura = getchar(); (lectura != EOF) && (lectura != '\n'); lectura = getchar()) {
		linea = realloc(linea,longitud+2);
		linea[longitud++] = lectura;
	}
	linea[longitud] = 0;
	return linea;
}

void mostrar_hexadecimal (void * datos, unsigned cantidad, unsigned dir_base) {
  if (!(datos && cantidad)) return;
  printf("          00 01 02 03  04 05 06 07 - 08 09 0A 0B  0C 0D 0E 0F  0123456789ABCDEF\n"
         "--------- -- -- -- --  -- -- -- --   -- -- -- --  -- -- -- --  ----------------\n");
  if ((cantidad + (dir_base % 16)) <= 16) {
    mostrar_linea_hexadecimal(datos, cantidad, dir_base);
    return;
  }
  unsigned char * dp = datos;
  unsigned char pos;
  if (dir_base % 16) {
    pos = 16 - (dir_base % 16);
    mostrar_linea_hexadecimal(datos, pos, dir_base);
    dp += pos;
    dir_base += pos;
    cantidad -= pos;
  }
  char linea[82];
  char buf[10];
  while (cantidad >= 16) {
    strcpy(linea, "********: ?? ?? ?? ??  ?? ?? ?? ?? - ?? ?? ?? ??  ?? ?? ?? ??  ................\n");
    sprintf(buf, "%08X", dir_base);
    memcpy(linea, buf, 8);
    for (pos = 0; pos < 16; pos ++) {
      sprintf(buf, "%02hhX", dp[pos]);
      memcpy(linea + 10 + 3 * pos + (pos / 4) + (pos / 8), buf, 2);
      if ((dp[pos] >= 32) && (dp[pos] <= 126)) linea[63 + pos] = dp[pos];
    }
    printf("%s", linea);
    dp += 16;
    dir_base += 16;
    cantidad -= 16;
  }
  if (cantidad) mostrar_linea_hexadecimal(dp, cantidad, dir_base);
}

void mostrar_linea_hexadecimal (void * datos, unsigned cantidad, unsigned dir_base) {
  char linea[81];
  char buf[10];
  unsigned char inicio, pos, fin;
  memset(linea, ' ', 79);
  linea[79] = '\n';
  linea[80] = 0;
  linea[8] = ':';
  inicio = dir_base % 16;
  dir_base &= ~15;
  fin = inicio + cantidad;
  sprintf(buf, "%08X", dir_base);
  memcpy(linea, buf, 8);
  if ((inicio < 8) && (fin > 8)) linea[35] = '-';
  for (pos = inicio; pos < fin; pos ++) {
    sprintf(buf, "%02hhX", (pos - inicio)[(unsigned char *) datos]);
    memcpy(linea + 10 + 3 * pos + (pos / 4) + (pos / 8), buf, 2);
    linea[63 + pos] = (((pos - inicio)[(unsigned char *) datos] >= 32) && ((pos - inicio)[(unsigned char *) datos] <= 126)) ?
                      (pos - inicio)[(unsigned char *) datos] : '.';
  }
  printf("%s", linea);
}

void menuCompactar(){
	char confirmacion;
	printf("CONFIRMAR S/N: ");
	scanf("%c", &confirmacion); while (getchar() != '\n');
	if ((confirmacion == 's') || (confirmacion == 'S')) {
		compactar();
		dumpEstructuras();
	}
}

void menuCrearSegmento(){
	unsigned pid,base,tamanio;
	char confirmacion;
	printf("Ingrese PID: ");
	scanf("%d", &pid); while (getchar() != '\n');
	printf("Ingrese tamanio: ");
	scanf("%d", &tamanio); while (getchar() != '\n');
	printf("CONFIRMAR S/N: ");
	scanf("%c", &confirmacion); while (getchar() != '\n');
	if ((confirmacion == 's') || (confirmacion == 'S')) {
		base = reservarSegmento(pid,tamanio); // puede fallar!
	}
	printf("Base otorgada: %u\n", base);
}

void menuBorrarSegmento(){
	int pid,base;
	char confirmacion;
	printf("Ingrese PID: ");
	scanf("%d", &pid); while (getchar() != '\n');
	printf("Ingrese base: ");
	scanf("%d", &base); while (getchar() != '\n');
	printf("CONFIRMAR S/N: ");
	scanf("%c", &confirmacion); while (getchar() != '\n');
	if ((confirmacion == 's') || (confirmacion == 'S')) {
		destruirSegmento(pid,base);
	}
}

void menuEscribirSegmento(){
	int pid,base,offset;
	char* buffer;
	char confirmacion;
	printf("Ingrese PID: ");
	scanf("%d", &pid); while (getchar() != '\n');
	printf("Ingrese base: ");
	scanf("%d", &base); while (getchar() != '\n');
	printf("Ingrese desplazamiento: ");
	scanf("%d", &offset); while (getchar() != '\n');
	printf("Ingrese contenido: ");
	buffer = leer_linea();
	printf("CONFIRMAR S/N: ");
	scanf("%c", &confirmacion); while (getchar() != '\n');
	if ((confirmacion == 's') || (confirmacion == 'S')) {
		escribirSegmento(pid,base+offset,strlen(buffer)+1,buffer); //puede fallar!
	}
}

void menuLeerSegmento(){
	int pid,base,offset,tamanio;
	char *buffer;
	char confirmacion;
	printf("Ingrese PID: ");
	scanf("%d", &pid); while (getchar() != '\n');
	printf("Ingrese base: ");
	scanf("%d", &base); while (getchar() != '\n');
	printf("Ingrese desplazamiento: ");
	scanf("%d", &offset); while (getchar() != '\n');
	printf("Ingrese tamanio: ");
	scanf("%d", &tamanio); while (getchar() != '\n');
	printf("CONFIRMAR S/N: ");
	scanf("%c", &confirmacion); while (getchar() != '\n');
	if ((confirmacion == 's') || (confirmacion == 'S')) {
		buffer = leerSegmento(pid,base+offset,tamanio); //puede fallar!
		mostrar_hexadecimal(buffer, tamanio, base + offset);
		free(buffer);
	}
}

void menuDestruirPrograma(){
	int pid;
	char confirmacion;
	printf("Ingrese PID: ");
	scanf("%d", &pid); while (getchar() != '\n');
	printf("CONFIRMAR S/N: ");
	scanf("%c", &confirmacion); while (getchar() != '\n');
	if ((confirmacion == 's') || (confirmacion == 'S')) {
		borrarPrograma(pid);
	}
}

void menuRetardo(){
	int ret;
	char confirmacion;
	printf("Retardo actual: %d (en milisegundos)\n", RETARDO);
	printf("Ingrese nuevo retardo: ");
	scanf("%d", &ret); while (getchar() != '\n');
	printf("CONFIRMAR S/N: ");
	scanf("%c", &confirmacion); while (getchar() != '\n');
	if ((confirmacion == 's') || (confirmacion == 'S')) {
		RETARDO = ret;
	}
}

void menuAlgoritmo(){
	int algoritmo;
	char confirmacion;
	puts("Ingrese numero de algoritmo a utilizar: \n\t 1 = FirstFit \n\t 2 = WorstFit ");
	scanf("%d", &algoritmo); while (getchar() != '\n');
	printf("CONFIRMAR S/N: ");
	scanf("%c", &confirmacion); while (getchar() != '\n');
	if ((confirmacion == 's') || (confirmacion == 'S')) {
		switch (algoritmo) {
			case 1: obtener_bloque = obtener_bloque_ff; break;
			case 2: obtener_bloque = obtener_bloque_wf; break;
			default: puts("Codigo invalido");
		}
	}
}

void menuDumpEstructuras(){
	char confirmacion;
	printf("CONFIRMAR S/N: ");
	scanf("%c", &confirmacion); while (getchar() != '\n');
	if ((confirmacion == 's') || (confirmacion == 'S')) {
		dumpEstructuras();
	}
}

void menuDumpEstructurasPrograma(){
	int pid;
	char confirmacion;
	printf("Ingrese PID: ");
	scanf("%d", &pid); while (getchar() != '\n');
	printf("CONFIRMAR S/N: ");
	scanf("%c", &confirmacion); while (getchar() != '\n');
	if ((confirmacion == 's') || (confirmacion == 'S')) {
		dumpEstructurasPrograma(pid);
	}
}

void menuDumpContentMemPrincipal(){
	int offset, tamanio;
	char confirmacion;
	printf("Ingrese desplazamiento: ");
	scanf("%d", &offset); while (getchar() != '\n');
	printf("Ingrese tamanio: ");
	scanf("%d", &tamanio); while (getchar() != '\n');
	printf("CONFIRMAR S/N: ");
	scanf("%c", &confirmacion); while (getchar() != '\n');
	if ((confirmacion == 's') || (confirmacion == 'S')) {
		dumpMemPrincipalContenido(offset, tamanio);
	}
}

void *consola(void * basura){
	unsigned option;

	while(1) {
		puts("********CONSOLA UMV*********");
		puts("*\tComandos:");
		puts("*\t 1.  COMPACTAR");
		puts("*\t 2.  OPERACION CREAR SEGMENTO <pid> <base> <tamanio>");
		puts("*\t 3.  OPERACION DESTRUIR SEGMENTO <pid> <base>");
		puts("*\t 4.  OPERACION ESCRIBIR <pid> <base> <offset> <buffer>");
		puts("*\t 5.  OPERACION LEER <pid> <base> <offset> <tamanio>");
		puts("*\t 6.  OPERACION DESTRUIR PROGRAMA <pid>");
		puts("*\t 7.  RETARDO <milisegundos>");
		puts("*\t 8.  ALGORITMO <algoritmo>: Cambia de algoritmo de insercion entre FIRST-FIT o WORST-FIT");
		puts("*\t 9.  DUMP ESTRUCTURAS");
		puts("*\t 10. DUMP ESTRUCTURAS PROGRAMA <pid>");
		puts("*\t 11. DUMP CONTENIDO_MEM_PPAL <offset> <tamanio>");
		puts("*\t 12. SALIR DEL PROGRAMA");
		puts("********************************");
		printf("Ingrese numero de operacion: ");
		scanf("%d",&option);
		while (getchar() != '\n');
		switch (option) {
			case 1: menuCompactar(); break;
			case 2: menuCrearSegmento(); break;
			case 3: menuBorrarSegmento();break;
			case 4: menuEscribirSegmento();break;
			case 5: menuLeerSegmento();break;
			case 6: menuDestruirPrograma();break;
			case 7: menuRetardo();break;
			case 8: menuAlgoritmo();break;
			case 9: dumpEstructuras();break; //Dump Estructuras
			case 10: menuDumpEstructurasPrograma();break;
			case 11: menuDumpContentMemPrincipal();break; //Dump Contenido Memoria Principal
			case 12: free(umv_dmr); exit(0); break;
		}
	}
}
