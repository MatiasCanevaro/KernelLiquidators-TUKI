#ifndef AUXILIARES_TRUNCAR_H_
#define AUXILIARES_TRUNCAR_H_

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
#include <fcb.h>
#include <filesystem.h>
#include <utils_fs.h>

void truncar_archivo(t_fcb* fcb,uint32_t nuevo_tamanio);
void liberar_bloques_excedentes(t_fcb* fcb,uint32_t bloques_actuales,uint32_t bloques_a_liberar);

void asignar_bloques(t_fcb* fcb, uint32_t bloques_actuales,uint32_t bloques_adicionales);


#endif