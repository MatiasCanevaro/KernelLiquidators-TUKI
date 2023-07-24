#include <finish_memoria.h>

void finalizar_memoria() {

    log_info(logger,"CERRANDO MEMORIA");
    
    liberar_conexion(socket_servidor);
    liberar_conexion(socket_cpu);
    liberar_conexion(socket_kernel);
    liberar_conexion(socket_filesystem);

    if(logger!=NULL) log_destroy(logger);
}