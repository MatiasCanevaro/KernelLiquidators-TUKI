#include <motivo_desalojo.h>

void manejar_motivo_desalojo(t_pcb* pcb,t_list* motivo_desalojo) {
    uint8_t motivo = (uint8_t) list_get(motivo_desalojo,0);
    int salirSwitch=0;
    int ejecutar=0;

    switch (motivo){
    case IO:

        log_info(logger, "PID: %d - Estado Anterior: EXEC - Estado Actual: BLOCK", pcb->PID);
        log_info(logger, "PID: %d - Bloqueado por: I/O", pcb->PID);

        int tiempo = atoi(list_get(motivo_desalojo,1));

        pthread_t hilo_io;
        
        t_param_io* param = malloc(sizeof(t_param_io));
        param->pcb=pcb;
        param->tiempo=tiempo;
        
        pthread_create(&hilo_io,NULL,proceso_io, param);
        pthread_detach(&hilo_io);
        break;

    case F_OPEN:
        char* archivo_open = malloc(strlen(list_get(motivo_desalojo,1))+1);
        memcpy(archivo_open,list_get(motivo_desalojo,1),strlen(list_get(motivo_desalojo,1))+1);
        //si existe el archivo en la TAG -> lo agrego a la cola de ese archivo
        if(dictionary_has_key(tabla_archivos,archivo_open)){
            log_info(logger, "PID: %d - Estado Anterior: EXEC - Estado Actual: BLOCK", pcb->PID);
            log_info(logger, "PID: %d - Bloqueado por: %s", pcb->PID,archivo_open);
            t_queue* cola_archivo_open=dictionary_get(tabla_archivos,archivo_open);
            queue_push(cola_archivo_open,pcb);
        }
        else { //si no existe el archivo en la TAG -> creo la cola en la TAG y pregunto a fs 
        // si el archivo existe
            log_info(logger, "PID: %d - Abrir Archivo: %s", pcb->PID, archivo_open);
        
            t_queue* cola_archivo_open=queue_create();
            queue_push(cola_archivo_open,pcb);
            dictionary_put(tabla_archivos,archivo_open,cola_archivo_open);

            t_list* parametros_fs_open = list_create();
            char* archivo_a_guardar = malloc(strlen(archivo_open)+1);
            memcpy(archivo_a_guardar,archivo_open,strlen(archivo_open)+1);
            list_add(parametros_fs_open,archivo_a_guardar);
            
            pthread_mutex_lock(&mutex_send_fs);
            enviar_operacion_a_filesystem(ABRIR_ARCHIVO,parametros_fs_open);
            
            uint32_t resultado_open;
            recv(socket_filesystem,&resultado_open,sizeof(uint32_t),MSG_WAITALL);
            pthread_mutex_unlock(&mutex_send_fs);

            if(resultado_open == 404) {
                enviar_operacion_a_filesystem(CREAR_ARCHIVO,parametros_fs_open);
                uint32_t resultado_create;
                recv(socket_filesystem,&resultado_create,sizeof(uint32_t),MSG_WAITALL);
            }

            list_destroy_and_destroy_elements(parametros_fs_open, free);
            ejecutar++;
        }

        t_entrada_tabla_archivos* entrada=malloc(sizeof(t_entrada_tabla_archivos));
        entrada->posicion=0;
        entrada->nombre=archivo_open;
        list_add(pcb->archivos_abiertos,entrada);

        break;
        
    case F_CLOSE:
        char* archivo_close = malloc(strlen(list_get(motivo_desalojo,1))+1);
        memcpy(archivo_close,list_get(motivo_desalojo,1),strlen(list_get(motivo_desalojo,1))+1);
        log_info("PID: %d - Cerrar Archivo: %s", pcb->PID, archivo_close);

        for(int i=0;i<list_size(pcb->archivos_abiertos);i++) {
            t_entrada_tabla_archivos* entrada_close = (t_entrada_tabla_archivos* )list_get(pcb->archivos_abiertos,i);
            if(strcmp(entrada_close->nombre,archivo_close)==0) {
                list_remove(pcb->archivos_abiertos,i);
                break;
            }
        }
        
        t_queue* cola_archivo_close = dictionary_get(tabla_archivos,archivo_close);
        queue_pop(cola_archivo_close);
    
        if(queue_size(cola_archivo_close)!=0){
            t_pcb* pcb_ejecutar = queue_peek(cola_archivo_close);

            pthread_mutex_lock(&mutex_cola_ready);
            queue_push(cola_ready, pcb_ejecutar);
            log_info(logger, "PID: %d - Estado Anterior: BLOCK - Estado Actual: READY", pcb->PID);
            ingreso_a_ready();
            pthread_mutex_unlock(&mutex_cola_ready);
            sem_post(&sem_cola_ready_vacia);
        } else{
            dictionary_remove(tabla_archivos,archivo_close);
        }
        ejecutar++;
        break;
        
    case F_SEEK:
        char* archivo_seek = malloc(strlen(list_get(motivo_desalojo,1))+1);
        memcpy(archivo_seek,list_get(motivo_desalojo,1),strlen(list_get(motivo_desalojo,1))+1);
        uint32_t posicion = (uint32_t)atoi(list_get(motivo_desalojo,2));
        
        for(int i=0;i<list_size(pcb->archivos_abiertos);i++){
            t_entrada_tabla_archivos* entrada_seek = (t_entrada_tabla_archivos* )list_get(pcb->archivos_abiertos,i);
            if(strcmp(entrada_seek->nombre,archivo_seek)==0) {
                entrada_seek->posicion = posicion;
                break;
            }       
        }

        log_info(logger, "PID: %d - Actualizar puntero archivo: %s - Puntero %d", pcb->PID,archivo_seek,(int)posicion);

        ejecutar++;
        break;
        
    case F_READ:
        char* archivo_read= malloc(strlen(list_get(motivo_desalojo,1))+1);
        memcpy(archivo_read,list_get(motivo_desalojo,1),strlen(list_get(motivo_desalojo,1))+1);

        char* tamanio_a_leer_read=malloc(strlen(list_get(motivo_desalojo,2))+1);
        memcpy(tamanio_a_leer_read,list_get(motivo_desalojo,2),strlen(list_get(motivo_desalojo,2))+1);

        char* direccion_fisica_read = malloc(strlen(list_get(motivo_desalojo,3))+1);
        memcpy(direccion_fisica_read,list_get(motivo_desalojo,3),strlen(list_get(motivo_desalojo,3))+1);
        
        char* posicion_read;

        for(int i=0;i<list_size(pcb->archivos_abiertos);i++){
            t_entrada_tabla_archivos* entrada_write = (t_entrada_tabla_archivos* )list_get(pcb->archivos_abiertos,i);
            if(strcmp(entrada_write->nombre,archivo_read)==0) {
                posicion_read = string_itoa(entrada_write->posicion);
                break;
            }       
        }
        for(int i=0;i<list_size(pcb->archivos_abiertos);i++){
            t_entrada_tabla_archivos* entrada_read = (t_entrada_tabla_archivos* )list_get(pcb->archivos_abiertos,i);
            if(strcmp(entrada_read->nombre,archivo_read)==0) {
                posicion_read = string_itoa(entrada_read->posicion);
                break;
            }       
        }
        char* pid_fread=string_itoa(pcb->PID);
        t_list* parametros_fread=list_create();
        list_add(parametros_fread,pcb);
        list_add(parametros_fread,F_READ);
        list_add(parametros_fread,archivo_read);
        list_add(parametros_fread,tamanio_a_leer_read);
        list_add(parametros_fread,direccion_fisica_read);
        list_add(parametros_fread,posicion_read);
        list_add(parametros_fread,pid_fread);
        
        sem_wait(&sem_memoria_fs);

        pthread_mutex_lock(&mutex_cola_fs);
        
        log_info(logger, "PID: %d - Escribir Archivo: %s - Puntero %d - Direccion Memoria %s - Tamaño %d",pcb->PID, archivo_read,atoi(posicion_read),direccion_fisica_read,atoi(tamanio_a_leer_read));

        queue_push(cola_fs,parametros_fread);

        pthread_mutex_unlock(&mutex_cola_fs);

        sem_post(&sem_cola_fs);

        break;

    case F_WRITE:
        char* archivo_write = malloc(strlen(list_get(motivo_desalojo,1))+1);
        memcpy(archivo_write,list_get(motivo_desalojo,1),strlen(list_get(motivo_desalojo,1))+1);

        char* tamanio_a_leer_write=malloc(strlen(list_get(motivo_desalojo,2))+1);
        memcpy(tamanio_a_leer_write,list_get(motivo_desalojo,2),strlen(list_get(motivo_desalojo,2))+1);
        char* direccion_fisica_write = malloc(strlen(list_get(motivo_desalojo,3))+1);
        memcpy(direccion_fisica_write,list_get(motivo_desalojo,3),strlen(list_get(motivo_desalojo,3))+1);
        char* posicion_write;

        for(int i=0;i<list_size(pcb->archivos_abiertos);i++){
            t_entrada_tabla_archivos* entrada_write = (t_entrada_tabla_archivos* )list_get(pcb->archivos_abiertos,i);
            if(strcmp(entrada_write->nombre,archivo_write)==0) {
                posicion_write = string_itoa(entrada_write->posicion);
                break;
            }       
        }
        
        char* pid_fwrite=string_itoa(pcb->PID);
        t_list* parametros_fwrite=list_create();
        list_add(parametros_fwrite,pcb);
        list_add(parametros_fwrite,F_WRITE);
        list_add(parametros_fwrite,archivo_write);
        list_add(parametros_fwrite,tamanio_a_leer_write);
        list_add(parametros_fwrite,direccion_fisica_write);
        list_add(parametros_fwrite,posicion_write);
        list_add(parametros_fwrite,pid_fwrite);

        sem_wait(&sem_memoria_fs);

        pthread_mutex_lock(&mutex_cola_fs);
        
        log_info(logger, "PID: %d - Leer Archivo: %s - Puntero %d - Direccion Memoria %s - Tamaño %d",pcb->PID, archivo_write,atoi(posicion_write),direccion_fisica_write,atoi(tamanio_a_leer_write));
        
        queue_push(cola_fs,parametros_fwrite);

        pthread_mutex_unlock(&mutex_cola_fs);

        sem_post(&sem_cola_fs);

        break;

    case F_TRUNCATE:
        char* archivo_truncate = malloc(strlen(list_get(motivo_desalojo,1))+1);
        memcpy(archivo_truncate,list_get(motivo_desalojo,1),strlen(list_get(motivo_desalojo,1))+1);
        char* tamanio_truncate = malloc(strlen(list_get(motivo_desalojo,2))+1);
        memcpy(tamanio_truncate,list_get(motivo_desalojo,2),strlen(list_get(motivo_desalojo,2))+1);
        
        t_list* parametros_ftruncate=list_create();
        list_add(parametros_ftruncate,pcb);
        list_add(parametros_ftruncate,F_TRUNCATE);
        list_add(parametros_ftruncate,archivo_truncate);
        list_add(parametros_ftruncate,tamanio_truncate);
       
        pthread_mutex_lock(&mutex_cola_fs);
        
        log_info(logger, "PID: %d - Archivo: %s - Tamaño: %d",pcb->PID, archivo_truncate,atoi(tamanio_truncate));
        queue_push(cola_fs,parametros_ftruncate);

        pthread_mutex_unlock(&mutex_cola_fs);

        sem_post(&sem_cola_fs);

        break;

    case WAIT:
        char* nombre_recurso_wait = list_get(motivo_desalojo,1);
        
        for(int i=0;i<list_size(lista_recursos_compartidos);i++){
            t_recurso* recurso= list_get(lista_recursos_compartidos,i);
            if(!strcmp(recurso->nombre,nombre_recurso_wait)){
                recurso->cant_instancias--;
                log_info(logger, "PID: %d - Wait: %s - Instancias: %d",pcb->PID, recurso->nombre,recurso->cant_instancias);
                if(recurso->cant_instancias<0){
                    queue_push(recurso->cola_bloqueados_recurso,pcb);
                    log_info(logger,"PID: %d - Estado Anterior: EXEC - Estado Actual: BLOCK",pcb->PID);
                    log_info(logger, "PID: %d - Bloqueado por: %s",pcb->PID,recurso->nombre);
                }else{
                    queue_push(recurso->cola_uso_recurso,pcb);
                    ejecutar++;
                }
                salirSwitch++;
            }
        }
        if(salirSwitch) {
            salirSwitch=0;
            break;
        }
        log_error(logger, "Finaliza el proceso %d - Motivo: WAIT_A_RECURSO_NO_RECONOCIDO",pcb->PID);
        enviar_a_exit(pcb);   
        break;

    case SIGNAL:
        char* nombre_recurso_signal = list_get(motivo_desalojo,1);
        
        for(int i=0;i<list_size(lista_recursos_compartidos);i++){
            t_recurso* recurso = list_get(lista_recursos_compartidos,i);
            if(strcmp(recurso->nombre,nombre_recurso_signal)==0){
                recurso->cant_instancias++;

                log_info(logger, "PID: %d - Signal: %s - Instancias: %d",pcb->PID, recurso->nombre,recurso->cant_instancias);
                
                if(queue_size(recurso->cola_uso_recurso)>0){
                    queue_pop(recurso->cola_uso_recurso);    
                }
                desbloquear_proceso_rec(recurso);
                ejecutar++;
                salirSwitch++;
            }
        }
        if(salirSwitch) {
            salirSwitch=0;
            break;
        }
        log_error(logger, "Finaliza el proceso %d - Motivo: SIGNAL_A_RECURSO_NO_RECONOCIDO", pcb->PID);
        enviar_a_exit(pcb);
        break;

    case CREATE_SEGMENT:
        t_list* lista_create_segment=list_create();
        char*pid=string_itoa(pcb->PID);
        log_info(logger,"PID: %d - Crear Segmento - Id: %d- Tamaño: %d",pcb->PID,atoi(list_get(motivo_desalojo,1)),atoi(list_get(motivo_desalojo,2)));
        list_add(lista_create_segment,pid);
        list_add(lista_create_segment,list_get(motivo_desalojo,1));
        list_add(lista_create_segment,list_get(motivo_desalojo,2));
        int enviarExit=0;
        pthread_mutex_lock(&mutex_memoria);

        enviar_operacion_a_memoria(CREATE_SEGMENT,lista_create_segment);

        
        uint32_t resultado_create_segment;

        recv(socket_memoria,&resultado_create_segment,sizeof(uint32_t),MSG_WAITALL);

        if(resultado_create_segment==200){
            t_entrada_tabla_segmentos* entrada_tabla_segmentos=list_get(pcb->tabla_segmentos,atoi(list_get(motivo_desalojo,1)));
            entrada_tabla_segmentos->tamanio=atoi(list_get(motivo_desalojo,2));
            recv(socket_memoria,&entrada_tabla_segmentos->base,sizeof(uint32_t),MSG_WAITALL);
            ejecutar++;
        }
        if(resultado_create_segment==NO_HAY_ESPACIO_SUFICIENTE){
            log_error(logger, "Finaliza el proceso %d - Motivo: OUT_OF_MEMORY", pcb->PID);
            enviarExit++;
        }
        if(resultado_create_segment==HAY_ESPACIO_TOTAL_NO_CONTIGUO){
            uint8_t compactar=COMPACTAR;
            int valor_sem;
            sem_getvalue(&sem_memoria_fs,&valor_sem);
            if(valor_sem<=0){
                log_info(logger,"Compactación: Esperando Fin de Operaciones de FS");
            }
            sem_wait(&sem_memoria_fs);
            log_info(logger,"Compactación: Se solicitó compactación");
            send(socket_memoria,&compactar,sizeof(uint8_t),0);
            uint32_t cero=0;
            send(socket_memoria,&cero,sizeof(uint32_t),0);
            recibir_resultado_compactacion();
            log_info(logger,"Se finalizó el proceso de compactación");
            sem_post(&sem_memoria_fs);

            ///Vuelvo a mandar la operacion
            enviar_operacion_a_memoria(CREATE_SEGMENT,lista_create_segment);
            recv(socket_memoria,&resultado_create_segment,sizeof(uint32_t),MSG_WAITALL);
            t_entrada_tabla_segmentos* entrada_tabla_segmentos=list_get(pcb->tabla_segmentos,atoi(list_get(motivo_desalojo,1)));
            entrada_tabla_segmentos->tamanio=atoi(list_get(motivo_desalojo,2));
            recv(socket_memoria,&entrada_tabla_segmentos->base,sizeof(uint32_t),MSG_WAITALL);
            ejecutar++;
        }

        list_destroy(lista_create_segment);
        free(pid);


        pthread_mutex_unlock(&mutex_memoria);
        if (enviarExit) enviar_a_exit(pcb);
        break;

    case DELETE_SEGMENT:
    t_list* lista_delete_segment=list_create();
        char*pid_delete=string_itoa(pcb->PID);
        log_info(logger,"PID: %d - Eliminar Segmento - Id: %d",pcb->PID,atoi(list_get(motivo_desalojo,1)));
        list_add(lista_delete_segment,pid_delete);
        list_add(lista_delete_segment,list_get(motivo_desalojo,1));
        
        pthread_mutex_lock(&mutex_memoria);

        enviar_operacion_a_memoria(DELETE_SEGMENT,lista_delete_segment);

        list_destroy(lista_delete_segment);
        free(pid_delete);
        uint32_t resultado_delete_segment;

        recv(socket_memoria,&resultado_delete_segment,sizeof(uint32_t),MSG_WAITALL);

        if(resultado_delete_segment==200){
            t_entrada_tabla_segmentos* entrada_tabla_segmentos=list_get(pcb->tabla_segmentos,atoi(list_get(motivo_desalojo,1)));
            entrada_tabla_segmentos->tamanio=0;
            entrada_tabla_segmentos->base=0;
        }
        pthread_mutex_unlock(&mutex_memoria);


        ejecutar++;
        break;

    case YIELD:
        
        t_temporal* tiempo_actual=temporal_create();
        temporal_destroy(pcb->tiempo_llegada_ready);
        pcb->tiempo_llegada_ready=tiempo_actual;

        pthread_mutex_lock(&mutex_cola_ready);
        queue_push(cola_ready,pcb);

        log_info(logger, "PID: %d - Estado Anterior: EXEC - Estado Actual: READY", pcb->PID);

        ingreso_a_ready(pcb);
        
        pthread_mutex_unlock(&mutex_cola_ready);

        sem_post(&sem_cola_ready_vacia);
        break;

    case EXIT:
        log_info(logger, "Finaliza el proceso %d - Motivo: SUCCESS", pcb->PID);
        enviar_a_exit(pcb);
        break;

    case SEG_FAULT:
        log_error(logger, "Finaliza el proceso %d - Motivo: SEG_FAULT", pcb->PID);
        enviar_a_exit(pcb);
        break;

    default:
        log_error(logger,"Instruccion no reconocida");
        break;
    }

    list_remove(motivo_desalojo,0);
    list_destroy_and_destroy_elements(motivo_desalojo,free);
    if(ejecutar) {
        ejecutar_pcb(pcb);
        ejecutar=0;
    }
}

t_list* recibir_motivo_desalojo() {
    t_paquete* paquete = recibir_paquete(socket_cpu);

    int offset = 0;

    // OBTENER EL MOTIVO DE DESALOJO AL PRINCIPIO DEL BUFFER
    uint8_t motivo;
    memcpy(&motivo, paquete->buffer->stream + offset, sizeof(uint8_t));
    offset += sizeof(uint8_t);

    t_list* lista_strings = list_create();
    list_add(lista_strings, motivo);

    // OBTENER EL TAMAÑO DE LA LISTA DE STRINGS DEL BUFFER
    uint32_t lista_size;
    memcpy(&lista_size, paquete->buffer->stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // RECORRER EL BUFFER PARA EXTRAER LOS STRINGS
    while (offset < lista_size) {
        // OBTENER EL TAMAÑO DEL STRING
        uint32_t string_size;
        memcpy(&string_size, paquete->buffer->stream + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        // OBTENER EL STRING Y AGREGARLO A LA LISTA
        char* string = malloc(string_size);
        memcpy(string, paquete->buffer->stream + offset, string_size);
        offset += string_size;

        list_add(lista_strings, string);
    }

    // LIBERAR MEMORIA UTILIZADA
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
    
    // DEVOLVER LA LISTA DE STRINGS Y EL MOTIVO DE DESALOJO
    return lista_strings;
}

void recibir_resultado_compactacion(){
    uint32_t cant_tablas;
    uint32_t cant_max_segmentos;
    recv(socket_memoria,&cant_tablas,sizeof(uint32_t),MSG_WAITALL);
    recv(socket_memoria,&cant_max_segmentos,sizeof(uint32_t),MSG_WAITALL);
    t_list* lista_pids=recibir_pids(cant_tablas);
    t_list* lista_tablas= recibir_lista_tablas(cant_max_segmentos,cant_tablas);

    for(int i=0;i<cant_tablas;i++){
        t_pcb* pcb= dictionary_get(diccionario_tds,list_get(lista_pids,i));
        destruir_lista_tabla_segmentos(pcb->tabla_segmentos);
        pcb->tabla_segmentos=list_get(lista_tablas,i);
    }
}
t_list* recibir_pids(uint32_t cantidad){
    t_list* lista_pids=list_create();
    for(int i=0;i<cantidad;i++){
        uint32_t tamaño;
        recv(socket_memoria,&tamaño,sizeof(uint32_t),MSG_WAITALL);
        char* pid=malloc(tamaño);
        recv(socket_memoria,pid,tamaño,MSG_WAITALL);
        list_add(lista_pids,pid);
    }
    return lista_pids;
}
t_list* recibir_lista_tablas(uint32_t cantidad_segmentos,uint32_t cant_tablas){
    t_list* lista_tablas=list_create();
    for(int i=0;i<cant_tablas;i++){
        t_list* lista_segmentos=list_create();
        for(int j=0;j<cantidad_segmentos;j++){
            t_entrada_tabla_segmentos* entrada_tabla_segmentos=malloc(sizeof(t_entrada_tabla_segmentos));
            recv(socket_memoria,entrada_tabla_segmentos,sizeof(t_entrada_tabla_segmentos),MSG_WAITALL);
            list_add(lista_segmentos,entrada_tabla_segmentos);
        }
        list_add(lista_tablas,lista_segmentos);
    }
    return lista_tablas;
}