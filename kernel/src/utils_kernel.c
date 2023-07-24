#include <utils_kernel.h>
/* CUANDO SE USE SE DEBEN APLICAR MUTEX PARA LA COLA DE READY*/
void ingreso_a_ready(){
    if(strcmp(algoritmo_planificacion, "HRRN") == 0){
        t_temporal* tiempo_actual=temporal_create();
        void* compararRR(void* pcb1, void* pcb2){
            t_pcb* pcb1_cast = (t_pcb*) pcb1;
            t_pcb* pcb2_cast = (t_pcb*) pcb2;
            float RR1 = calcularRR(pcb1_cast, tiempo_actual);
            float RR2 = calcularRR(pcb2_cast, tiempo_actual);
            if(RR1 > RR2) return 1;
            else return 0;
        }
        list_sort(cola_ready->elements, (void*)compararRR);
        temporal_destroy(tiempo_actual);
    }

    char* lista_pids = generar_lista_pids();
    log_info(logger, "Cola Ready %s: %s", algoritmo_planificacion, lista_pids);
    free(lista_pids);
}

void enviar_operacion_con_parametros(uint8_t motivo, t_list* lista_parametros,int socket_fd){
    uint32_t cant_parametros =(uint32_t) list_size(lista_parametros);
    int size=sizeof(uint8_t)+sizeof(uint32_t);

    for(int i=0; i<list_size(lista_parametros); i++) {
        char* parametro = list_get(lista_parametros, i);
        size+=sizeof(uint32_t);
        size+=strlen(parametro)+1;
    }

    void* stream_a_enviar=malloc(size);
    int offset=0;

    memcpy(stream_a_enviar+offset,&motivo,sizeof(uint8_t));
    offset+=sizeof(uint8_t);

    memcpy(stream_a_enviar+offset,&cant_parametros,sizeof(uint32_t));
    offset+=sizeof(uint32_t);
    
    for(int i=0; i<list_size(lista_parametros); i++) {
        char* parametro = list_get(lista_parametros, i);
        uint32_t param_size = strlen(parametro)+1;
        memcpy(stream_a_enviar+offset,&param_size,sizeof(uint32_t));
        offset+=sizeof(uint32_t);
        memcpy(stream_a_enviar+offset,parametro,param_size);
        offset+=param_size;
    }
    
    send(socket_fd,stream_a_enviar,size,0);
    }

void enviar_operacion_a_filesystem(uint8_t motivo, t_list* lista_parametros) {
    enviar_operacion_con_parametros(motivo, lista_parametros, socket_filesystem);
}

void enviar_operacion_a_memoria(uint8_t motivo, t_list* lista_parametros){
    enviar_operacion_con_parametros(motivo, lista_parametros, socket_memoria);
}