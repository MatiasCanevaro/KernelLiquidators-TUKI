#ifndef PCB_H_
#define PCB_H_

#include <stdint.h>
#include <stdlib.h>
#include <tads/instruccion.h>
#include <commons/collections/list.h>
#include <commons/temporal.h>
#include <tads/registros_cpu.h>
#include <tads/tabla_segmentos.h>
#include <tads/archivos_abiertos.h>

typedef struct
{
    uint32_t PID;
    uint32_t socket_consola;
    uint32_t program_counter;
    t_registros_cpu* registros_cpu;
    t_list* instrucciones;
    t_list* tabla_segmentos;
    uint32_t est_proxima_rafaga; //en milisegundos
    t_temporal* tiempo_llegada_ready; 
    t_list* archivos_abiertos; //puede cambiar
}t_pcb;

typedef struct {
    uint32_t PID; 
    uint32_t program_counter;
    t_list* instrucciones;
    t_registros_cpu* registros_cpu;
    t_list* tabla_segmentos;
}t_pcb_cpu;

typedef struct {
    uint32_t program_counter;
    t_registros_cpu* registros_cpu;
}t_pcb_kernel;


void destruir_pcb(t_pcb* pcb);
t_buffer* serializar_pcb_cpu(t_pcb* pcb);
t_pcb_cpu* deserializar_pcb_cpu(void* stream);
t_buffer* serializar_pcb_kernel(t_pcb_cpu* pcb);
t_pcb_kernel* deserializar_pcb_kernel(void* stream);
int enviar_paquete_pcb(t_buffer* buffer_pcb, int socket_destino);
t_pcb_cpu* recibir_paquete_pcb_cpu(int socket_origen);
t_pcb_kernel* recibir_paquete_pcb_kernel(int socket_origen);

/*
//MOCKS
t_buffer* serializar_archivos_abiertos(t_list* archivos);
t_list* deserializar_archivos_abiertos(void* stream);
*/
#endif

/*
-- PREGUNTAR Tabla de Segmentos: Contendrá ids, direcciones base y tamaños de los segmentos de datos del proceso.
*/