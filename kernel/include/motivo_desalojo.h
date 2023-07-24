#ifndef MOTIVO_DESALOJO
#define MOTIVO_DESALOJO
#include <stdlib.h>
#include <commons/collections/dictionary.h>
#include <utils_kernel.h>

void manejar_motivo_desalojo(t_pcb* pcb,t_list* motivo_desalojo);
t_list* recibir_motivo_desalojo();
t_list* recibir_pids(uint32_t cantidad);
t_list* recibir_lista_tablas(uint32_t cantidad_segmentos,uint32_t cant_tablas);
#endif