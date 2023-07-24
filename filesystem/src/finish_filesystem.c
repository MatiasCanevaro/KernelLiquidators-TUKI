#include <finish_filesystem.h>

void finalizar_filesystem(){

    log_info(logger,"CERRANDO FILESYSTEM");

    liberar_conexion(socket_servidor);
    liberar_conexion(socket_memoria);

    if(logger!=NULL) log_destroy(logger);
}