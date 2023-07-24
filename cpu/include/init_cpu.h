#ifndef INIT_CPU_H_
#define INIT_CPU_H_
#include <cpu.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <logs.h>
#include <commons/config.h>
#include <sockets.h>
#include <tads/pcb.h>
#include <tads/instruccion.h>
#include <unistd.h>

void iniciar_logger();
void leer_config(char* archivo_config);
void iniciar_servidor_cpu();
int conectarse_a_memoria();
int conectarse_a_kernel();

#endif