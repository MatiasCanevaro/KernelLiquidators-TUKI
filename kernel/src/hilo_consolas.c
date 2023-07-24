#include <hilo_consolas.h>

void* esperar_consolas(){
    while(1){
        int socket_consola = esperar_cliente(socket_servidor); 
        int hs = handshake_servidor(socket_consola);
        if(hs<0){
            log_error(logger,"Resultado del handshake incorrecto");
            liberar_conexion(socket_consola);
        }
        else {
        log_info(logger,"Se conecto la consola %d", socket_consola);

        // RECIBO LISTA DE INSTRUCCIONES
        t_list* lista_instrucciones = recibir_paquete_instrucciones(socket_consola);
        uint32_t paquete_recibido_ok = 200;
        send(socket_consola, &paquete_recibido_ok, sizeof(uint32_t), NULL);
        
        // CREO PCB
        t_pcb* pcb= crear_pcb(lista_instrucciones,socket_consola);

        // AGREGO A LA COLA NEW
        pthread_mutex_lock(&mutex_cola_new);
        queue_push(cola_new, pcb);
        pthread_mutex_unlock(&mutex_cola_new);

        sem_post(&sem_cola_new_vacia);
        
        //destruir_lista_instrucciones(lista_instrucciones);
        }
    }
}

t_pcb* crear_pcb(t_list* instruccionesRecibidas,uint32_t socket_consola){
    t_pcb* pcb = malloc(sizeof(t_pcb)); 

    pthread_mutex_lock(&mutex_pid);
    pcb->PID = pid_actual;
    pid_actual++;
    pthread_mutex_unlock(&mutex_pid);

    (pcb->instrucciones) = instruccionesRecibidas;
    pcb->program_counter = 0;
    // TODO: TABLA SEGMENTOS
    // TABLA DE SEGMENTOS DE PRUEBA
    /*
    t_entrada_tabla_segmentos* entrada1 = malloc(sizeof(t_entrada_tabla_segmentos));
    entrada1->sid = 0;
    entrada1->base=123131;
    entrada1->tamanio=25;
    pcb->tabla_segmentos = list_create();
    list_add(pcb->tabla_segmentos,entrada1);
    */
    //TABLA DE SEGMENTOS
    
    pthread_mutex_lock(&mutex_memoria);

    pcb->tabla_segmentos = solcitar_tds_inicial_a_memoria(pcb->PID);

    pthread_mutex_lock(&mutex_diccionario_tds);

    char* pid=string_itoa(pcb->PID);
    dictionary_put(diccionario_tds,pid,pcb);
    pthread_mutex_unlock(&mutex_diccionario_tds);

    pthread_mutex_unlock(&mutex_memoria);
    
    // TODO: TABLA DE ARCHIVOS ABIERTOS
    pcb->archivos_abiertos = list_create();

    pcb->registros_cpu = malloc(sizeof(t_registros_cpu));
    memset(pcb->registros_cpu, 1, sizeof(t_registros_cpu));

    pcb->est_proxima_rafaga = estimacion_inicial;

    pcb->socket_consola=socket_consola;

    log_info(logger, "Se crea el proceso %d en NEW", pcb->PID);

    return pcb;
}

t_list* solcitar_tds_inicial_a_memoria(uint32_t pid){
    t_list* tabla_segmentos = list_create();
    uint32_t tamanio_tabla;
    t_list*lista=list_create();
    list_add(lista,string_itoa((int) pid));

    enviar_operacion_a_memoria(CREAR_PROCESO,lista);
    list_destroy(lista);

    recv(socket_memoria,&tamanio_tabla,sizeof(uint32_t),MSG_WAITALL);
    for (int i = 0; i < tamanio_tabla; i++)
    {
        t_entrada_tabla_segmentos* entrada = malloc(sizeof(t_entrada_tabla_segmentos));
        recv(socket_memoria,entrada,sizeof(t_entrada_tabla_segmentos),MSG_WAITALL);
        list_add(tabla_segmentos,entrada);
    }

    return tabla_segmentos;
}