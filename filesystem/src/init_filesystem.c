#include <init_filesystem.h>

// LOGGER
t_log* logger;

// RECIBIDOS DE CONFIG
char* ip_memoria;
char* puerto_memoria;
char* puerto_escucha;
char* path_superbloque;
char* path_bitmap;
char* path_bloques;
char* path_fcb;
int retardo_acceso_bloque;

// SUPERBLOQUE
int block_size;
int block_count;

// BITMAP
char* bitmap;
t_bitarray* bitarray;

// ARCHIVO DE BLOQUES
char* archivo_bloques;

// ARCHIVOS ABIERTOS
t_dictionary* archivos_abiertos;

// SOCKETS
int socket_servidor;
int socket_memoria;
int socket_kernel;

void leer_config(char* archivo_config){
    t_config* config = config_create(archivo_config);

    ip_memoria = config_get_string_value(config,"IP_MEMORIA");
    puerto_memoria = config_get_string_value(config,"PUERTO_MEMORIA");
    puerto_escucha = config_get_string_value(config,"PUERTO_ESCUCHA"); 
    path_superbloque = config_get_string_value(config,"PATH_SUPERBLOQUE"); 
    path_bitmap = config_get_string_value(config,"PATH_BITMAP"); 
    path_bloques = config_get_string_value(config,"PATH_BLOQUES"); 
    path_fcb = config_get_string_value(config,"PATH_FCB"); 
    retardo_acceso_bloque = config_get_int_value(config,"RETARDO_ACCESO_BLOQUE");  
}

void leer_superbloque(){
    t_config* superbloque = config_create(path_superbloque);

    block_size = config_get_int_value(superbloque,"BLOCK_SIZE");
    block_count = config_get_int_value(superbloque,"BLOCK_COUNT");
}

void iniciar_logger(){
    logger = crear_logger("cfg/filesystem.log","log_filesystem");
    log_info(logger,"INICIO FILESYSTEM");
}

void iniciar_bitmap(){
    int archivo_bitmap;
    int existe=0;

    archivo_bitmap = open(path_bitmap,O_RDWR);
    if(archivo_bitmap == -1){

        archivo_bitmap = open(path_bitmap,O_RDWR|O_CREAT,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
        ftruncate(archivo_bitmap,block_count/8);
        existe++;
    }
    bitmap = mmap(NULL,block_count/8,PROT_READ|PROT_WRITE,MAP_SHARED,archivo_bitmap,0);

    if(bitmap == NULL) log_error(logger,"ERROR AL MAPEAR BITMAP");

    if(existe){
    memset(bitmap,0,block_count/8);
    msync(bitmap,block_count/8,MS_SYNC);
    }
    
    bitarray = bitarray_create_with_mode(bitmap,block_count/8,LSB_FIRST);
}

void iniciar_archivo_bloques(){
    int archivo_bloques_fd;

    archivo_bloques_fd = open(path_bloques,O_RDWR);

    if(archivo_bloques_fd == -1){
        archivo_bloques_fd = open(path_bloques,O_RDWR|O_CREAT,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
    }

    // ASIGNO EL TAMAÑO DEL ARCHIVO DE BLOQUES
    ftruncate(archivo_bloques_fd,block_size*block_count);
    archivo_bloques = mmap(NULL,block_size*block_count,PROT_READ|PROT_WRITE,MAP_SHARED,archivo_bloques_fd,0);
}

void iniciar_servidor_filesystem(){
    socket_servidor = iniciar_servidor(puerto_escucha);
    log_info(logger,"Servidor iniciado en el puerto %s",puerto_escucha);
}

int conectarse_a_memoria(){
    socket_memoria = crear_conexion(ip_memoria,puerto_memoria);
    
    // HANDSHAKE
    int hs = handshake_cliente(socket_memoria);
        if(hs<0){
            log_error(logger,"Resultado del handshake incorrecto");
            return 0;
        }

    log_info(logger,"Filesystem conectado con la Memoria");

    return 1;
}

int conectarse_a_kernel(){
    socket_kernel = esperar_cliente(socket_servidor);

    if(socket_kernel<0){
        log_error(logger,"Error al conectarse al kernel");
        return 0;
    }
    // HANDSHAKE
    int hs = handshake_servidor(socket_kernel);
        if(hs<0){
            log_error(logger,"Resultado del handshake incorrecto");
            return 0;
        } 

    log_info(logger,"Se conecto el Kernel");

    return 1;
}