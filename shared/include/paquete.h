#ifndef PAQUETE_H_
#define PAQUETE_H_

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

typedef struct {
    uint32_t size; // Tamaño del payload
    void* stream; // Payload
} t_buffer;

typedef struct {
    uint8_t codigo_operacion;
    t_buffer* buffer;
} t_paquete;


typedef enum{
    MENSAJE,
    INSTRUCCIONES,
    PCB,
    MOTIVO_DESALOJO
} codigo_operacion;




void crear_buffer(t_paquete* paquete);

t_paquete* crear_paquete(codigo_operacion op_code);

t_paquete* recibir_paquete(int socket_cliente);

void* serializar_paquete(t_paquete* paquete);

int enviar_paquete(int socket_fd, t_paquete* paquete);

void eliminar_paquete(t_paquete* paquete);


#endif 