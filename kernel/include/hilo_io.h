#ifndef HILO_IO_H_
#define HILO_IO_H_

#include <kernel.h>
#include <tads/pcb.h>

typedef struct{
t_pcb* pcb;
int tiempo;
}t_param_io;


void* proceso_io(t_param_io* param_io);

#endif 