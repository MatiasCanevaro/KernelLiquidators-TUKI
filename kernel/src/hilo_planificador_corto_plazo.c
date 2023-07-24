#include <hilo_planificador_corto_plazo.h>

void* planificador_corto_plazo() {
    while(1)
    {
        sem_wait(&sem_cola_ready_vacia);

        pthread_mutex_lock(&mutex_cola_ready);
        t_pcb* pcb = queue_pop(cola_ready);
        log_info(logger,"PID: %d - Estado Anterior: READY - Estado Actual: EXEC", pcb->PID);

        pthread_mutex_unlock(&mutex_cola_ready);

        ejecutar_pcb(pcb);
    }  
}

void ejecutar_pcb(t_pcb* pcb) {
    // ENVIO A EJECUTAR A LA CPU
    t_buffer* buffer_pcb_cpu= serializar_pcb_cpu(pcb);
    int resultado_envio = enviar_paquete_pcb(buffer_pcb_cpu,socket_cpu);
    if(resultado_envio == -1) {
        log_error(logger,"Error al enviar el pcb a la cpu");
    }

    t_temporal* tiempo_envio_a_cpu = temporal_create();
    

    // RECIBO RESPUESTA DE LA CPU
    t_pcb_kernel* pcb_recibido = recibir_paquete_pcb_kernel(socket_cpu);
    pcb->program_counter = pcb_recibido->program_counter;
    *pcb->registros_cpu = *pcb_recibido->registros_cpu;
    
    free(pcb_recibido->registros_cpu);
    free(pcb_recibido);

    t_list* motivo_desalojo=recibir_motivo_desalojo();
    
    if(strcmp("HRRN",algoritmo_planificacion)==0){
        pcb->est_proxima_rafaga = estimadoProximaRafaga(tiempo_envio_a_cpu, pcb->est_proxima_rafaga);
    }

    temporal_destroy(tiempo_envio_a_cpu);
    
    // MANEJO MOTIVO DE DESALOJO
    manejar_motivo_desalojo(pcb,motivo_desalojo);
}

float calcularRR(t_pcb* pcb, t_temporal* tiempo){
    float w = temporal_diff(pcb->tiempo_llegada_ready,tiempo);
    float s = pcb->est_proxima_rafaga;

    return (w+s)/s;
}

uint32_t estimadoProximaRafaga(t_temporal* tiempo_envio_a_cpu, uint32_t est_anterior){
    int tiempo_ejecucion = temporal_gettime(tiempo_envio_a_cpu);
    uint32_t nuevo_estimado = (uint32_t) (hrrn_alpha * tiempo_ejecucion + (1 - hrrn_alpha) * est_anterior);

    if(nuevo_estimado == 0) {
        return 1;
    }

    return nuevo_estimado;
}

void enviar_a_exit(t_pcb* pcb){
    // AVISO A LA CONSOLA QUE TERMINO EL PROCESO
    uint32_t termino_ok=200;
    int enviado=send((int) pcb->socket_consola, &termino_ok, sizeof(uint32_t), NULL);
    log_info(logger,"Se envio a la consola el mensaje de que termino el proceso, socket: %d",pcb->socket_consola);
    if (enviado<0){
        log_error(logger,"No se pudo enviar el mensaje a la consola, socket: %d",pcb->socket_consola);
    }
    //LIBERAR RECURSOS
    for(int i=0;i<list_size(lista_recursos_compartidos);i++){
        t_recurso* recurso = list_get(lista_recursos_compartidos,i);
        for(int j=0;j<list_size(recurso->cola_uso_recurso->elements);j++){
            t_pcb* proceso_actual=list_get(recurso->cola_uso_recurso->elements,j);
            if(pcb==proceso_actual){
                recurso->cant_instancias++;
                list_remove(recurso->cola_uso_recurso->elements,j);
                log_info(logger, "PID: %d - Libero el recurso: %s - Instancias: %d",pcb->PID,recurso->nombre,recurso->cant_instancias);
                desbloquear_proceso_rec(recurso);
                j--;
            }
        }
    }
    //CERRAR ARCHIVOS
    int cant_archivos=list_size(pcb->archivos_abiertos);
    for(int i=0;i<cant_archivos;i++){
        t_entrada_tabla_archivos* entrada=(t_entrada_tabla_archivos* )list_get(pcb->archivos_abiertos,i);
        t_queue* cola_archivo = dictionary_get(tabla_archivos,entrada->nombre);
        queue_pop(cola_archivo);
        
        if(queue_size(cola_archivo)!=0){
            t_pcb* pcb_ejecutar = queue_peek(cola_archivo);
            pthread_mutex_lock(&mutex_cola_ready);
            
            queue_push(cola_ready, pcb_ejecutar);

            log_info(logger, "PID: %d - Estado Anterior: BLOCK - Estado Actual: READY", pcb_ejecutar->PID);

            ingreso_a_ready();
            
            pthread_mutex_unlock(&mutex_cola_ready);

            sem_post(&sem_cola_ready_vacia);
        } else{
            dictionary_remove(tabla_archivos,entrada->nombre);
        }
    }
    

    // TODO (avisar a memoria)
    
    t_list* params_memoria=list_create();
    list_add(params_memoria,string_itoa((int) pcb->PID));
    pthread_mutex_lock(&mutex_memoria);
    
    enviar_operacion_a_memoria(ELIMINAR_PROCESO,params_memoria);
    uint32_t resultado;

    recv(socket_memoria,&resultado,sizeof(uint),MSG_WAITALL);
    pthread_mutex_unlock(&mutex_memoria);
    list_destroy_and_destroy_elements(params_memoria,free);
    
    pthread_mutex_lock(&mutex_diccionario_tds);
    char* pid=string_itoa(pcb->PID);
    dictionary_remove(diccionario_tds,pid);
    
    pthread_mutex_unlock(&mutex_diccionario_tds);

    destruir_pcb(pcb);  

    sem_post(&sem_grado_multiprogramacion);
}

void desbloquear_proceso_rec(t_recurso* recurso){
    if(queue_size(recurso->cola_bloqueados_recurso)>0){
            t_pcb* proceso_liberado=queue_pop(recurso->cola_bloqueados_recurso);
            queue_push(recurso->cola_uso_recurso,proceso_liberado);

            pthread_mutex_lock(&mutex_cola_ready);
            queue_push(cola_ready, proceso_liberado);
            
            log_info(logger,"PID: %d - Estado Anterior: BLOCK - Estado Actual: READY",proceso_liberado->PID);

            ingreso_a_ready();

            pthread_mutex_unlock(&mutex_cola_ready);

            sem_post(&sem_cola_ready_vacia);
        }          
}