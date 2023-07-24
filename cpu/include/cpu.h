#ifndef CPU_H_
#define CPU_H_

#include <init_cpu.h>
#include <finish_cpu.h>
#include <ciclo_instruccion.h>
#include <commons/config.h>
#include <commons/string.h>
#include <math.h>

// logger
extern t_log* logger;

// recibidos de config
extern int retardo_instruccion;
extern char* ip_memoria;
extern char* puerto_memoria;
extern char* puerto_escucha;
extern int tam_max_segmento;

// config
extern t_config* config;
// sockets
extern int socket_servidor;
extern int socket_memoria;
extern int socket_kernel;

// estructuras
extern t_registros_cpu* registros_cpu;

#endif