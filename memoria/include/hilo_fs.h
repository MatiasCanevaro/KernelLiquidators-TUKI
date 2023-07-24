#ifndef HILO_FS
#define HILO_FS

#include <memoria.h>

void* esperar_fs(void);
void recibir_parametros_fs(t_list* parametros_fs);
void leer_memoria_fs(uint32_t df, uint32_t tamanio);
void escribir_memoria_fs(uint32_t df, uint32_t tamanio, void* valor_a_escribir);

#endif