#ifndef KERNEL_H_
#define KERNEL_H_

#include <init_kernel.h>
#include <finish_kernel.h>
#include <hilo_consolas.h>
#include <hilo_agregar_ready.h>
#include <hilo_planificador_corto_plazo.h>
#include <hilo_io.h>
#include <hilo_fs.h>
#include <utils_kernel.h>
#include <commons/config.h>
#include <commons/collections/dictionary.h>

// logger y config
extern t_log* logger;
extern t_config* config;

// recibidos de config
extern char* ip_memoria;
extern char* puerto_memoria;
extern char* ip_fs;
extern char* puerto_fs;
extern char* ip_cpu;
extern char* puerto_cpu;
extern char* puerto_escucha;
extern char* algoritmo_planificacion;
extern int estimacion_inicial;
extern double hrrn_alpha;
extern int grado_max_multiprogramacion;
extern char** recursos;
extern char** instancias_recursos;

// sockets
extern int socket_servidor;
extern int socket_cpu;
extern int socket_memoria;
extern int socket_filesystem;

// variables auxiliares
extern int pid_actual;
extern t_list* lista_recursos_compartidos;
extern t_dictionary* diccionario_tds;

// estados
extern t_queue* cola_new;
extern t_queue* cola_ready;
extern t_queue* cola_fs;

// semaforos
extern sem_t sem_grado_multiprogramacion;
extern sem_t sem_cola_new_vacia;
extern sem_t sem_cola_ready_vacia;
extern sem_t sem_cola_fs;
extern sem_t sem_memoria_fs;

extern pthread_mutex_t mutex_cola_new;
extern pthread_mutex_t mutex_cola_ready;
extern pthread_mutex_t mutex_pid;
extern pthread_mutex_t mutex_lista_recursos;
extern pthread_mutex_t mutex_cola_fs;
extern pthread_mutex_t mutex_memoria;
extern pthread_mutex_t mutex_send_fs;
extern pthread_mutex_t mutex_diccionario_tds;
// tabla de archivos  Key->Nombrearchivo Value->Cola con los PIDS esperando el archivo
extern t_dictionary* tabla_archivos;


#endif