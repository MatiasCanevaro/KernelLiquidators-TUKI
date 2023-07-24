#include <init_consola.h>

// LOGGER
t_log* logger;
// CONFIG
t_config* config;

// RECIBIDOS DE CONFIG
char* ip_kernel;
char* puerto_kernel;

// SOCKETS
int socket_kernel;

void iniciar_logger(){
    logger = crear_logger("cfg/consola.log","log_consola");
    log_info(logger, "INICIO CONSOLA");
}

void leer_config(){
    config = config_create("cfg/consola.config");

    ip_kernel = config_get_string_value(config, "IP_KERNEL");
    puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL"); 

    //config_destroy(config); //TODO rompe
}

int conectarse_a_kernel() {
    socket_kernel = crear_conexion(ip_kernel, puerto_kernel);
    
    // HANDSHAKE
    int hs = handshake_cliente(socket_kernel);
        if(hs<0){
            log_error(logger,"Resultado del handshake incorrecto");
            return 0;
        }

    log_info(logger, "Consola conectada con el Kernel");

    return 1;
}