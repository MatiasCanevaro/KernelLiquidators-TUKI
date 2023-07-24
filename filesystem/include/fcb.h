#ifndef FCB_H
#define FCB_H

#include <stdint.h>
#include <commons/config.h>

typedef struct
{
    char* nombre_archivo;
    uint32_t tamanio_archivo;
    uint32_t puntero_directo;
    int puntero_indirecto;
    t_config* config;
}t_fcb;

#endif