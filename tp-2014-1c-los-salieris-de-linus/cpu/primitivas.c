#include "primitivas.h"
#include "cpu.h"
#include <parser/parser.h>

t_puntero pr_definir_variable (t_nombre_variable variable) {
  if (estado_programa) return -1;
  if (strchr(nombresDiccionario, variable)) {
    // la variable ya existe, error de redefinicion
    estado_programa = 4;
    return -1;
  }
  unsigned base = PCB_actual.pointer_stack;
  unsigned desplazamiento = 5 * PCB_actual.tam_contexto_actual;
  char datos[5];
  *datos = variable;
  int estado = escribir_datos(base, desplazamiento, 5, datos);
  if (estado < 0) {
    estado_programa = 5;
    return -1;
  } else if (!estado) {
    estado_programa = 3;
    return -1;
  }
  PCB_actual.tam_contexto_actual ++;
  nombresDiccionario = realloc(nombresDiccionario, 1 + PCB_actual.tam_contexto_actual);
  nombresDiccionario[PCB_actual.tam_contexto_actual - 1] = variable;
  nombresDiccionario[PCB_actual.tam_contexto_actual] = 0;
  punterosDiccionario = realloc(punterosDiccionario, sizeof(unsigned) * PCB_actual.tam_contexto_actual);
  punterosDiccionario[PCB_actual.tam_contexto_actual - 1] = (base+desplazamiento) + 1;
  // + 1 porque el primer byte almacena el nombre de la variable
  return (base+desplazamiento) + 1;
}

t_puntero pr_puntero_variable (t_nombre_variable variable) {
  if (estado_programa) return -1;
  char * posicion = strchr(nombresDiccionario, variable);
  if (!posicion) return -1;
  return punterosDiccionario[posicion - nombresDiccionario];
}

t_valor_variable pr_obtener_valor (t_puntero puntero) {
  if (estado_programa >= 2) return 0;
  char datos_leidos[4];
  int estado = leer_datos(puntero, 0, 4, datos_leidos);
  if (estado < 0) {
    estado_programa = 5;
    return 0;
  } else if (!estado) {
    estado_programa = 2;
    return 0;
  }
  return desempaquetar_32(datos_leidos);
}

void pr_escribir_valor (t_puntero puntero, t_valor_variable valor) {
  if (estado_programa) return;
  char datos_escritura[4];
  empaquetar_32(valor, datos_escritura);
  int estado = escribir_datos(puntero, 0, 4, datos_escritura); //cambiar! falta desplazamiento!
  if (estado <= 0) estado_programa = estado ? 5 : 2;
}

t_valor_variable pr_obtener_variable_compartida (t_nombre_compartida nombre_variable) {
  if (estado_programa) return 0;
  mensaje_t mensaje;
  mensaje.codigo = SYS_OBTENER_VALOR;
  mensaje.cantidad = 0;
  mensaje.parametros = NULL;
  mensaje.tam_extra = strlen(nombre_variable) + 1;
  mensaje.extra = nombre_variable;
  enviar_mensaje(socketKernel, mensaje);
  int estado = recibir_mensaje(socketKernel, &mensaje);
  if (!estado) exit(1);
  if (estado < 0) {
    estado_programa = 5;
    handshake(socketKernel, 0);
    return 0;
  }
  unsigned valor = 0;
  switch (mensaje.codigo) {
    case SYS_VALOR_OBTENIDO:
      if ((mensaje.cantidad != 1) || mensaje.tam_extra) {
        estado_programa = 5;
        break;
      }
      valor = mensaje.parametros[0];
      break;
    case VARIABLE_NO_EXISTE:
      estado_programa = 6;
      break;
    default:
      estado_programa = 5;
  }
  destruir_mensaje(&mensaje);
  return valor;
}

t_valor_variable pr_asignar_variable_compartida (t_nombre_compartida variable, t_valor_variable valor) {
  if (estado_programa) return 0;
  unsigned valor_nuevo = valor;
  mensaje_t mensaje;
  mensaje.codigo = SYS_ASIGNAR_VALOR;
  mensaje.cantidad = 1;
  mensaje.parametros = &valor_nuevo;
  mensaje.tam_extra = strlen(variable) + 1;
  mensaje.extra = variable;
  enviar_mensaje(socketKernel, mensaje);
  int estado = recibir_mensaje(socketKernel, &mensaje);
  if (!estado) exit(1);
  if (estado < 0) {
    estado_programa = 5;
    handshake(socketKernel, 0);
    return 0;
  }
  unsigned codigo = mensaje.codigo;
  destruir_mensaje(&mensaje);
  switch (codigo) {
    case SYS_ASIGNAR_OK:
      return valor_nuevo;
    case VARIABLE_NO_EXISTE:
      estado_programa = 6;
      return 0;
    default:
      estado_programa = 5;
      handshake(socketKernel, 0);
      return 0;
  }
}

void pr_ir_a (t_nombre_etiqueta etiqueta) {
  if (estado_programa) return;
  unsigned puntero = obtener_puntero_de_etiqueta(etiqueta);
  if (puntero == ((unsigned) -1))
    estado_programa = 7;
  else
    PCB_actual.pc = puntero;
}

void pr_ejecutar_procedimiento (t_nombre_etiqueta procedimiento) {
  if (estado_programa) return;
  unsigned puntero = obtener_puntero_de_etiqueta(procedimiento);
  if (puntero == ((unsigned) -1)) {
    estado_programa = 7;
    return;
  }
  unsigned sp = PCB_actual.pointer_stack + 5 * PCB_actual.tam_contexto_actual;
  int estado;
  estado = push(&sp, PCB_actual.pointer_stack);
  if (estado <= 0) {
    estado_programa = 3;
    return;
  }
  estado = push(&sp, PCB_actual.pc);
  if (estado <= 0) {
    estado_programa = 3;
    return;
  }
  destruirDiccionario();
  PCB_actual.pointer_stack = sp;
  PCB_actual.tam_contexto_actual = 0;
  construirDiccionario(PCB_actual);
  PCB_actual.pc = puntero;
}

void pr_ejecutar_funcion (t_nombre_etiqueta funcion, t_puntero valor_retorno) {
  if (estado_programa) return;
  unsigned puntero = obtener_puntero_de_etiqueta(funcion);
  if (puntero == ((unsigned) -1)) {
    estado_programa = 7;
    return;
  }
  unsigned sp = PCB_actual.pointer_stack + 5 * PCB_actual.tam_contexto_actual;
  int estado;
  estado = push(&sp, PCB_actual.pointer_stack);
  if (estado <= 0) {
    estado_programa = 3;
    return;
  }
  estado = push(&sp, PCB_actual.pc);
  if (estado <= 0) {
    estado_programa = 3;
    return;
  }
  estado = push(&sp, valor_retorno);
  if (estado <= 0) {
    estado_programa = 3;
    return;
  }
  destruirDiccionario();
  PCB_actual.pointer_stack = sp;
  PCB_actual.tam_contexto_actual = 0;
  construirDiccionario(PCB_actual);
  PCB_actual.pc = puntero;
}

// corregir finalizar y retornar, retornan un puntero (o -1 al finalizar el programa)

void pr_finalizar (void) {
  if (estado_programa) return;
  if (PCB_actual.pointer_stack == PCB_actual.segmento_stack) {
    estado_programa = 1;
    return;
  }
  int estado;
  unsigned sp = PCB_actual.pointer_stack;
  unsigned pc_retorno;
  estado = pop(&sp, &pc_retorno);
  if (estado <= 0) {
    estado_programa = 8;
    return;
  }
  unsigned contexto_anterior;
  estado = pop(&sp, &contexto_anterior);
  if (estado <= 0) {
    estado_programa = 8;
    return;
  }
  destruirDiccionario();
  PCB_actual.pointer_stack = contexto_anterior;
  PCB_actual.tam_contexto_actual = (sp - contexto_anterior) / 5;
  PCB_actual.pc = pc_retorno;
  construirDiccionario(PCB_actual);
}

void pr_retornar (t_valor_variable valor) {
  if (estado_programa) return;
  if (PCB_actual.pointer_stack == PCB_actual.segmento_stack) {
    estado_programa = 8;
    return;
  }
  unsigned puntero_retorno;
  int estado;
  unsigned sp = PCB_actual.pointer_stack;
  estado = pop(&sp, &puntero_retorno);
  if (estado <= 0) {
    estado_programa = 8;
    return;
  }
  pr_escribir_valor(puntero_retorno, valor);
  if (estado_programa) return;
  unsigned pc_retorno;
  estado = pop(&sp, &pc_retorno);
  if (estado <= 0) {
    estado_programa = 8;
    return;
  }
  unsigned contexto_anterior;
  estado = pop(&sp, &contexto_anterior);
  if (estado <= 0) {
    estado_programa = 8;
    return;
  }
  destruirDiccionario();
  PCB_actual.pointer_stack = contexto_anterior;
  PCB_actual.tam_contexto_actual = (sp - contexto_anterior) / 5;
  PCB_actual.pc = pc_retorno;
  construirDiccionario(PCB_actual);
}

void pr_imprimir_numero (t_valor_variable numero) {
  if (estado_programa) return;
  unsigned num = numero; 
  enviar_mensaje(socketKernel, (mensaje_t) {
    .codigo = SYS_IMPR,
    .tam_extra = 0,
    .extra = NULL,
    .cantidad = 1,
    .parametros = &num
  });
}

void pr_imprimir_texto (char * texto) {
  if (estado_programa) return;
  enviar_mensaje(socketKernel, (mensaje_t) {
    .codigo = SYS_IMPR_TEXTO,
    .cantidad = 0,
    .parametros = NULL,
    .tam_extra = strlen(texto) + 1,
    .extra = texto
  });
}

void pr_solicitar_IO (t_nombre_dispositivo dispositivo, int tiempo) {
  if (estado_programa) return;
  unsigned cantidad_tiempo = tiempo;
  enviar_mensaje(socketKernel, (mensaje_t) {
    .codigo = SYS_IO,
    .cantidad = 1,
    .parametros = &cantidad_tiempo,
    .tam_extra = strlen(dispositivo) + 1,
    .extra = dispositivo
  });
}

void pr_wait (t_nombre_semaforo semaforo) {
  if (estado_programa) return;
  enviar_mensaje(socketKernel, (mensaje_t) {
    .codigo = SYS_WAIT,
    .cantidad = 0,
    .parametros = NULL,
    .tam_extra = strlen(semaforo) + 1,
    .extra = semaforo
  });
}

void pr_signal (t_nombre_semaforo semaforo) {
  if (estado_programa) return;
  enviar_mensaje(socketKernel, (mensaje_t) {
    .codigo = SYS_SIGNAL,
    .cantidad = 0,
    .parametros = NULL,
    .tam_extra = strlen(semaforo) + 1,
    .extra = semaforo
  });
}


int push (unsigned * ptr, unsigned val) {
  char buffer[4];
  empaquetar_32(val, buffer);
  int estado = escribir_datos(*ptr, 0, 4, buffer); //ojo con el desplazamiento!
  *ptr += 4;
  return estado;
}

int pop (unsigned * ptr, unsigned * val) {
  char buffer[4];
  *ptr -= 4;
  int estado = leer_datos(*ptr, 0, 4, buffer);
  if (estado < 0) return estado;
  *val = desempaquetar_32(buffer);
  return estado;
}
