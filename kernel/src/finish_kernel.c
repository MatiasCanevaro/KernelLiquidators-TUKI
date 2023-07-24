#include <finish_kernel.h>

void finalizar_kernel() {

    log_info(logger,"CERRANDO KERNEL");
    
    liberar_conexion(socket_servidor);
    liberar_conexion(socket_cpu);
    liberar_conexion(socket_memoria);
    liberar_conexion(socket_filesystem);

    if(logger!=NULL) log_destroy(logger);
}