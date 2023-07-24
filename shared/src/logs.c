#include <logs.h>

t_log* crear_logger(char* direccion, char* nombre){

    t_log* logger = log_create(direccion,nombre,1,LOG_LEVEL_INFO);

    if(logger==NULL){
        printf("No se pudo crear el logger");
        exit(1);
    }

    return logger;
}