typedef struct mensaje {
  unsigned codigo;
  unsigned cantidad;
  unsigned tam_extra;
  unsigned *parametros;
  char *extra;
} mensaje_t;

int enviar_mensaje(int, mensaje_t);
int recibir_mensaje(int, mensaje_t *);
void destruir_mensaje(mensaje_t *);
