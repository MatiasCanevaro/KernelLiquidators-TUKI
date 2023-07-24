#include <tads/pcb.h>

void destruir_pcb(t_pcb* pcb){
    destruir_lista_instrucciones(pcb->instrucciones);
    destruir_lista_tabla_segmentos(pcb->tabla_segmentos);
    temporal_destroy(pcb->tiempo_llegada_ready);
    destruir_lista_archivos_abiertos(pcb->archivos_abiertos);
    free(pcb->registros_cpu);
    free(pcb);
}

t_buffer* serializar_pcb_cpu(t_pcb* pcb) {
    // Se calcula el tamaño del payload sumando el tamaño de cada campo
    uint32_t size = sizeof(uint32_t) + // PID
                    sizeof(uint32_t) + // tamaño de la lista de instrucciones 
                    sizeof(uint32_t) + // program_counter
                    sizeof(t_registros_cpu) + // registros_cpu
                    sizeof(uint32_t) ; // tamaño de la tabla de segmentos

    // Se reserva memoria para el payload
    void* stream = malloc(size);
    uint32_t offset = 0;

    // Se copia el valor de PID en el payload
    memcpy(stream+offset, &(pcb->PID), sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Se copia el valor de program_counter en el payload
    memcpy(stream+offset, &(pcb->program_counter), sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Se copia la estructura t_registros_cpu en el payload
    memcpy(stream+offset, pcb->registros_cpu, sizeof(t_registros_cpu));
    offset +=sizeof(t_registros_cpu);
    
    // Se serializa la lista de instrucciones y se copia en el payload
    t_buffer* instrucciones_serializadas = serializar_instrucciones(pcb->instrucciones);
    memcpy(stream+offset, &(instrucciones_serializadas->size), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    size+=instrucciones_serializadas->size;

    stream = realloc(stream, size);
    memcpy(stream+offset, instrucciones_serializadas->stream, instrucciones_serializadas->size);
    offset += instrucciones_serializadas->size;

    free(instrucciones_serializadas->stream);
    free(instrucciones_serializadas);
    
    // Se serializa la tabla de segmentos y se copia en el payload
    t_buffer* tabla_segmentos_serializada = serializar_tabla_segmentos(pcb->tabla_segmentos);
    memcpy(stream+offset, &(tabla_segmentos_serializada->size), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    size += tabla_segmentos_serializada->size;
    stream = realloc(stream, size);

    memcpy(stream+offset, tabla_segmentos_serializada->stream, tabla_segmentos_serializada->size);
    offset += tabla_segmentos_serializada->size;

    free(tabla_segmentos_serializada->stream);
    free(tabla_segmentos_serializada);

    // Se crea la estructura t_buffer y se asigna el tamaño y dirección de memoria del payload
    t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer->size = size;
    buffer->stream = stream;
        

    // Se retorna la estructura t_buffer
    return buffer;
}

t_pcb_cpu* deserializar_pcb_cpu(void* stream) {
    t_pcb_cpu* pcb = malloc(sizeof(t_pcb_cpu));

    int offset = 0;

    // Deserializamos el PID
    memcpy(&(pcb->PID), stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Deserializamos el program counter
    memcpy(&(pcb->program_counter), stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Deserializamos los registros de CPU
    pcb->registros_cpu = malloc(sizeof(t_registros_cpu));
    memcpy(pcb->registros_cpu, stream + offset, sizeof(t_registros_cpu));
    offset += sizeof(t_registros_cpu);

    // Deserializamos las instrucciones
    uint32_t size_instrucciones;
    memcpy(&size_instrucciones,stream+offset,sizeof(uint32_t));
    offset += sizeof(uint32_t);
    
    pcb->instrucciones = deserializar_instrucciones(stream + offset);
    offset += size_instrucciones;

    // Deserializamos la tabla de segmentos
    uint32_t size_tds;
    memcpy(&size_tds, stream + offset, sizeof(uint32_t));
    offset+=sizeof(uint32_t);
    pcb->tabla_segmentos = deserializar_tabla_segmentos(stream + offset);
    offset+=size_tds;

    return pcb;
}

t_buffer* serializar_pcb_kernel(t_pcb_cpu* pcb) {
    uint32_t size = sizeof(uint32_t) + // program_counter
                    sizeof(t_registros_cpu); // registros_cpu
                   
    // Se reserva memoria para el payload
    void* stream = malloc(size);
    uint32_t offset = 0;

    // Se copia el valor de program_counter en el payload
    memcpy(stream+offset, &(pcb->program_counter), sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Se copia la estructura t_registros_cpu en el payload
    memcpy(stream+offset, pcb->registros_cpu, sizeof(t_registros_cpu));
    offset +=sizeof(t_registros_cpu);

    // Se crea la estructura t_buffer y se asigna el tamaño y dirección de memoria del payload
    t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer->size = size;
    buffer->stream = stream;
        

    // Se retorna la estructura t_buffer
    return buffer;
}

t_pcb_kernel* deserializar_pcb_kernel(void* stream) {
    t_pcb_kernel* pcb = malloc(sizeof(t_pcb_kernel));

    int offset = 0;

    // Deserializamos el program counter
    memcpy(&(pcb->program_counter), stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Deserializamos los registros de CPU
    pcb->registros_cpu = malloc(sizeof(t_registros_cpu));
    memcpy(pcb->registros_cpu, stream + offset, sizeof(t_registros_cpu));
    offset += sizeof(t_registros_cpu);

    return pcb;
}


int enviar_paquete_pcb(t_buffer* buffer_pcb, int socket_destino) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = PCB;
    paquete->buffer = buffer_pcb;

    int resultado_envio = enviar_paquete(socket_destino, paquete);

    if (resultado_envio == -1) {
        printf("Error al enviar el paquete");
        return -1;
    }
    return 0;
}

t_pcb_cpu* recibir_paquete_pcb_cpu(int socket_origen) {
    
    // Recibimos el paquete
    t_paquete* paquete = recibir_paquete(socket_origen);

    if (!paquete) {
        printf("Error al recibir el paquete");
        return NULL;
    }   
    // Verificamos el código de operación
    if (paquete->codigo_operacion != PCB) {
        printf("El paquete recibido no es de pcb\n");
        free(paquete->buffer->stream);
        free(paquete->buffer);
        free(paquete);
        return NULL;
    }
    //Deserializamos el pcb
    t_pcb_cpu* pcb = deserializar_pcb_cpu(paquete->buffer->stream);

    if (!pcb) {
        printf("Error al deserializar pcb\n");
        return NULL;
    }
    
    // Liberamos memoria
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);

    return pcb;
}

t_pcb_kernel* recibir_paquete_pcb_kernel(int socket_origen) {
    
    // Recibimos el paquete
    t_paquete* paquete = recibir_paquete(socket_origen);

    if (!paquete) {
        printf("Error al recibir el paquete");
        return NULL;
    }   
    // Verificamos el código de operación
    if (paquete->codigo_operacion != PCB) {
        printf("El paquete recibido no es de pcb\n");
        free(paquete->buffer->stream);
        free(paquete->buffer);
        free(paquete);
        return NULL;
    }
    //Deserializamos el pcb
    t_pcb_kernel* pcb = deserializar_pcb_kernel(paquete->buffer->stream);

    if (!pcb) {
        printf("Error al deserializar pcb\n");
        return NULL;
    }
    
    // Liberamos memoria
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);

    return pcb;
}