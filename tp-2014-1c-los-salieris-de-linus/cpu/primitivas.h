#ifndef PRIMITIVAS_H_
#define PRIMITIVAS_H_

#include <parser/parser.h>

t_puntero pr_definir_variable(t_nombre_variable);
t_puntero pr_puntero_variable(t_nombre_variable);
t_valor_variable pr_obtener_valor(t_puntero);
void pr_escribir_valor(t_puntero, t_valor_variable);
t_valor_variable pr_obtener_variable_compartida(t_nombre_compartida);
t_valor_variable pr_asignar_variable_compartida (t_nombre_compartida, t_valor_variable);
void pr_ir_a(t_nombre_etiqueta);
void pr_ejecutar_procedimiento(t_nombre_etiqueta);
void pr_ejecutar_funcion(t_nombre_etiqueta, t_puntero);
void pr_finalizar(void);
void pr_retornar(t_valor_variable);
void pr_imprimir_numero(t_valor_variable);
void pr_imprimir_texto(char *);
void pr_solicitar_IO(t_nombre_dispositivo, int);
void pr_wait(t_nombre_semaforo);
void pr_signal(t_nombre_semaforo);

#endif
