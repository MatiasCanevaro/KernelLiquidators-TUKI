#ifndef INIT_MEMORIA_H_
#define INIT_MEMORIA_H_

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
#include <commons/collections/dictionary.h>

void iniciar_logger();
void leer_config(char* archivo_config);
void iniciar_servidor_memoria();
int conectarse_a_cpu();
int conectarse_a_kernel();
int conectarse_a_filesystem();
void iniciar_espacio_de_usuario();
void iniciar_segmento_cero();



#endif