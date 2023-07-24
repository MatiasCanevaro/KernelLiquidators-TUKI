#ifndef HILO_CPU
#define HILO_CPU

#include <memoria.h>
#include <string.h>

void* esperar_cpu(void);
void recibir_parametros_cpu(t_list* parametros_cpu);
void leer_memoria_cpu(uint32_t df, uint32_t tamanio);
void escribir_memoria_cpu(uint32_t df, uint32_t tamanio, void* valor_a_escribir);

#endif