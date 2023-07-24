#ifndef TABLA_SEGMENTOS_H
#define TABLA_SEGMENTOS_H

#include <commons/collections/list.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <paquete.h>

typedef struct 
{
    uint32_t pid;
    uint32_t sid;
    uint32_t base;
    uint32_t tamanio;
} t_entrada_tabla_segmentos;

typedef struct 
{
    uint32_t base;
    uint32_t tamanio;
} t_hueco_libre;

void destruir_lista_tabla_segmentos(t_list* tabla_segmentos);

t_buffer* serializar_tabla_segmentos(t_list* tabla_segmentos);

t_list* deserializar_tabla_segmentos(void* stream);

#endif