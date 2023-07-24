#ifndef UTILS_KERNEL
#define UTILS_KERNEL
#include <kernel.h>
typedef struct{
    uint32_t posicion;
    char* nombre;
}t_entrada_tabla_archivos;

void ingreso_a_ready();
void enviar_operacion_con_parametros(uint8_t motivo, t_list* lista_parametros,int socket_fd);
void enviar_operacion_a_filesystem(uint8_t motivo, t_list* lista_parametros);
void enviar_operacion_a_memoria(uint8_t motivo, t_list* lista_parametros);

#endif