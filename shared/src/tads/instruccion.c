#include <tads/instruccion.h>




uint32_t convertir_instruccion_a_entero(char* instruccion){
    if(!strcmp(instruccion,"F_READ")){return F_READ;}
    if(!strcmp(instruccion,"F_WRITE")) return F_WRITE;
    if(!strcmp(instruccion,"SET")) return SET;
    if(!strcmp(instruccion,"MOV_IN")) return MOV_IN;
    if(!strcmp(instruccion,"MOV_OUT")) return MOV_OUT;
    if(!strcmp(instruccion,"F_TRUNCATE")) return F_TRUNCATE;
    if(!strcmp(instruccion,"F_SEEK")) return F_SEEK;
    if(!strcmp(instruccion,"CREATE_SEGMENT")) return CREATE_SEGMENT;
    if(!strcmp(instruccion,"EXIT")) return EXIT;
    if(!strcmp(instruccion,"YIELD")) return YIELD;
    if(!strcmp(instruccion,"I/O")) return IO;
    if(!strcmp(instruccion,"WAIT")) return WAIT;
    if(!strcmp(instruccion,"SIGNAL")) return SIGNAL;
    if(!strcmp(instruccion,"F_OPEN")) return F_OPEN;
    if(!strcmp(instruccion,"F_CLOSE")) return F_CLOSE;
    if(!strcmp(instruccion,"DELETE_SEGMENT")) return DELETE_SEGMENT;

    return I_NORECONOCIDA;
}
void imprimir_parametro(void* parametro) {
    printf("%s ", (char*) parametro);
}

void imprimir_instruccion(t_instruccion* instruccion) {
    printf("Codigo: %d\n", instruccion->codigo);
    printf("Parametros: ");
    list_iterate(instruccion->parametros, (void*) imprimir_parametro);
    printf("\n");
}

void imprimir_instrucciones(t_list* lista) {
    printf("Instrucciones:\n");
    list_iterate(lista, (void*) imprimir_instruccion);
}

t_instruccion* crear_instruccion(){
    t_instruccion* instruccion=malloc(sizeof(t_instruccion));
    instruccion->parametros=list_create();
    instruccion->codigo=NULL;
    return instruccion;
}

void destruir_instruccion(t_instruccion* instruccion){
    list_destroy_and_destroy_elements(instruccion->parametros,free);
    free(instruccion);
}

void destruir_lista_instrucciones(t_list* lista_instrucciones){
    list_destroy_and_destroy_elements(lista_instrucciones,(void *)destruir_instruccion);
}

t_buffer* serializar_instrucciones(t_list* lista_instrucciones) {
    t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer->size = 0;
    buffer->stream = NULL;

    uint32_t cantidad_instrucciones = list_size(lista_instrucciones);
    buffer->size += sizeof(uint32_t);

    for(int i = 0; i < cantidad_instrucciones; i++) {
        t_instruccion* instruccion = list_get(lista_instrucciones, i);
        buffer->size += sizeof(uint32_t); // tamaño del codigo

        t_list* parametros = instruccion->parametros;
        uint32_t cantidad_parametros = list_size(parametros);
        buffer->size += sizeof(uint32_t); // tamaño de la lista de parametros

        for(int j = 0; j < cantidad_parametros; j++) {
            char* parametro = list_get(parametros, j);
            uint32_t tamanio_parametro = strlen(parametro) + 1; // tamaño del parametro +1 por el caracter nulo
            buffer->size += sizeof(uint32_t); // tamaño del parametro
            buffer->size += tamanio_parametro; // tamaño real del parametro
        }
    }

    void* stream = malloc(buffer->size);
    uint32_t offset = 0;

    memcpy(stream + offset, &cantidad_instrucciones, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    for(int i = 0; i < cantidad_instrucciones; i++) {
        t_instruccion* instruccion = list_get(lista_instrucciones, i);
        uint32_t codigo = instruccion->codigo;
        memcpy(stream + offset, &codigo, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        t_list* parametros = instruccion->parametros;
        uint32_t cantidad_parametros = list_size(parametros);
        memcpy(stream + offset, &cantidad_parametros, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        for(int j = 0; j < cantidad_parametros; j++) {
            char* parametro = list_get(parametros, j);
            uint32_t tamanio_parametro = strlen(parametro) + 1; // tamaño del parametro +1 por el caracter nulo
            memcpy(stream + offset, &tamanio_parametro, sizeof(uint32_t));
            offset += sizeof(uint32_t);

            memcpy(stream + offset, parametro, tamanio_parametro);
            offset += tamanio_parametro;
        }
    }

    buffer->stream = stream;
    return buffer;
}

t_list* deserializar_instrucciones(void* stream) {
    t_list* lista = list_create();

    uint32_t size;
    memcpy(&size, stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    for (int i = 0; i < size; i++) {
        t_instruccion* instruccion = malloc(sizeof(t_instruccion));

        memcpy(&instruccion->codigo, stream, sizeof(uint32_t));
        stream += sizeof(uint32_t);

        t_list* parametros = list_create();
        uint32_t parametros_size;
        memcpy(&parametros_size, stream, sizeof(uint32_t));
        stream += sizeof(uint32_t);

        for (int j = 0; j < parametros_size; j++) {
            uint32_t parametro_size;
            memcpy(&parametro_size, stream, sizeof(uint32_t));
            stream += sizeof(uint32_t);

            char* parametro = malloc(parametro_size + 1);
            memcpy(parametro, stream, parametro_size);
            parametro[parametro_size] = '\0';
            stream += parametro_size;

            list_add(parametros, parametro);
        }

        instruccion->parametros = parametros;
        list_add(lista, instruccion);
    }

    return lista;
}

void enviar_paquete_instrucciones(int socket_destino, t_list* instrucciones) {
    // Serializamos la lista de instrucciones
    t_buffer* buffer = serializar_instrucciones(instrucciones);
    if (buffer == NULL) {
        printf("Error al serializar las instrucciones\n");
        return;
    }

    // Creamos el paquete a enviar
    t_paquete* paquete = malloc(sizeof(t_paquete));
    if (paquete == NULL) {
        printf("Error al crear el paquete\n");
        return;
    }
    paquete->codigo_operacion = INSTRUCCIONES; // Código de operación para instrucciones
    paquete->buffer = buffer;

    // Enviamos el paquete
    if (enviar_paquete(socket_destino, paquete) == -1) {
        printf("Error al enviar el paquete\n");
        return;
    }
}

t_list* recibir_paquete_instrucciones(int socket_origen) {
    // Recibimos el paquete
    t_paquete* paquete = recibir_paquete(socket_origen);
    if (paquete == NULL) {
        printf("Error al recibir el paquete\n");
        return NULL;
    }

    // Verificamos el código de operación
    if (paquete->codigo_operacion != INSTRUCCIONES) {
        printf("El paquete recibido no es de instrucciones\n");
        free(paquete->buffer->stream);
        free(paquete->buffer);
        free(paquete);
        return NULL;
    }

    // Deserializamos la lista de instrucciones
    t_list* instrucciones = deserializar_instrucciones(paquete->buffer->stream);

    // Liberamos memoria
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);

    return instrucciones;
}
