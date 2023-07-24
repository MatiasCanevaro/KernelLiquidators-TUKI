#ifndef INIT_KERNEL_H_
#define INIT_KERNEL_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <logs.h>
#include <commons/config.h>
#include <sockets.h>
#include <semaphore.h>
#include <pthread.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <tads/pcb.h>
#include <commons/temporal.h>
#include <commons/string.h>
#include <tads/tabla_segmentos.h>

void iniciar_logger();
void leer_config(char* archivo_config);
void iniciar_colas_planificador();
void iniciar_semaforos();
void iniciar_servidor_kernel();
int conectarse_a_cpu();
int conectarse_a_memoria();
int conectarse_a_filesystem();
void iniciar_lista_recursos();

typedef struct {
    char* nombre;
    int cant_instancias;
    int cant_instancias_max;
    t_queue* cola_bloqueados_recurso;
    t_queue* cola_uso_recurso;
} t_recurso;

#endif