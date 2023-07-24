#ifndef INIT_CONSOLA_H_
#define INIT_CONSOLA_H_

#include <stdlib.h>
#include <stdio.h>
#include <logs.h>
#include <commons/config.h>
#include <sockets.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <tads/instruccion.h>

void iniciar_logger();
void leer_config();
int conectarse_a_kernel();

#endif