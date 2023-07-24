#include <hilo_fs.h>

void* manejar_fs(){
    while (1)
    {   
        sem_wait(&sem_cola_fs);

        t_list* parametros_operacion;

        pthread_mutex_lock(&mutex_cola_fs);
        
        if(queue_size(cola_fs) > 0){
            parametros_operacion = queue_pop(cola_fs);
        }
        
        pthread_mutex_unlock(&mutex_cola_fs);
        
        t_pcb* pcb = list_remove(parametros_operacion, 0);
        uint8_t codigo_operacion = list_remove(parametros_operacion, 0);
        char* nombre_archivo= (char*)list_get(parametros_operacion,0);
        log_info(logger, "PID: %d - Bloqueado por: %s",pcb->PID,nombre_archivo);

        enviar_operacion_a_filesystem(codigo_operacion,parametros_operacion);
        
        uint32_t resultado_operacion;
        recv(socket_filesystem,&resultado_operacion, sizeof(uint32_t), 0);
        
        if(codigo_operacion==F_READ || codigo_operacion== F_WRITE){
            sem_post(&sem_memoria_fs);
            }
        pthread_mutex_unlock(&mutex_send_fs);
        
        if(resultado_operacion==200){         
            temporal_destroy(pcb->tiempo_llegada_ready);
    
            t_temporal* nuevo_tiempo_ready=temporal_create();
        
            pcb->tiempo_llegada_ready=nuevo_tiempo_ready;
            
            pthread_mutex_lock(&mutex_cola_ready);
            queue_push(cola_ready,pcb);
            
            log_info(logger, "PID: %d - Estado Anterior: BLOCK - Estado Actual: READY", pcb->PID);
        
            ingreso_a_ready();
        
            pthread_mutex_unlock(&mutex_cola_ready);
            sem_post(&sem_cola_ready_vacia);
            list_destroy_and_destroy_elements(parametros_operacion,free);
        }
    }
}