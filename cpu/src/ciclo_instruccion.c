#include <ciclo_instruccion.h>

void conexion_kernel(){
    while (1)
    {
        t_pcb_cpu* pcb = recibir_paquete_pcb_cpu(socket_kernel);
        registros_cpu=pcb->registros_cpu;
        ciclo_instruccion(pcb);
    }
}

void ciclo_instruccion(t_pcb_cpu* pcb){
    while (1)
    {
        int salir=0;
        
        t_instruccion* instruccion = list_get(pcb->instrucciones, pcb->program_counter);
        pcb->program_counter++;

        switch (instruccion->codigo)
        {
            case SET:
                log_info(logger,"PID: %i - Ejecutando: SET - %s %s",pcb->PID,list_get(instruccion->parametros,0),list_get(instruccion->parametros,1));
                usleep(retardo_instruccion*1000);
                ejecutar_set(instruccion);
                break;
            case MOV_IN:
                log_info(logger,"PID: %i - Ejecutando: MOV_IN - %s %s",pcb->PID,list_get(instruccion->parametros,0),list_get(instruccion->parametros,1));
                if(ejecutar_mov_in(pcb,instruccion->parametros)<0)
                    salir++;
                break;
            case MOV_OUT:
                log_info(logger,"PID: %i - Ejecutando: MOV_OUT - %s %s",pcb->PID,list_get(instruccion->parametros,0),list_get(instruccion->parametros,1));
                if(ejecutar_mov_out(pcb,instruccion->parametros)<0)
                    salir++;
                break;
            case IO:
                log_info(logger,"PID: %i - Ejecutando: I/O - %s",pcb->PID,list_get(instruccion->parametros,0));
                devolver_contexto(pcb, IO, instruccion->parametros);
                salir++;
                break;
            case F_OPEN:
                log_info(logger,"PID: %i - Ejecutando: F_OPEN - %s",pcb->PID,list_get(instruccion->parametros,0));
                devolver_contexto(pcb, F_OPEN, instruccion->parametros);
                salir++;
                break;
            case F_CLOSE:
                log_info(logger,"PID: %i - Ejecutando: F_CLOSE - %s",pcb->PID,list_get(instruccion->parametros,0));
                devolver_contexto(pcb, F_CLOSE, instruccion->parametros);
                salir++;
                break;
            case F_SEEK:
                log_info(logger,"PID: %i - Ejecutando: F_SEEK - %s %s",pcb->PID,list_get(instruccion->parametros,0),list_get(instruccion->parametros,1));
                devolver_contexto(pcb, F_SEEK, instruccion->parametros);
                salir++;
                break;
            case F_READ:
                log_info(logger,"PID: %i - Ejecutando: F_READ - %s %s %s",pcb->PID,list_get(instruccion->parametros,0),list_get(instruccion->parametros,1),list_get(instruccion->parametros,2));
                ejecutar_read_o_write(pcb, instruccion->parametros, F_READ);
                salir++;
                break;
            case F_WRITE:
                log_info(logger,"PID: %i - Ejecutando: F_WRITE - %s %s %s",pcb->PID,list_get(instruccion->parametros,0),list_get(instruccion->parametros,1),list_get(instruccion->parametros,2));
                ejecutar_read_o_write(pcb, instruccion->parametros, F_WRITE);
                salir++;
                break;
            case F_TRUNCATE:
                log_info(logger,"PID: %i - Ejecutando: F_TRUNCATE - %s %s",pcb->PID,list_get(instruccion->parametros,0),list_get(instruccion->parametros,1));
                devolver_contexto(pcb, F_TRUNCATE, instruccion->parametros);
                salir++;
                break;
            case WAIT:
                log_info(logger,"PID: %i - Ejecutando: WAIT - %s",pcb->PID,list_get(instruccion->parametros,0));
                devolver_contexto(pcb, WAIT, instruccion->parametros);
                salir++;
                break;
            case SIGNAL:
                log_info(logger,"PID: %i - Ejecutando: SIGNAL - %s",pcb->PID,list_get(instruccion->parametros,0));
                devolver_contexto(pcb, SIGNAL, instruccion->parametros);
                salir++;
                break;
            case CREATE_SEGMENT:
                log_info(logger,"PID: %i - Ejecutando: CREATE_SEGMENT - %s %s",pcb->PID,list_get(instruccion->parametros,0),list_get(instruccion->parametros,1));
                devolver_contexto(pcb, CREATE_SEGMENT, instruccion->parametros);
                salir++;
                break;
            case DELETE_SEGMENT:
                log_info(logger,"PID: %i - Ejecutando: DELETE_SEGMENT - %s",pcb->PID,list_get(instruccion->parametros,0));
                devolver_contexto(pcb, DELETE_SEGMENT, instruccion->parametros);
                salir++;
                break;
            case YIELD:
                log_info(logger,"PID: %i - Ejecutando: YIELD",pcb->PID);
                devolver_contexto(pcb, YIELD, instruccion->parametros);
                salir++;
                break;
            case EXIT:
                log_info(logger,"PID: %i - Ejecutando: EXIT",pcb->PID);
                devolver_contexto(pcb, EXIT, instruccion->parametros);
                salir++;
                break;
            default:
                log_info(logger,"PID: %i - INSTRUCCION NO RECONOCIDA",pcb->PID);
                break;
        }

        if(salir){
            return;
        }
    }
}

void ejecutar_set(t_instruccion* instruccion){
    char* nombre_registro = list_get(instruccion->parametros, 0);
    char* valor = list_get(instruccion->parametros, 1);

    if(strcmp(nombre_registro, "AX") == 0){
        strncpy(registros_cpu->AX, valor, 4);
    } else if(strcmp(nombre_registro, "BX") == 0){
        strncpy(registros_cpu->BX, valor, 4);
    } else if(strcmp(nombre_registro, "CX") == 0){
        strncpy(registros_cpu->CX, valor, 4);
    } else if(strcmp(nombre_registro, "DX") == 0){
        strncpy(registros_cpu->DX, valor, 4);
    } else if(strcmp(nombre_registro, "EAX") == 0){
        strncpy(registros_cpu->EAX, valor, 8);
    } else if(strcmp(nombre_registro, "EBX") == 0){
        strncpy(registros_cpu->EBX, valor, 8);
    } else if(strcmp(nombre_registro, "ECX") == 0){
        strncpy(registros_cpu->ECX, valor, 8);
    } else if(strcmp(nombre_registro, "EDX") == 0){
        strncpy(registros_cpu->EDX, valor, 8);
    } else if(strcmp(nombre_registro, "RAX") == 0){
        strncpy(registros_cpu->RAX, valor, 16);
    } else if(strcmp(nombre_registro, "RBX") == 0){
        strncpy(registros_cpu->RBX, valor, 16);
    } else if(strcmp(nombre_registro, "RCX") == 0){
        strncpy(registros_cpu->RCX, valor, 16);
    } else if(strcmp(nombre_registro, "RDX") == 0){
        strncpy(registros_cpu->RDX, valor, 16);
    }
}

void devolver_contexto(t_pcb_cpu* pcb, uint32_t motivo_desalojo, t_list* parametros){
    
    t_paquete* paquete_pcb = malloc(sizeof(t_paquete));
    paquete_pcb->codigo_operacion = PCB;
    paquete_pcb->buffer = serializar_pcb_kernel(pcb);

    t_paquete* paq_motivo_desalojo = malloc(sizeof(t_paquete));
    paq_motivo_desalojo->codigo_operacion = MOTIVO_DESALOJO;
    paq_motivo_desalojo->buffer = paquete_motivo_desalojo(motivo_desalojo,parametros);

    int offset=0;
    uint32_t size_stream=sizeof(uint8_t)*2+sizeof(uint32_t)*2+paquete_pcb->buffer->size+paq_motivo_desalojo->buffer->size;
    void* stream_a_enviar=malloc(size_stream);
    void* stream_pcb=serializar_paquete(paquete_pcb);
    void* stream_motivo_desalojo=serializar_paquete(paq_motivo_desalojo);
    
    memcpy(stream_a_enviar+offset,stream_pcb,paquete_pcb->buffer->size+sizeof(uint8_t)+sizeof(uint32_t));
    offset+=paquete_pcb->buffer->size+sizeof(uint8_t)+sizeof(uint32_t);
    memcpy(stream_a_enviar+offset,stream_motivo_desalojo,paq_motivo_desalojo->buffer->size+sizeof(uint8_t)+sizeof(uint32_t));

    send(socket_kernel,stream_a_enviar,size_stream,0);

    free(stream_a_enviar);
    free(stream_pcb);
    free(stream_motivo_desalojo);

    free(paquete_pcb->buffer->stream);
    free(paquete_pcb->buffer);
    free(paquete_pcb);
    
    free(paq_motivo_desalojo->buffer->stream);
    free(paq_motivo_desalojo->buffer);
    free(paq_motivo_desalojo);

    destruir_lista_instrucciones(pcb->instrucciones);
    destruir_lista_tabla_segmentos(pcb->tabla_segmentos);
    free(pcb->registros_cpu);
    free(pcb);
}

t_buffer* paquete_motivo_desalojo(uint8_t motivo, t_list* lista_strings) {
    // CALCULAR EL TAMAÑO TOTAL DEL BUFFER
    uint32_t buffer_size = sizeof(uint8_t) + sizeof(uint32_t);
    void* buffer_stream = NULL;

    void agregar_string_serializado(char* string) {
        // OBTENER EL TAMAÑO DEL STRING Y SUMAR 1 PARA INCLUIR EL CARACTER NULO
        uint32_t string_size = strlen(string) + 1;

        // AGREGAR EL TAMAÑO DEL STRING AL TAMAÑO TOTAL DEL BUFFER
        buffer_size += sizeof(uint32_t) + string_size;

        // REALIZAR LA REALOCACION DEL BUFFER SI ES NECESARIO
        buffer_stream = realloc(buffer_stream, buffer_size);

        // AGREGAR EL TAMAÑO DEL STRING AL BUFFER
        memcpy(buffer_stream + buffer_size - sizeof(uint32_t) - string_size, &string_size, sizeof(uint32_t));

        // AGREGAR EL STRING AL BUFFER
        memcpy(buffer_stream + buffer_size - string_size, string, string_size);
    }

    // SERIALIZAR LA LISTA DE STRINGS EN EL BUFFER
    list_iterate(lista_strings, (void*) agregar_string_serializado);

    // AGREGAR EL MOTIVO DE DESALOJO AL PRINCIPIO DEL BUFFER
    buffer_stream = realloc(buffer_stream, buffer_size);
    memcpy(buffer_stream, &motivo, sizeof(uint8_t));

    // AGREGAR EL TAMAÑO DE LA LISTA DE STRINGS AL BUFFER
    memcpy(buffer_stream + sizeof(uint8_t), &buffer_size, sizeof(uint32_t));

    // CREAR UN NUEVO BUFFER CON EL CONTENIDO SERIALIZADO
    t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer->size = buffer_size;
    buffer->stream = buffer_stream;

    // CREAR UN NUEVO PAQUETE CON EL CÓDIGO DE OPERACIÓN 1 (POR EJEMPLO)
    
    // DEVUELVO EL PAQUETE
    return buffer;
}

int ejecutar_mov_in(t_pcb_cpu* pcb, t_list* parametros){
    char* registro = encontrar_registro(list_get(parametros,0),pcb);
    int tamanio_registro = encontrar_tamanio_registro(list_get(parametros,0));
    int direccion_logica = atoi(list_get(parametros, 1));
    
    int num_segmento = direccion_logica/tam_max_segmento;
    int desplazamiento_segmento = direccion_logica%tam_max_segmento;

    t_entrada_tabla_segmentos* entrada=list_get(pcb->tabla_segmentos, num_segmento);

    int direccion_fisica=entrada->base+desplazamiento_segmento;
    list_remove(parametros, 0); //elimino registro
    list_remove(parametros, 0); //elimino la direccion logica
    list_add(parametros,string_itoa(direccion_fisica));
    list_add(parametros,string_itoa(tamanio_registro));
    
    if(tamanio_registro+desplazamiento_segmento>entrada->tamanio){
        log_error(logger, "PID: %d - Error SEG_FAULT - Segmento: %d - Offset: %d - Tamanio: %d", pcb->PID, num_segmento, desplazamiento_segmento, entrada->tamanio);
        devolver_contexto(pcb,SEG_FAULT,parametros);
        return -1;
    } else{
    enviar_op_memoria(pcb,parametros,MOV_IN);
    recv(socket_memoria,registro,tamanio_registro,0);
    
    char* registro_print=malloc(tamanio_registro + 1);
    memcpy(registro_print,registro,tamanio_registro);
    registro_print[tamanio_registro]='\0';

    log_info(logger,"PID: %d - Acción: LEER - Segmento: %d - Dirección Física: %d - Valor: %s",pcb->PID,num_segmento,direccion_fisica,registro_print);     
        
    free(registro_print);
    return 1;
    }
}

int ejecutar_mov_out(t_pcb_cpu* pcb, t_list* parametros){
    int direccion_logica = atoi(list_get(parametros, 0));
    char* registro = encontrar_registro(list_get(parametros,1),pcb);
    int tamanio_registro = encontrar_tamanio_registro(list_get(parametros,1));
    
    int num_segmento = direccion_logica/tam_max_segmento;
    int desplazamiento_segmento = direccion_logica%tam_max_segmento;

    t_entrada_tabla_segmentos* entrada=list_get(pcb->tabla_segmentos, num_segmento);

    int direccion_fisica=entrada->base+desplazamiento_segmento;

    list_remove(parametros,0); //elimino la direccion logica
    list_add(parametros,string_itoa(direccion_fisica));

    char*registro_enviar=malloc(tamanio_registro + 1);
    memcpy(registro_enviar,registro,tamanio_registro);
    registro_enviar[tamanio_registro]='\0';
    
    list_remove(parametros,0); //elimino el registro sin \0
    list_add(parametros,registro_enviar);
     if(tamanio_registro+desplazamiento_segmento>entrada->tamanio){
        log_error(logger, "PID: %d - Error SEG_FAULT - Segmento: %d - Offset: %d - Tamanio: %d", pcb->PID, num_segmento, desplazamiento_segmento+tamanio_registro, entrada->tamanio);
        devolver_contexto(pcb,SEG_FAULT,parametros);
        return -1;
    }else{
    enviar_op_memoria(pcb,parametros,MOV_OUT);
    
    uint32_t cod_ok=200;
    recv(socket_memoria,&cod_ok,sizeof(uint32_t),0);
    
    log_info(logger,"PID: %d - Acción: ESCRIBIR - Segmento: %d - Dirección Física: %d - Valor: %s",(int)pcb->PID,num_segmento,direccion_fisica,registro_enviar);
    return 1;
    }
}

char* encontrar_registro(char* nombre_registro,t_pcb_cpu* pcb){
    char*registro;
    strcmp(nombre_registro,"AX")==0?registro=pcb->registros_cpu->AX:0;
    strcmp(nombre_registro,"BX")==0?registro=pcb->registros_cpu->BX:0;
    strcmp(nombre_registro,"CX")==0?registro=pcb->registros_cpu->CX:0;
    strcmp(nombre_registro,"DX")==0?registro=pcb->registros_cpu->DX:0;
    strcmp(nombre_registro,"EAX")==0?registro=pcb->registros_cpu->EAX:0;
    strcmp(nombre_registro,"EBX")==0?registro=pcb->registros_cpu->EBX:0;
    strcmp(nombre_registro,"ECX")==0?registro=pcb->registros_cpu->ECX:0;
    strcmp(nombre_registro,"EDX")==0?registro=pcb->registros_cpu->EDX:0;
    strcmp(nombre_registro,"RAX")==0?registro=pcb->registros_cpu->RAX:0;
    strcmp(nombre_registro,"RBX")==0?registro=pcb->registros_cpu->RBX:0;
    strcmp(nombre_registro,"RCX")==0?registro=pcb->registros_cpu->RCX:0;
    strcmp(nombre_registro,"RDX")==0?registro=pcb->registros_cpu->RDX:0;
    return registro;  
}

int encontrar_tamanio_registro(char* nombre){
    if(strcmp(nombre,"RAX")<0){
        if (strcmp(nombre,"EAX")<0){
            return 4;
        }
        else{
            return 8;
        }
    }else{
        return 16;
    }
}

void ejecutar_read_o_write(t_pcb_cpu* pcb, t_list* parametros, uint8_t codigo_operacion) {
    int direccion_logica = atoi(list_get(parametros, 1));
    int tamanio_a_leer = atoi(list_get(parametros, 2));
    
    int num_segmento = direccion_logica/tam_max_segmento;//(int) floor((double) direccion_logica/(double) tam_max_segmento);
    int desplazamiento_segmento = direccion_logica%tam_max_segmento;

    t_entrada_tabla_segmentos* entrada=list_get(pcb->tabla_segmentos, num_segmento);

    if(desplazamiento_segmento + tamanio_a_leer > entrada->tamanio) {
        log_error(logger, "PID: %d - Error SEG_FAULT - Segmento: %d - Offset: %d - Tamanio: %d", pcb->PID, num_segmento, desplazamiento_segmento, entrada->tamanio);
        devolver_contexto(pcb, SEG_FAULT, parametros);
    } else {
        int direccion_fisica = entrada->base + desplazamiento_segmento;
        list_remove(parametros, 1); //elimino la direccion logica
        list_add(parametros, string_itoa(direccion_fisica));
        devolver_contexto(pcb, codigo_operacion, parametros);
    }
}

void enviar_op_memoria(t_pcb_cpu* pcb,t_list*lista, uint8_t codigo_operacion){
    int size_stream=sizeof(uint8_t)+2*sizeof(uint32_t)+2;
    for (int i = 0; i < list_size(lista); i++)
    {
        char* elem=list_get(lista,i);
        size_stream+=sizeof(uint32_t);
        size_stream+=strlen(elem)+1;
    }
    void* stream=malloc(size_stream);
    int offset=0;

    memcpy(stream+offset,&codigo_operacion,sizeof(uint8_t));
    offset+=sizeof(uint8_t);
    uint32_t cant_elem=list_size(lista)+1;
    memcpy(stream+offset,&cant_elem,sizeof(uint32_t));
    offset+=sizeof(uint32_t);
    char* pid=string_itoa(pcb->PID);
    uint32_t lenpid=strlen(pid)+1;
    memcpy(stream+offset,&lenpid,sizeof(uint32_t));
    offset+=sizeof(uint32_t);
    memcpy(stream+offset,pid,strlen(pid)+1);
    offset+=lenpid;

    for(int i=0;i<list_size(lista);i++){
        char* elem=list_get(lista,i);
        uint32_t size_elem=strlen(elem)+1;

        memcpy(stream+offset,&size_elem,sizeof(uint32_t));
        offset+=sizeof(uint32_t);
        
        memcpy(stream+offset,elem,size_elem);
        offset+=strlen(elem)+1;
    }
    send(socket_memoria,stream,size_stream,0);
    free(stream);
}