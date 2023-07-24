#ifndef HILO_CONSOLAS_H
#define HILO_CONSOLAS_H

#include <kernel.h> 

void* esperar_consolas();
t_pcb* crear_pcb(t_list* instruccionesRecibidas,uint32_t socket_consola);
t_list* solcitar_tds_inicial_a_memoria(uint32_t pid);
#endif