#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <init_memoria.h>
#include <finish_memoria.h>
#include <hilo_kernel.h>
#include <hilo_fs.h>
#include <hilo_cpu.h>
#include <commons/config.h>
#include <commons/collections/dictionary.h>
#include <string.h>


// logger
extern t_log* logger;

// RECIBIDOS DE CONFIG
extern char* ip_kernel;
extern char* puerto_kernel;
extern char* ip_filesystem;
extern char* puerto_filesystem;
extern char* ip_cpu;
extern char* puerto_cpu;
extern char* puerto_escucha;
extern int tam_memoria;
extern int tam_segmento_cero;
extern int cant_segmentos;
extern int retardo_memoria;
extern int retardo_compactacion;
extern char* algoritmo_asignacion;

// config
extern t_config* config;

// VARIABLES AUXILIARES
extern void* memoria;
extern t_entrada_tabla_segmentos* segmento_cero;
extern t_list* lista_huecos_libres;
extern t_dictionary* tabla_general;

// sockets
extern int socket_servidor;
extern int socket_cpu;
extern int socket_kernel;
extern int socket_filesystem;

// semaforos
extern pthread_mutex_t mutex_memoria;

#endif