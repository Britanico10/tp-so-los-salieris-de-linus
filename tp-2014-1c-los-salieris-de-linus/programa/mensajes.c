#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "mensajes.h"

unsigned desempaquetar_32 (const void *buffer) {
  unsigned resultado = 0;
  unsigned char posicion;
  for (posicion = 0; posicion < 4; posicion ++)
    resultado |= ((unsigned) ((const unsigned char *) buffer)[posicion]) << (8 * posicion);
  return resultado;
}

void empaquetar_32 (unsigned numero, void *resultado) {
  unsigned char posicion;
  for (posicion = 0; posicion < 4; posicion ++) {
    ((char *) resultado)[posicion] = numero & 0xff;
    numero >>= 8;
  }
}

void *empaquetar_mensaje (mensaje_t mensaje) {
  char *resultado = malloc(12 + 4 * mensaje.cantidad + mensaje.tam_extra);
  if (!resultado) return NULL;
  empaquetar_32(mensaje.codigo, resultado);
  empaquetar_32(mensaje.cantidad, resultado + 4);
  empaquetar_32(mensaje.tam_extra, resultado + 8);
  unsigned numero_parametro;
  for (numero_parametro = 0; numero_parametro < mensaje.cantidad; numero_parametro ++)
    empaquetar_32(mensaje.parametros[numero_parametro], resultado + 12 + 4 * numero_parametro);
  memcpy(resultado + 12 + 4 * mensaje.cantidad, mensaje.extra, mensaje.tam_extra);
  return resultado;
}

mensaje_t desempaquetar_mensaje (const void *buffer) {
  const char *datos = buffer;
  mensaje_t resultado;
  resultado.codigo = desempaquetar_32(datos);
  resultado.cantidad = desempaquetar_32(datos + 4);
  resultado.tam_extra = desempaquetar_32(datos + 8);
  resultado.parametros = malloc(sizeof(unsigned) * resultado.cantidad);
  unsigned parametro;
  for (parametro = 0; parametro < resultado.cantidad; parametro ++)
    resultado.parametros[parametro] = desempaquetar_32(datos + 12 + 4 * parametro);
  resultado.extra = malloc(resultado.tam_extra);
  memcpy(resultado.extra, datos + 12 + 4 * resultado.cantidad, resultado.tam_extra);
  return resultado;
}

int enviar_mensaje (int numero_socket, mensaje_t mensaje) {
  void *datosBinarios = empaquetar_mensaje(mensaje);
  unsigned tamano = 12 + 4 * mensaje.cantidad + mensaje.tam_extra;
  char *envio = datosBinarios;
  int reintentos = 0;
  int t;
  while (tamano) {
    t = send(numero_socket, envio, tamano, MSG_NOSIGNAL);
    if (t < 0) {
      reintentos ++;
      if (reintentos > 3) {
        free(datosBinarios);
        return 0;
      }
      continue;
    }
    reintentos = 0;
    tamano -= t;
    envio += t;
  }
  free(datosBinarios);
  return 1;
}

int recibir_mensaje (int numero_socket, mensaje_t *mensaje) {
  char buffer_parametros[12];
  int estado = recibir_bytes(numero_socket, buffer_parametros, 12);
  if (estado <= 0) return estado;
  unsigned resto = 4 * desempaquetar_32(buffer_parametros + 4) + desempaquetar_32(buffer_parametros + 8);
  char *buffer_mensaje = malloc(12 + resto);
  memcpy(buffer_mensaje, buffer_parametros, 12);
  estado = recibir_bytes(numero_socket, buffer_mensaje + 12, resto);
  if (estado <= 0) {
    free(buffer_mensaje);
    return estado;
  }
  *mensaje = desempaquetar_mensaje(buffer_mensaje);
  free(buffer_mensaje);
  return 1;
}

int recibir_bytes (int numero_socket, void *buffer, unsigned cantidad) {
  char *datos = buffer;
  unsigned resto = cantidad;
  int reintentos = 0;
  int t;
  while (resto) {
    t = recv(numero_socket, datos, resto, MSG_WAITALL);
    if (!t) return -1; // negativo: la conexion se cerro
    if (t < 0) {
      reintentos ++;
      if (reintentos > 3) return 0;
      continue;
    }
    datos += t;
    resto -= t;
    reintentos = 0;
  }
  return 1;
}

void destruir_mensaje (mensaje_t *mensaje) {
  free(mensaje->parametros);
  free(mensaje->extra);
}