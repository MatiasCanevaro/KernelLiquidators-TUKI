#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include <init_filesystem.h>
#include <finish_filesystem.h>
#include <sys/mman.h>
#include <commons/string.h>
#include <auxiliares_truncar.h>
#include <utils_fs.h>

// LOGGER
extern t_log* logger;

// RECIBIDOS DE CONFIG
extern char* ip_memoria;
extern char* puerto_memoria;
extern char* puerto_escucha;
extern char* path_superbloque;
extern char* path_bitmap;
extern char* path_bloques;
extern char* path_fcb;
extern int retardo_acceso_bloque;

// SUPERBLOQUE
extern int block_size;
extern int block_count;

// BITMAP
extern char* bitmap;
extern t_bitarray* bitarray;

// ARCHIVO DE BLOQUES
extern char* archivo_bloques;

// ARCHIVOS ABIERTOS
extern t_dictionary* archivos_abiertos;

// SOCKETS
extern int socket_servidor;
extern int socket_memoria;
extern int socket_kernel;

t_list* recibir_parametros_fs();
char* ruta_archivo(char* nombre_archivo);

#endif