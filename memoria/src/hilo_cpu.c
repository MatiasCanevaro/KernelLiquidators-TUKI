#include <hilo_cpu.h>

void* esperar_cpu(){
    while(1){
        uint8_t cod_op;
        recv(socket_cpu,&cod_op,sizeof(uint8_t),0);
        t_list* parametros_cpu = list_create();
        recibir_parametros_cpu(parametros_cpu);
        
        switch (cod_op){
            case MOV_IN:
                //recibe (pid, df, tamanio)
                log_info(logger, "PID: %d - Acción: LEER - Dirección física: %d - Tamaño: %d - Origen: CPU",atoi(list_get(parametros_cpu,0)),atoi(list_get(parametros_cpu,1)),atoi(list_get(parametros_cpu,2)));
                leer_memoria_cpu(atoi(list_get(parametros_cpu,1)),atoi(list_get(parametros_cpu,2)));
                break;
            
            case MOV_OUT:
                //recibe (pid, df, valor_a_escribir)
                log_info(logger, "PID: %d - Acción: ESCRIBIR - Dirección física: %d - Tamaño: %d - Origen: CPU",atoi(list_get(parametros_cpu,0)),atoi(list_get(parametros_cpu,1)),strlen(list_get(parametros_cpu,2)));
                escribir_memoria_cpu(atoi(list_get(parametros_cpu,1)),strlen(list_get(parametros_cpu,2)),list_get(parametros_cpu,2));
                break;

            default:
                log_warning(logger, "Operación desconocida");
                break;
        }
        list_destroy_and_destroy_elements(parametros_cpu,free);
    }
}

void recibir_parametros_cpu(t_list* parametros_cpu){
    int ok;
    uint32_t list_size;
    recv(socket_cpu,&list_size,sizeof(uint32_t),MSG_WAITALL);

    for(int i=0;i<list_size;i++) {
        uint32_t param_size;
        recv(socket_cpu,&param_size,sizeof(uint32_t),MSG_WAITALL);

        char* param = malloc(param_size);
        ok = recv(socket_cpu,param,param_size,MSG_WAITALL);

        list_add(parametros_cpu, param);
    } 
    if(ok<=0){
        log_error(logger,"Memoria desconectada");
        finalizar_memoria();
        return 0;
    }
}

void leer_memoria_cpu(uint32_t df, uint32_t tamanio){
    void* valor_leido = malloc(tamanio);

    pthread_mutex_lock(&mutex_memoria);
    memcpy(valor_leido,memoria+df,tamanio);//destino,origen,tamaño
    pthread_mutex_unlock(&mutex_memoria);
    
    usleep(retardo_memoria * 1000);
    send(socket_cpu,valor_leido,tamanio,NULL);
    free(valor_leido);
}

void escribir_memoria_cpu(uint32_t df, uint32_t tamanio, void* valor_a_escribir){
    pthread_mutex_lock(&mutex_memoria);
    memcpy(memoria+df,valor_a_escribir,tamanio);//destino,origen,tamaño
    pthread_mutex_unlock(&mutex_memoria);
    
    usleep(retardo_memoria * 1000);
    uint32_t ok = 200;
    send(socket_cpu,&ok,sizeof(uint32_t),NULL);
}