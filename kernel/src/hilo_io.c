#include <hilo_io.h>

void* proceso_io(t_param_io* param_io){

    log_info(logger, "PID: %d - Ejecuta I/O: %d", param_io->pcb->PID, param_io->tiempo);

    usleep(param_io->tiempo * 1000);

    temporal_destroy(param_io->pcb->tiempo_llegada_ready);
    
    t_temporal* nuevo_tiempo_ready=temporal_create();

    param_io->pcb->tiempo_llegada_ready=nuevo_tiempo_ready;
    
    pthread_mutex_lock(&mutex_cola_ready);
    queue_push(cola_ready,param_io->pcb);
    
    log_info(logger, "PID: %d - Estado Anterior: BLOCK - Estado Actual: READY", param_io->pcb->PID);

    ingreso_a_ready();

    pthread_mutex_unlock(&mutex_cola_ready);
    sem_post(&sem_cola_ready_vacia);

    
    free(param_io);
    
    return NULL;
}