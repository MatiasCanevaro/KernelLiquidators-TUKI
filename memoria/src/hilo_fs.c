#include <hilo_fs.h>

void* esperar_fs(){
    while(1){
        uint8_t cod_op;
        recv(socket_filesystem,&cod_op,sizeof(uint8_t),0);
        t_list* parametros_fs = list_create();
        recibir_parametros_fs(parametros_fs);
        
        switch (cod_op){
            case F_WRITE:
                //recibe (pid, df, tamanio)
                log_info(logger, "PID: %d - Acción: LEER - Dirección física: %d - Tamaño: %d - Origen: FS",atoi(list_get(parametros_fs,0)),atoi(list_get(parametros_fs,1)),atoi(list_get(parametros_fs,2)));
                leer_memoria_fs(atoi(list_get(parametros_fs,1)),atoi(list_get(parametros_fs,2)));
                break;
            
            case F_READ:
                //recibe (pid, df, tamanio, valor_a_escribir)
                log_info(logger, "PID: %d - Acción: ESCRIBIR - Dirección física: %d - Tamaño: %d - Origen: FS",atoi(list_get(parametros_fs,0)),atoi(list_get(parametros_fs,1)),strlen(list_get(parametros_fs,2)));
                escribir_memoria_fs(atoi(list_get(parametros_fs,1)),strlen(list_get(parametros_fs,2)),list_get(parametros_fs,2));
                break;

            default:
                log_warning(logger, "Operación desconocida");
                break;
        }
        list_destroy_and_destroy_elements(parametros_fs,free);
    }
}

void recibir_parametros_fs(t_list* parametros_fs){
    int ok;
    uint32_t list_size;
    recv(socket_filesystem,&list_size,sizeof(uint32_t),MSG_WAITALL);

    for(int i=0;i<list_size;i++) {
        uint32_t param_size;
        recv(socket_filesystem,&param_size,sizeof(uint32_t),MSG_WAITALL);

        char* param = malloc(param_size);
        ok = recv(socket_filesystem,param,param_size,MSG_WAITALL);

        list_add(parametros_fs, param);
    } 
}

void leer_memoria_fs(uint32_t df, uint32_t tamanio){
    void* valor_leido = malloc(tamanio);
    
    pthread_mutex_lock(&mutex_memoria);
    memcpy(valor_leido,memoria+df,tamanio);//destino,origen,tamaño
    pthread_mutex_unlock(&mutex_memoria);

    usleep(retardo_memoria * 1000);
    send(socket_filesystem,valor_leido,tamanio,NULL);
    free(valor_leido);
}

void escribir_memoria_fs(uint32_t df, uint32_t tamanio, void* valor_a_escribir){
    pthread_mutex_lock(&mutex_memoria);
    memcpy(memoria+df,valor_a_escribir,tamanio);//destino,origen,tamaño
    pthread_mutex_unlock(&mutex_memoria);

    usleep(retardo_memoria * 1000);
    uint32_t ok = 200;
    send(socket_filesystem,&ok,sizeof(uint32_t),NULL);
}