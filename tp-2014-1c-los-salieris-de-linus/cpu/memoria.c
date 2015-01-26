#include "cpu.h"

int leer_datos (unsigned base, unsigned desplazamiento, unsigned cantidad, void * buffer) {
  /*
    valores de retorno:
    -1: error de lectura
    0: segmentation fault
    valor positivo: cantidad de bytes leidos
  */
  if (!cantidad) return -1;
  mensaje_t mensaje;
  mensaje.codigo = LEER_SEGMENTO;
  mensaje.parametros = malloc(3 * sizeof(unsigned));
  mensaje.parametros[0] = base;
  mensaje.parametros[1] = desplazamiento;
  mensaje.parametros[2] = cantidad;
  mensaje.cantidad = 3;
  mensaje.tam_extra = 0;
  mensaje.extra = NULL;
  enviar_mensaje(socketUMV, mensaje);
  destruir_mensaje(&mensaje);
  int estado = recibir_mensaje(socketUMV, &mensaje);
  if (estado < 0) return -1;
  if (!estado) exit(1);
  switch (mensaje.codigo) {
    case SEGMENTATION_FAULT:
      destruir_mensaje(&mensaje);
      return 0;
    case DATOS_SEGMENTO:
      memcpy(buffer, mensaje.extra, mensaje.tam_extra);
      unsigned cantidad_leida = mensaje.tam_extra;
      destruir_mensaje(&mensaje);
      return cantidad_leida;
    default:
      destruir_mensaje(&mensaje);
      handshake(socketUMV, 0);
      return -1;
  }
}

int escribir_datos (unsigned base, unsigned desplazamiento, unsigned cantidad, void * datos) {
  mensaje_t mensaje;
  mensaje.codigo = ESCRIBIR_SEGMENTO;
  mensaje.cantidad = 2;
  mensaje.parametros = malloc(2 * sizeof(unsigned));
  mensaje.parametros[0] = base;
  mensaje.parametros[1] = desplazamiento;
  mensaje.tam_extra = cantidad;
  mensaje.extra = datos;
  enviar_mensaje(socketUMV, mensaje);
  int estado = recibir_mensaje(socketUMV, &mensaje);
  if (!estado) exit(1);
  if (estado < 0) return -1;
  unsigned codigo = mensaje.codigo;
  destruir_mensaje(&mensaje);
  switch (codigo) {
    case SEGMENTATION_FAULT:
      return 0;
    case ESCRIBIR_SEGMENTO_OK:
      return cantidad;
    default:
      handshake(socketUMV, 0);
      return -1;
  }
}

unsigned obtener_puntero_de_etiqueta (const char *etiqueta) {
  char *diccionario = malloc(PCB_actual.tam_indice_etiquetas);
  int estado = leer_datos(PCB_actual.indice_etiquetas, 0,PCB_actual.tam_indice_etiquetas, diccionario);
  if (estado <= 0) return -1;
  char * etiqueta_real = strchr(etiqueta, '\n') ? strndup(etiqueta, strchr(etiqueta, '\n') - etiqueta) : strdup(etiqueta);
  unsigned punteroActual = 0;
  punteroActual = metadata_buscar_etiqueta(etiqueta_real, diccionario, PCB_actual.tam_indice_etiquetas);
  free(etiqueta_real);
  free(diccionario);
  return punteroActual;
}