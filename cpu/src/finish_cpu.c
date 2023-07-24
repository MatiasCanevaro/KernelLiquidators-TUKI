#include <finish_cpu.h>

void finalizar_cpu() {

    log_info(logger,"CERRANDO CPU\n");
    
    liberar_conexion(socket_servidor);
    liberar_conexion(socket_kernel);
    liberar_conexion(socket_memoria);

    if(logger!=NULL) log_destroy(logger);
}