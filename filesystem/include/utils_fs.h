#ifndef UTILSFS
#define UTILSFS

#include <init_filesystem.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdio.h>
#include <filesystem.h>

void* obtener_bloque(uint32_t numero_bloque);
void desasignar_bloque(uint32_t bloque);
uint32_t encontrar_bloque_libre();

char* obtener_contenido_archivo(t_fcb* fcb,uint32_t posicion,uint32_t tamanio_a_leer,char* nombre_archivo);
void escribir_archivo_bloques(t_fcb* fcb,uint32_t posicion,uint32_t tamanio_a_escribir,char* contenido_a_escribir,char* nombre_archivo);

uint32_t obtener_bloque_siguiente(t_fcb* fcb,uint32_t num_bloque_actual);
void enviar_op_con_parametros(uint8_t motivo,t_list* lista_parametros,int socket_fd);

#endif