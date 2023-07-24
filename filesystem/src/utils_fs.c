#include <utils_fs.h>

void* obtener_bloque(uint32_t numero_bloque){
    void* bloque = (void*)(archivo_bloques + numero_bloque * block_size);
    return bloque;
}

void desasignar_bloque(uint32_t bloque){
    bitarray_clean_bit(bitarray,bloque);
    log_info(logger,"Acceso a Bitmap - Bloque: %d - Estado: 0",bloque);
}

uint32_t encontrar_bloque_libre(){
    for (int i=0;i < block_count;i++){
        if (!bitarray_test_bit(bitarray,i)){
            bitarray_set_bit(bitarray,i); // MARCAR EL BLOQUE COMO ASIGNADO
            log_info(logger,"Acceso a Bitmap - Bloque: %d - Estado: 1",i);
            return i; // RETORNAR EL NÚMERO DEL BLOQUE ASIGNADO
        }
    }

    return -1; // EN CASO DE NO ENCONTRAR BLOQUES LIBRES
}

char* obtener_contenido_archivo(t_fcb* fcb,uint32_t posicion,uint32_t tamanio_a_leer,char* nombre_archivo){
    uint32_t bytes_leidos = 0;
    uint32_t bloque_actual;
    uint32_t num_bloque_actual = posicion / block_size;
    uint32_t offset_inicial = posicion % (uint32_t) block_size;
    
    char* contenido = malloc(tamanio_a_leer)+1;
    
    if(posicion < block_size){
        bloque_actual = fcb->puntero_directo;
    } else {
        // POSICION EN EL PUNTERO INDIRECTO
        uint32_t bloque_indirecto = fcb->puntero_indirecto;

        uint32_t bloque_indirecto_actual = num_bloque_actual - 1;

        uint32_t* puntero_bloque_indirecto = (uint32_t*)obtener_bloque(bloque_indirecto);

        bloque_actual = puntero_bloque_indirecto[bloque_indirecto_actual];
        
    }

    //LEO PRIMER BLOQUE
    if(tamanio_a_leer < block_size-offset_inicial){
        memcpy(contenido,obtener_bloque(bloque_actual)+offset_inicial,tamanio_a_leer);
        log_info(logger,"Acceso Bloque - Archivo: %s - Bloque Archivo: %d - Bloque File System %d",nombre_archivo,bloque_actual,num_bloque_actual);
        bytes_leidos = tamanio_a_leer;
    } else{
        memcpy(contenido,obtener_bloque(bloque_actual)+offset_inicial,block_size-offset_inicial);
        log_info(logger,"Acceso Bloque - Archivo: %s - Bloque Archivo: %d - Bloque File System %d",nombre_archivo,bloque_actual,num_bloque_actual);
        bytes_leidos = block_size-offset_inicial;
    }

    //LEO EL RESTO DE LOS BLOQUES
    while (bytes_leidos < tamanio_a_leer){
        bloque_actual = obtener_bloque_siguiente(fcb,num_bloque_actual);

        if(tamanio_a_leer < block_size){
            memcpy(contenido+tamanio_a_leer-bytes_leidos,obtener_bloque(bloque_actual),tamanio_a_leer-bytes_leidos);
            log_info(logger,"Acceso Bloque - Archivo: %s - Bloque Archivo: %d - Bloque File System %d",nombre_archivo,bloque_actual,num_bloque_actual);
            bytes_leidos = tamanio_a_leer;
        } else{
            memcpy(contenido+tamanio_a_leer-bytes_leidos,obtener_bloque(bloque_actual),block_size);
            log_info(logger,"Acceso Bloque - Archivo: %s - Bloque Archivo: %d - Bloque File System %d",nombre_archivo,bloque_actual,num_bloque_actual);
            bytes_leidos += block_size;
        }
        num_bloque_actual++;
    }

    contenido[tamanio_a_leer] = '\0'; // AGREGO \0 AL FINAL DEL CONTENIDO

    msync(bitmap,block_count / 8,MS_SYNC);
    msync(archivo_bloques,block_size * block_count,MS_SYNC);

    return contenido;
}

void escribir_archivo_bloques(t_fcb* fcb,uint32_t posicion,uint32_t tamanio_a_escribir,char* contenido,char* nombre_archivo){
    uint32_t bytes_escritos = 0;
    uint32_t bloque_actual;
    uint32_t num_bloque_actual = posicion / block_size;
    uint32_t offset_inicial = posicion % (uint32_t) block_size;
    
    if(posicion<block_size){
        bloque_actual = fcb->puntero_directo;
    } else {
        // POSICION EN EL PUNTERO INDIRECTO
        uint32_t bloque_indirecto = fcb->puntero_indirecto;

        uint32_t bloque_indirecto_actual = num_bloque_actual - 1;

        uint32_t* puntero_bloque_indirecto = (uint32_t*)obtener_bloque(bloque_indirecto);

        bloque_actual = puntero_bloque_indirecto[bloque_indirecto_actual];
        
    }
    //COPIO EL PRIMER BLOQUE
    if(tamanio_a_escribir < block_size-offset_inicial){
        memcpy(obtener_bloque(bloque_actual)+offset_inicial,contenido,tamanio_a_escribir);
        log_info(logger,"Acceso Bloque - Archivo: %s - Bloque Archivo: %d - Bloque File System %d",nombre_archivo,bloque_actual,num_bloque_actual);
        bytes_escritos = tamanio_a_escribir;
    } else{
        memcpy(obtener_bloque(bloque_actual)+offset_inicial,contenido,block_size-offset_inicial);
        log_info(logger,"Acceso Bloque - Archivo: %s - Bloque Archivo: %d - Bloque File System %d",nombre_archivo,bloque_actual,num_bloque_actual);
        bytes_escritos = block_size-offset_inicial;
    }

    //COPIO EL RESTO DE LOS BLOQUES
    while (bytes_escritos < tamanio_a_escribir){
        bloque_actual = obtener_bloque_siguiente(fcb,num_bloque_actual);

        if(tamanio_a_escribir<block_size){
            memcpy(obtener_bloque(bloque_actual),contenido+bytes_escritos,tamanio_a_escribir-bytes_escritos);
            log_info(logger,"Acceso Bloque - Archivo: %s - Bloque Archivo: %d - Bloque File System %d",nombre_archivo,bloque_actual,num_bloque_actual);
            bytes_escritos = tamanio_a_escribir;
        } else{
            memcpy(obtener_bloque(bloque_actual),contenido+bytes_escritos,block_size);
            log_info(logger,"Acceso Bloque - Archivo: %s - Bloque Archivo: %d - Bloque File System %d",nombre_archivo,bloque_actual,num_bloque_actual);
            bytes_escritos += block_size;
        }
        num_bloque_actual++;
    }

    msync(bitmap,block_count / 8,MS_SYNC);
    msync(archivo_bloques,block_size * block_count,MS_SYNC);
}

uint32_t obtener_bloque_siguiente(t_fcb* fcb,uint32_t num_bloque_actual){
    uint32_t* puntero_bloque_indirecto = (uint32_t*)obtener_bloque(fcb->puntero_indirecto);
    return puntero_bloque_indirecto[num_bloque_actual];
}

void enviar_op_con_parametros(uint8_t motivo,t_list* lista_parametros,int socket_fd){
    uint32_t cant_parametros = (uint32_t) list_size(lista_parametros);
    int size = sizeof(uint8_t)+sizeof(uint32_t);

    for(int i=0;i<list_size(lista_parametros);i++){
        char* parametro = list_get(lista_parametros,i);
        size += sizeof(uint32_t);
        size += strlen(parametro)+1;
    }

    void* stream_a_enviar = malloc(size);
    int offset = 0;

    memcpy(stream_a_enviar+offset,&motivo,sizeof(uint8_t));
    offset += sizeof(uint8_t);

    memcpy(stream_a_enviar+offset,&cant_parametros,sizeof(uint32_t));
    offset += sizeof(uint32_t);
    
    for(int i=0;i<list_size(lista_parametros);i++){
        char* parametro = list_get(lista_parametros,i);
        uint32_t param_size = strlen(parametro)+1;
        memcpy(stream_a_enviar+offset,&param_size,sizeof(uint32_t));
        offset += sizeof(uint32_t);
        memcpy(stream_a_enviar+offset,parametro,param_size);
        offset += param_size;
    }
    
    send(socket_memoria,stream_a_enviar,size,0);
}