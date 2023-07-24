#include <finish_consola.h>

void finalizar_consola() {

    log_info(logger,"CERRANDO CONSOLA\n");

    liberar_conexion(socket_kernel);

    if(logger!=NULL) log_destroy(logger);
}