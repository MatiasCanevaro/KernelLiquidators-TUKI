#include <tads/archivos_abiertos.h>

void destruir_lista_archivos_abiertos(t_list* archivos_abiertos){
    //Recordar cambiar la implementacion cuando se tenga la estructura de los archivos abiertos
    list_destroy_and_destroy_elements(archivos_abiertos, free);
}

t_list* deserializar_archivos_abiertos(void* stream){
    //Recordar cambiar la implementacion cuando se tenga la estructura de los archivos abiertos
    t_list* lista_archivos_abiertos=list_create();
    
    return lista_archivos_abiertos;

}

t_buffer* serializar_archivos_abiertos(t_list* archivos_abiertos){
    //Recordar cambiar la implementacion cuando se tenga la estructura de los archivos abiertos
    t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer->size=0;
    buffer->stream=NULL;
    return buffer;
}