#ifndef ARCHIVOS_ABIERTOS_H
#define ARCHIVOS_ABIERTOS_H
#include <commons/collections/list.h>
#include <stdint.h>
#include <stdlib.h>
#include <paquete.h>

void destruir_lista_archivos_abiertos(t_list* archivos_abiertos);
t_list* deserializar_archivos_abiertos(void* stream);
t_buffer* serializar_archivos_abiertos(t_list* archivos_abiertos);



#endif