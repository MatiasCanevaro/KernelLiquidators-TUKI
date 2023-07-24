#include <hilo_agregar_ready.h>

// PASAR DE NEW A READY
void* agregar_ready(){
    while (1)
    {
        sem_wait(&sem_grado_multiprogramacion);
        
        sem_wait(&sem_cola_new_vacia);

        pthread_mutex_lock(&mutex_cola_new);
        t_pcb* pcb = queue_pop(cola_new);
        pthread_mutex_unlock(&mutex_cola_new);

        // ASIGNO TIEMPO DE LLEGADA A READY
        pcb->tiempo_llegada_ready = temporal_create();

        pthread_mutex_lock(&mutex_cola_ready);

        queue_push(cola_ready,pcb);

        log_info(logger, "PID: %d - Estado Anterior: NEW - Estado Actual: READY", pcb->PID);

        ingreso_a_ready();
        
        pthread_mutex_unlock(&mutex_cola_ready);

        sem_post(&sem_cola_ready_vacia);
 
        // TODO tabla de segmentos            
    }
}

char* generar_lista_pids(){
    char* lista_pids = string_new();
        string_append(&lista_pids, "[");
        
        uint32_t tam_lista_ready=queue_size(cola_ready); 
        
        for(int i=0;i<tam_lista_ready;i++) {
            int id = (int) ((t_pcb*) list_get(cola_ready->elements,i))->PID;
            char* id_string=string_itoa(id);
            string_append(&lista_pids, id_string);
            free(id_string);
            if(i!=tam_lista_ready-1)string_append(&lista_pids, ","); 
        }
        
        string_append(&lista_pids, "]");
    return lista_pids;
}