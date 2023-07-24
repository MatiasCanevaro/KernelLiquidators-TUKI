#ifndef INSTRUCCION_H_
#define INSTRUCCION_H_

#include <stdlib.h>
#include <commons/collections/list.h>
#include <logs.h>
#include <paquete.h>

// PSEUDO INSTRUCCIONES
#define CREAR_PROCESO 19
#define ABRIR_ARCHIVO 20
#define CREAR_ARCHIVO 21
#define TRUNCAR_ARCHIVO 22
#define ELIMINAR_PROCESO 23
#define COMPACTAR 26
#define HAY_ESPACIO_TOTAL_NO_CONTIGUO 30
#define NO_HAY_ESPACIO_SUFICIENTE 31


typedef struct 
{
    uint32_t codigo;
    t_list* parametros;
} t_instruccion;

typedef enum{
SET,
MOV_IN,
MOV_OUT,
IO,
F_OPEN,
F_CLOSE,
F_SEEK,
F_READ,
F_WRITE,
F_TRUNCATE,
WAIT,
SIGNAL,
CREATE_SEGMENT,
DELETE_SEGMENT,
YIELD,
EXIT,
SEG_FAULT,
I_NORECONOCIDA
}cod_instruccion;

uint32_t convertir_instruccion_a_entero(char* instruccion);

t_instruccion* crear_instruccion();

void destruir_instruccion(t_instruccion* instruccion);

void destruir_lista_instrucciones(t_list* lista_instrucciones);

t_list* deserializar_instrucciones(void* stream);

t_buffer* serializar_instrucciones(t_list* lista_instrucciones);

t_list* recibir_paquete_instrucciones(int socket_origen);

void enviar_paquete_instrucciones(int socket_destino, t_list* instrucciones);

void imprimir_instrucciones(t_list* lista);

#endif