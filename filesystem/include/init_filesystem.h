#ifndef INIT_FILESYSTEM_H_
#define INIT_FILESYSTEM_H_

#include <stdlib.h>
#include <stdio.h>
#include <logs.h>
#include <commons/config.h>
#include <sockets.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <commons/bitarray.h>
#include <tads/instruccion.h>

void leer_config(char* archivo_config);
void leer_superbloque();

void iniciar_logger();
void iniciar_bitmap();
void iniciar_archivo_bloques();
void iniciar_servidor_filesystem();

int conectarse_a_memoria();
int conectarse_a_kernel();

#endif