#include <tads/tabla_segmentos.h>

void destruir_lista_tabla_segmentos(t_list* tabla_segmentos){
    list_destroy_and_destroy_elements(tabla_segmentos,free);
}

t_buffer* serializar_tabla_segmentos(t_list* tabla_segmentos){
    t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer->size = sizeof(uint32_t)+sizeof(t_entrada_tabla_segmentos)* list_size(tabla_segmentos);
    buffer->stream = malloc(buffer->size);
    uint32_t offset = 0;
    t_entrada_tabla_segmentos* entrada;

    //Copio la cantidad de elementos de la tds
    uint32_t size = list_size(tabla_segmentos);
    memcpy(buffer->stream + offset, &size, sizeof(uint32_t));
    offset+=sizeof(uint32_t);

    //Copio cada entrada
    for(int i = 0; i < list_size(tabla_segmentos); i++){
        entrada=(t_entrada_tabla_segmentos*)list_get(tabla_segmentos, i);
        memcpy(buffer->stream + offset, entrada, sizeof(t_entrada_tabla_segmentos));
        offset += sizeof(t_entrada_tabla_segmentos);
    }
    
    return buffer;
}

t_list* deserializar_tabla_segmentos(void* stream){
    t_list* tabla_segmentos=list_create();
    uint32_t offset = 0;
    uint32_t size;

    memcpy(&size, stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    for(int i=0;i<size;i++){
        t_entrada_tabla_segmentos* entrada=malloc(sizeof(t_entrada_tabla_segmentos));
        memcpy(entrada, stream + offset, sizeof(t_entrada_tabla_segmentos));
        offset += sizeof(t_entrada_tabla_segmentos);
        list_add(tabla_segmentos, entrada);
    }
    return tabla_segmentos;
}