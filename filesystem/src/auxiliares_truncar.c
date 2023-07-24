#include <auxiliares_truncar.h>

void truncar_archivo(t_fcb* fcb,uint32_t nuevo_tamanio){
    // OBTENER EL TAMAÑO ACTUAL DEL ARCHIVO
    uint32_t tamanio_actual = fcb->tamanio_archivo;
    uint32_t bloques_actuales = (fcb->tamanio_archivo/block_size);

    if((fcb->tamanio_archivo%block_size) != 0) bloques_actuales++;
    if(tamanio_actual == 0) bloques_actuales=1;
    
    // NUEVA CANTIDAD DE BLOQUES
    uint32_t bloques_necesarios = (nuevo_tamanio/block_size);
    if((nuevo_tamanio%block_size) != 0) bloques_necesarios++;
    if (nuevo_tamanio == 0) bloques_necesarios=1;

    if (nuevo_tamanio > tamanio_actual){ // AMPLIAR EL TAMAÑO DEL ARCHIVO
        
        // CALCULAR LA CANTIDAD DE BLOQUES ADICIONALES NECESARIOS
        uint32_t bloques_adicionales = bloques_necesarios-bloques_actuales;
        
        // BUSCAR BLOQUES LIBRES EN EL BITMAP
        if(bloques_adicionales > 0){
            asignar_bloques(fcb,bloques_actuales,bloques_adicionales);
        }

    } else if (nuevo_tamanio < tamanio_actual){ // REDUCIR EL TAMAÑO DEL ARCHIVO
        
        // CALCULAR LA CANTIDAD DE BLOQUES A LIBERAR
        uint32_t bloques_a_liberar = bloques_actuales-bloques_necesarios;

        // LIBERAR LOS BLOQUES EXCEDENTES
        liberar_bloques_excedentes(fcb,bloques_actuales,bloques_a_liberar);
    }
    
    // ACTUALIZAR EL TAMAÑO DEL ARCHIVO EN EL FCB
    fcb->tamanio_archivo = nuevo_tamanio;
    config_set_value(fcb->config,"TAMANIO",string_itoa((int) fcb->tamanio_archivo));

    // GUARDAR LOS CAMBIOS EN EL ARCHIVO FCB
    config_save(fcb->config);

    // GUARDAR EL BITMAP
    msync(bitmap,block_count / 8,MS_SYNC);
}

void liberar_bloques_excedentes(t_fcb* fcb,uint32_t bloques_actuales,uint32_t bloques_a_liberar){
    // LIBERAR LOS BLOQUES EXCEDENTES
    for (int i = 0; i < bloques_a_liberar; i++){
        // OBTENER EL BLOQUE DE PUNTEROS
        uint32_t* bloque_punteros = obtener_bloque(fcb->puntero_indirecto);

        // LIBERAR EL ÚLTIMO BLOQUE DE DATOS EN EL BLOQUE DE PUNTEROS
        uint32_t ultimo_bloque = bloque_punteros[bloques_actuales - i - 1];
        desasignar_bloque(ultimo_bloque);

    }

    if(bloques_actuales-bloques_a_liberar == 1){
            if(config_has_property(fcb->config,"PUNTERO_INDIRECTO")){
                config_remove_key(fcb->config,"PUNTERO_INDIRECTO");
                desasignar_bloque(fcb->puntero_indirecto);
            }
        }
        fcb->puntero_indirecto = -1;
    }


void asignar_bloques(t_fcb* fcb, uint32_t bloques_actuales,uint32_t bloques_adicionales){

    // ACTUALIZAR LOS PUNTEROS DIRECTO E INDIRECTO DEL FCB
     if (fcb->puntero_indirecto == -1){
        // OBTENER EL BLOQUE DE PUNTEROS
        fcb->puntero_indirecto = encontrar_bloque_libre();
        config_set_value(fcb->config,"PUNTERO_INDIRECTO",string_itoa(fcb->puntero_indirecto));
    }

    // OBTENER EL BLOQUE DE PUNTEROS
    uint32_t* bloque_punteros = obtener_bloque(fcb->puntero_indirecto);

    for(int i=0;i<bloques_adicionales;i++){
        // OBTENER UN BLOQUE LIBRE
        uint32_t bloque_libre = encontrar_bloque_libre();

        // ASIGNAR EL BLOQUE LIBRE EN EL BLOQUE DE PUNTEROS
        bloque_punteros[bloques_actuales+i] = bloque_libre;
    }
}