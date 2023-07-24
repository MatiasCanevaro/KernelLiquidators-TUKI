#include <init_cpu.h>

//CONFIG
t_config* config;
// LOGGER
t_log* logger;

// RECIBIDOS DE CONFIG
int retardo_instruccion;
char* ip_memoria;
char* puerto_memoria;
char* puerto_escucha;
int tam_max_segmento;

// SOCKETS
int socket_servidor;
int socket_memoria;
int socket_kernel;

// ESTRUCTURAS
t_registros_cpu* registros_cpu;

void iniciar_logger() {
    logger = crear_logger("cfg/cpu.log", "log_cpu");
    log_info(logger, "INICIO CPU");
}

void leer_config(char* archivo_config) {
    t_config* config_ips = config_create("cfg/ips.config");
    ip_memoria = config_get_string_value(config_ips, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config_ips, "PUERTO_MEMORIA");
    puerto_escucha = config_get_string_value(config_ips, "PUERTO_ESCUCHA");

    config = config_create(archivo_config);

    retardo_instruccion = config_get_int_value(config, "RETARDO_INSTRUCCION");
    tam_max_segmento = config_get_int_value(config, "TAM_MAX_SEGMENTO");

    //config_destroy(config); //TODO rompe
}

void iniciar_servidor_cpu() {
    socket_servidor = iniciar_servidor(puerto_escucha);
    log_info(logger, "Servidor iniciado en el puerto %s", puerto_escucha);
}

int conectarse_a_memoria() {
    socket_memoria = crear_conexion(ip_memoria, puerto_memoria);
    
    // HANDSHAKE
    int hs = handshake_cliente(socket_memoria);
        if(hs<0){
            log_error(logger,"Resultado del handshake incorrecto");
            return 0;
        }

    log_info(logger, "CPU conectada con la Memoria");

    return 1;
}

int conectarse_a_kernel() {
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