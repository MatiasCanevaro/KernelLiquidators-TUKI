#include <filesystem.h>

int main(int argc,char* argv[]){
    
    // INIT 
    iniciar_logger();
    leer_config(argv[1]);
    leer_superbloque();
    iniciar_bitmap();
    iniciar_archivo_bloques();
    archivos_abiertos = dictionary_create();

    // CONEXIONES
    iniciar_servidor_filesystem();

    // MEMORIA
    int conexion_ok = conectarse_a_memoria();
    if(!conexion_ok){
        finalizar_filesystem();
        return 1;
    }

    // KERNEL
    conexion_ok = conectarse_a_kernel();
    if(!conexion_ok){
        finalizar_filesystem();
        return 1;
    }
    
    // RECIBO OPERACION DEL KERNEL
    while (1) {
        uint8_t codigo_operacion;
        recv(socket_kernel,&codigo_operacion,sizeof(uint8_t),MSG_WAITALL);

        t_config* archivo_fcb;

        t_list* parametros = recibir_parametros_fs();
        char* nombre_archivo = list_get(parametros,0);

        // RUTA DEL ARCHIVO

        switch(codigo_operacion) {
            case ABRIR_ARCHIVO:
                archivo_fcb = config_create(ruta_archivo(nombre_archivo)); // SI NO EXISTE RETORNA NULL

                if(!archivo_fcb){
                    uint32_t cod_not_found = 404;
                    send(socket_kernel,&cod_not_found,sizeof(uint32_t),0);
                    
                } else{
                    log_info(logger,"Abrir Archivo: %s",nombre_archivo); 

                    uint32_t cod_found = 200;
                    send(socket_kernel,&cod_found,sizeof(uint32_t),0);
                       
                    t_fcb* fcb = malloc(sizeof(t_fcb));

                    fcb->nombre_archivo = nombre_archivo;
                    fcb->config = archivo_fcb;
                    fcb->tamanio_archivo = (uint32_t)config_get_int_value(archivo_fcb,"TAMANIO");
                    fcb->puntero_directo = (uint32_t)config_get_int_value(archivo_fcb,"PUNTERO_DIRECTO");

                    if(config_has_property(archivo_fcb,"PUNTERO_INDIRECTO")){
                        fcb->puntero_indirecto = config_get_int_value(archivo_fcb,"PUNTERO_INDIRECTO");
                    }else{
                            fcb->puntero_indirecto = -1;
                    }
                    
                    dictionary_put(archivos_abiertos,nombre_archivo,fcb);
                }
                break;

            case CREAR_ARCHIVO:
                log_info(logger,"Crear Archivo: %s", nombre_archivo);

                uint32_t puntero_directo_index = encontrar_bloque_libre();

                msync(bitmap,block_count/8,MS_SYNC);

                archivo_fcb = malloc(sizeof(t_config));
                archivo_fcb->properties = dictionary_create();
                archivo_fcb->path=string_duplicate(ruta_archivo(nombre_archivo));

                config_set_value(archivo_fcb,"NOMBRE_ARCHIVO",nombre_archivo);
                config_set_value(archivo_fcb,"TAMANIO", string_itoa(0));
                config_set_value(archivo_fcb,"PUNTERO_DIRECTO",string_itoa(puntero_directo_index));
                //config_set_value(archivo_fcb,"PUNTERO_INDIRECTO",string_itoa(0));
                config_save_in_file(archivo_fcb,ruta_archivo(nombre_archivo));

                // AGREGAMOS A LOS ARCHIVOS FALTANTES
                t_fcb* fcb = malloc(sizeof(t_fcb));
                
                fcb->nombre_archivo = nombre_archivo;
                fcb->config = archivo_fcb;
                fcb->tamanio_archivo = 0;
                fcb->puntero_directo = puntero_directo_index;
                fcb->puntero_indirecto = -1;

                dictionary_put(archivos_abiertos,nombre_archivo,fcb);

                uint32_t cod_creacion_ok = 200;
                send(socket_kernel,&cod_creacion_ok,sizeof(uint32_t),0);

                break;

            case F_TRUNCATE:
                uint32_t nuevo_tamanio = atoi(list_get(parametros,1));

                log_info(logger,"Truncar Archivo: %s - Tamaño: %d",nombre_archivo,nuevo_tamanio);

                t_fcb* fcb_truncar = dictionary_get(archivos_abiertos,nombre_archivo);

                if (fcb_truncar == NULL) {
                    uint32_t cod_not_found = 404;
                    send(socket_kernel,&cod_not_found,sizeof(uint32_t),0);
                } else {
                    uint32_t cod_found = 200;
                    
                    truncar_archivo(fcb_truncar,nuevo_tamanio);

                    send(socket_kernel,&cod_found,sizeof(uint32_t),0);
                }
                break;

            case F_READ:
                char* tamanio_a_leer = list_get(parametros,1);
                char* direccion_leer = list_get(parametros,2);
                char* posicion_leer  = list_get(parametros,3);
                char* pid_fread      = list_get(parametros,4);

                log_info(logger,"Leer Archivo: %s - Puntero: %s - Memoria: %s - Tamaño: %s ",nombre_archivo,posicion_leer,direccion_leer,tamanio_a_leer);

                t_fcb* fcb_leer = dictionary_get(archivos_abiertos,nombre_archivo);

                if (fcb_leer == NULL){
                    uint32_t cod_not_found = 404;
                    send(socket_kernel,&cod_not_found,sizeof(uint32_t),0);
                } else{
                    uint32_t cod_found = 200;
                    uint32_t resultado;

                    char* contenido_read = obtener_contenido_archivo(fcb_leer,(uint32_t) atoi(posicion_leer),(uint32_t) atoi(tamanio_a_leer),nombre_archivo);

                    t_list* parametros_fread = list_create();
                    list_add(parametros_fread,pid_fread);
                    list_add(parametros_fread,direccion_leer);
                    list_add(parametros_fread,contenido_read);
                    
                    enviar_op_con_parametros(F_READ,parametros_fread,socket_memoria); // ENVIO CONTENIDO LEIDO A MEMORIA

                    recv(socket_memoria,&resultado,sizeof(uint32_t),MSG_WAITALL); // ESPERO QUE LA OPERACION SE COMPLETE EN MEMORIA

                    send(socket_kernel,&cod_found,sizeof(uint32_t),0);
                }
                break;

            case F_WRITE:
                char* tamanio_a_escribir = list_get(parametros,1);
                char* direccion_escribir = list_get(parametros,2);
                char* posicion_escribir  = list_get(parametros,3);
                char* pid_fwrite         = list_get(parametros,4);

                log_info(logger,"Escribir Archivo: %s - Puntero: %s - Memoria: %s - Tamaño: %s ",nombre_archivo,posicion_escribir,direccion_escribir,tamanio_a_escribir);

                t_fcb* fcb_escribir = dictionary_get(archivos_abiertos,nombre_archivo);

                if (fcb_escribir == NULL){
                    uint32_t cod_not_found = 404;
                    send(socket_kernel,&cod_not_found,sizeof(uint32_t),0);
                } else{
                    uint32_t cod_found = 200;

                    t_list* parametros_fwrite = list_create();
                    list_add(parametros_fwrite,pid_fwrite);
                    list_add(parametros_fwrite,direccion_escribir);
                    list_add(parametros_fwrite,tamanio_a_escribir);
                    
                    enviar_op_con_parametros(F_WRITE,parametros_fwrite,socket_memoria); // ENVIO INFORMACION A MEMORIA PARA QUE ME MANDE CONTENIDO

                    char* contenido_a_escribir = malloc(atoi(tamanio_a_escribir));

                    recv(socket_memoria,contenido_a_escribir,atoi(tamanio_a_escribir),MSG_WAITALL); // RECIBO EL CONTENIDO A ESCRIBIR DESDE MEMORIA

                    escribir_archivo_bloques(fcb_escribir,atoi(posicion_escribir),atoi(tamanio_a_escribir),contenido_a_escribir,nombre_archivo);

                    send(socket_kernel,&cod_found,sizeof(uint32_t),0);
                }
                break;
            
            default:
                log_error(logger,"Operacion no reconocida");
                break;
        }
        list_remove(parametros,0);
        list_destroy_and_destroy_elements(parametros,free);
    }

    // FINISH
    finalizar_filesystem();
    return 0;
}

t_list* recibir_parametros_fs(){
    t_list* parametros = list_create();
    int ok;
    uint32_t list_size;

    recv(socket_kernel,&list_size,sizeof(uint32_t),MSG_WAITALL);

    for(int i=0;i < list_size;i++){
        uint32_t param_size;

        recv(socket_kernel,&param_size,sizeof(uint32_t),MSG_WAITALL);

        char* param = malloc(param_size);

        ok = recv(socket_kernel,param,param_size,MSG_WAITALL);

        list_add(parametros,param);
    } 
    
    if(ok <= 0){
        log_error(logger,"Kernel desconectado");
        finalizar_filesystem();
        return 0;
    }

    return parametros;
}

char* ruta_archivo(char* nombre_archivo){
    char* ruta = string_new();
                string_append(&ruta,"cfg/FCBs/");
                string_append(&ruta,nombre_archivo);
                string_append(&ruta,".dat");
        
    return ruta;
}