#include <hilo_kernel.h>

void* esperar_kernel(){
    while(1){
        uint8_t cod_op;
        recv(socket_kernel,&cod_op,sizeof(uint8_t),0);
        t_list* parametros_kernel = list_create();
        recibir_parametros_kernel(parametros_kernel);
        
        switch (cod_op){
            case CREAR_PROCESO:
                log_info(logger, "Creación de Proceso PID: %d",atoi(list_get(parametros_kernel,0)));
                crear_tabla_segmentos(list_get(parametros_kernel,0));
                break;
            
            case ELIMINAR_PROCESO:
                log_info(logger, "Eliminación de Proceso PID: %d",atoi(list_get(parametros_kernel,0)));
                liberar_lista_tabla_segmentos_y_sus_entradas(list_get(parametros_kernel,0));
                break;

            case CREATE_SEGMENT: 
                //Recibe (pid, sid, tamanio)
                administrar_create_segment((list_get(parametros_kernel,0)),atoi(list_get(parametros_kernel,1)),atoi(list_get(parametros_kernel,2)));
                break;

            case DELETE_SEGMENT: 
                //Recibe (pid, sid)
                administrar_delete_segment((list_get(parametros_kernel,0)),atoi(list_get(parametros_kernel,1)));
                break;
            case COMPACTAR:
                log_info(logger, "Solicitud de Compactación");
                administrar_compactacion();
                break;
            default:
                log_warning(logger, "Operación desconocida");
                break;
        }
        list_destroy_and_destroy_elements(parametros_kernel,free);
    }
}

void recibir_parametros_kernel(t_list* parametros_kernel){
    int ok=1;
    uint32_t list_size;
    recv(socket_kernel,&list_size,sizeof(uint32_t),MSG_WAITALL);

    for(int i=0;i<list_size;i++) {
        uint32_t param_size;
        recv(socket_kernel,&param_size,sizeof(uint32_t),MSG_WAITALL);

        char* param = malloc(param_size);
        ok = recv(socket_kernel,param,param_size,MSG_WAITALL);

        list_add(parametros_kernel, param);
    } 
    if(ok<=0){
        log_error(logger,"Memoria desconectada");
        finalizar_memoria();
        return 0;
    }
}

void crear_tabla_segmentos(char* pid){
    t_list* lista_tabla_segmentos = list_create();

    list_add(lista_tabla_segmentos,segmento_cero);//El primer elemento es el segmento_cero
    for(int i=1;i<cant_segmentos;i++){
        t_entrada_tabla_segmentos* entrada = malloc(sizeof(t_entrada_tabla_segmentos));
        entrada->pid = atoi(pid);
        entrada->sid = i;
        entrada->base = 0;
        entrada->tamanio = 0;
        list_add(lista_tabla_segmentos,entrada);
    }

    dictionary_put(tabla_general,pid,lista_tabla_segmentos);
    
    int buffer_size = cant_segmentos * sizeof(t_entrada_tabla_segmentos) + sizeof(uint32_t);
    void* buffer = malloc(buffer_size);
    int offset = 0;
    memcpy(buffer + offset,&cant_segmentos,sizeof(uint32_t));
    offset += sizeof(uint32_t);
    
    for(int i=0;i<cant_segmentos;i++){
        t_entrada_tabla_segmentos* entrada = list_get(lista_tabla_segmentos,i);
        memcpy(buffer + offset,entrada,sizeof(t_entrada_tabla_segmentos));
        offset += sizeof(t_entrada_tabla_segmentos);
    }

    send(socket_kernel,buffer,buffer_size,NULL);
    free(buffer);
}

void liberar_lista_tabla_segmentos_y_sus_entradas(char* pid){
    t_list* lista_tabla_segmentos = dictionary_remove(tabla_general,pid);
    for(int i=1;i<cant_segmentos;i++){
        t_entrada_tabla_segmentos* entrada = list_get(lista_tabla_segmentos,i);
        if(entrada->tamanio != 0){
            log_info(logger, "PID: %d - Eliminar Segmento: %d - Base: %d - TAMAÑO: %d",atoi(pid),entrada->sid,entrada->base,entrada->tamanio);
            consolidar(entrada->base,entrada->tamanio);
        }
        free(entrada);
    }
    list_destroy(lista_tabla_segmentos);
    enviar_ok_al_kernel();
}

void enviar_ok_al_kernel(){
    uint32_t ok = 200;
    send(socket_kernel,&ok,sizeof(uint32_t),0);
}

void administrar_delete_segment(char* pid, uint32_t sid){
    t_entrada_tabla_segmentos* entrada = obtener_entrada(pid,sid);
    log_info(logger, "PID: %d - Eliminar Segmento: %d - Base: %d - TAMAÑO: %d",atoi(pid),sid,entrada->base,entrada->tamanio);
    consolidar(entrada->base,entrada->tamanio);
    entrada->base = 0;
    entrada->tamanio = 0;
    enviar_ok_al_kernel();
}

void consolidar(uint32_t base, uint32_t tamanio){
    t_hueco_libre* hueco_izq = obtener_hueco_libre_a_la_izquierda(base);
    t_hueco_libre* hueco_der = obtener_hueco_libre_a_la_derecha(base,tamanio);
    if(hueco_izq!=NULL && hueco_der!=NULL){
        hueco_izq->tamanio += tamanio + hueco_der->tamanio;
        list_remove_and_destroy_element(lista_huecos_libres, obtener_posicion_hueco(hueco_der), free);
    }
    else if(hueco_izq!=NULL){
        hueco_izq->tamanio += tamanio;
    }
    else if(hueco_der!=NULL){
        hueco_der->base = base;
        hueco_der->tamanio += tamanio;
    }
    else {
        t_hueco_libre* hueco = malloc(sizeof(t_hueco_libre));
        hueco->base = base;
        hueco->tamanio = tamanio;
        //inserta el hueco en lista_huecos_libres ordenado segun base de menor a mayor
        list_add_sorted(lista_huecos_libres, hueco, comparar_bases_huecos);
    }
}

t_hueco_libre* obtener_hueco_libre_a_la_izquierda(uint32_t base){
    for(int i=0;i<list_size(lista_huecos_libres);i++){
        t_hueco_libre* hueco = list_get(lista_huecos_libres,i);
        if((hueco->base + hueco->tamanio) == base){
            return hueco;
        }
    }
    return NULL;
}

t_hueco_libre* obtener_hueco_libre_a_la_derecha(uint32_t base, uint32_t tamanio){
    for(int i=0;i<list_size(lista_huecos_libres);i++){
        t_hueco_libre* hueco = list_get(lista_huecos_libres,i);
        if(hueco->base == (base+tamanio)){
            return hueco;
        }
    }
    return NULL;
}

uint32_t comparar_bases_huecos(t_hueco_libre* hueco1, t_hueco_libre* hueco2) {
    return hueco1->base < hueco2->base;
}

void administrar_create_segment(char* pid, uint32_t sid_a_crear, uint32_t tamanio_segmento_a_crear){
    //SI HAY ESPACIO CONTIGUO
    if(algun_hueco_tiene_tamanio_suficiente(tamanio_segmento_a_crear)){
        t_entrada_tabla_segmentos* entrada = obtener_entrada(pid,sid_a_crear);
        uint32_t df_base = obtener_df_base(tamanio_segmento_a_crear);
        entrada->base = df_base;
        entrada->tamanio = tamanio_segmento_a_crear;
        log_info(logger, "PID: %d - Crear Segmento: %d - Base: %d - TAMAÑO: %d",atoi(pid),sid_a_crear,df_base,tamanio_segmento_a_crear);
        reducir_o_borrar_hueco(df_base, tamanio_segmento_a_crear);
        enviar_df_al_kernel(df_base);
    }
    //SI HAY ESPACIO TOTAL PERO NO CONTIGUO
    else if(espacioTotal() >= tamanio_segmento_a_crear){
        uint32_t hay_espacio=HAY_ESPACIO_TOTAL_NO_CONTIGUO;
        send(socket_kernel,&hay_espacio,sizeof(uint32_t),NULL);        
    }
    //SI NO HAY ESPACIO
    else {
        uint32_t no_hay_espacio=NO_HAY_ESPACIO_SUFICIENTE;
        send(socket_kernel,&no_hay_espacio,sizeof(uint32_t),NULL);
    }
}

uint32_t algun_hueco_tiene_tamanio_suficiente(uint32_t tamanio_segmento_a_crear){
    for(int i=0;i<list_size(lista_huecos_libres);i++){
        t_hueco_libre* hueco = list_get(lista_huecos_libres, i);
        if(hueco->tamanio >= tamanio_segmento_a_crear){
            return 1;
        }
    }
    return 0;
}

t_entrada_tabla_segmentos* obtener_entrada(char* pid_buscado,uint32_t sid_a_crear){
    t_list* lista_tabla_segmentos = dictionary_get(tabla_general, pid_buscado);
    return list_get(lista_tabla_segmentos,sid_a_crear);
}

void reducir_o_borrar_hueco(uint32_t df_base_hueco,uint32_t tamanio_segmento_a_crear){
    t_hueco_libre* hueco = obtener_hueco_de_base(df_base_hueco);
    if(hueco->tamanio > tamanio_segmento_a_crear){
        hueco->base += tamanio_segmento_a_crear;
        hueco->tamanio -= tamanio_segmento_a_crear;
    }
    else if(hueco->tamanio == tamanio_segmento_a_crear){
        int pos_hueco = obtener_posicion_hueco(hueco);
        list_remove_and_destroy_element(lista_huecos_libres, pos_hueco, free);
    }
}

void enviar_df_al_kernel(uint32_t df_base){
    void* buffer = malloc(sizeof(uint32_t)*2);
    uint32_t ok = 200;
    memcpy(buffer,&ok,sizeof(uint32_t));
    int offset = sizeof(uint32_t);
    memcpy(buffer+offset,&df_base,sizeof(uint32_t));
    send(socket_kernel,buffer,sizeof(uint32_t)*2,NULL);
    free(buffer);
}

t_hueco_libre* obtener_hueco_de_base(uint32_t df_base_hueco){
    for(int i=0;i<list_size(lista_huecos_libres);i++){
        t_hueco_libre* hueco = list_get(lista_huecos_libres, i);
        if(hueco->base == df_base_hueco){
            return hueco;
        }
    }
}

int obtener_posicion_hueco(t_hueco_libre* hueco){
    for(int i=0;i<list_size(lista_huecos_libres);i++){
        if(hueco == list_get(lista_huecos_libres,i)){
            return i;
        }
    }
}

uint32_t obtener_df_base(uint32_t tamanio_segmento_a_crear){
    if(strcmp(algoritmo_asignacion, "FIRST") == 0){
        t_hueco_libre* resultado = algoritmo_first(tamanio_segmento_a_crear);
        return resultado->base;
    }
    else if(strcmp(algoritmo_asignacion, "BEST") == 0){
        t_list* lista_filtrada = list_create();
        filtrar_lista(lista_filtrada,tamanio_segmento_a_crear); //agrego huecos con tamanio >= tamanio_segmento_buscado
        t_hueco_libre* resultado = algoritmo_best(lista_filtrada);
        list_destroy(lista_filtrada);
        return resultado->base;
    }
    else if(strcmp(algoritmo_asignacion, "WORST") == 0){
        t_list* lista_filtrada = list_create();
        filtrar_lista(lista_filtrada,tamanio_segmento_a_crear); //agrego huecos con tamanio >= tamanio_segmento_buscado
        t_hueco_libre* resultado = algoritmo_worst(lista_filtrada);
        list_destroy(lista_filtrada);
        return resultado->base;
    }
}

//precondicion: la lista_huecos_libres debe estar ordenada segun su base
t_hueco_libre* algoritmo_first(uint32_t tamanio_segmento_a_crear){
    for(int i=0;i<list_size(lista_huecos_libres);i++){
        t_hueco_libre* hueco = list_get(lista_huecos_libres, i);
        if(hueco->tamanio >= tamanio_segmento_a_crear){
            return hueco;
        }
    }
}

void filtrar_lista(t_list* lista_filtrada,uint32_t tamanio_segmento_a_crear){
    for(int i=0;i<list_size(lista_huecos_libres);i++){
        t_hueco_libre* hueco_actual = list_get(lista_huecos_libres,i);
        if(hueco_actual->tamanio >= tamanio_segmento_a_crear){
            list_add(lista_filtrada,hueco_actual);
        }
    }
}

//precondicion: la lista_filtrada debe estar ordenada segun su base y los huecos deben cumplir: hueco->tamanio >= tamanio_segmento_a_crear
t_hueco_libre* algoritmo_best(t_list* lista_filtrada){
    t_hueco_libre* hueco_con_menor_tamanio = list_get(lista_filtrada, 0);
    for(int i=1;i<list_size(lista_filtrada);i++){
        t_hueco_libre* hueco_actual = list_get(lista_filtrada, i);
        if(hueco_actual->tamanio < hueco_con_menor_tamanio->tamanio){
            hueco_con_menor_tamanio = hueco_actual;
        }
    }
    return hueco_con_menor_tamanio;
}

//precondicion: la lista_filtrada debe estar ordenada segun su base y los huecos deben cumplir: hueco->tamanio >= tamanio_segmento_a_crear
t_hueco_libre* algoritmo_worst(t_list* lista_filtrada){
    t_hueco_libre* hueco_con_mayor_tamanio = list_get(lista_filtrada, 0);
    for(int i=1;i<list_size(lista_filtrada);i++){
        t_hueco_libre* hueco_actual = list_get(lista_filtrada, i);
        if(hueco_actual->tamanio > hueco_con_mayor_tamanio->tamanio){
            hueco_con_mayor_tamanio = hueco_actual;
        }
    }
    return hueco_con_mayor_tamanio;
}

uint32_t espacioTotal(){
    uint32_t espacioTotal = 0;
    for(int i=0;i<list_size(lista_huecos_libres);i++){
        t_hueco_libre* hueco = list_get(lista_huecos_libres, i);
        espacioTotal += hueco->tamanio;
    }
    return espacioTotal;
}

//====================================COMPACTACION===============================================
void administrar_compactacion(){
    t_list* lista_temporal = list_create();
    
    uint32_t base_nuevo_hueco = 0;
    uint32_t tamanio_total_entradas = 0;

    agregar_entradas(lista_temporal);
    
    pthread_mutex_lock(&mutex_memoria);
    compactar_entradas(lista_temporal,&base_nuevo_hueco,&tamanio_total_entradas);
    pthread_mutex_unlock(&mutex_memoria);

    list_destroy(lista_temporal);

    borrar_todos_los_huecos_libres();
    crear_nuevo_hueco(base_nuevo_hueco,tamanio_total_entradas);

    usleep(retardo_compactacion * 1000);
    enviar_claves_y_listas_tabla_segmentos();
}

uint32_t comparar_bases_entradas(t_entrada_tabla_segmentos* entrada1, t_entrada_tabla_segmentos* entrada2) {
    return entrada1->base < entrada2->base;
}

//Se agregan todas las entradas de todas las tablas de segmentos de la tabla general 
//ordenadas por base (salvo por segmento_cero y las que tienen tamaño 0)
void agregar_entradas(t_list* lista_temporal){
    t_list* lista_general = dictionary_elements(tabla_general);//Retorna todos los elementos de la tabla_general
    for(int i=0;i<list_size(lista_general);i++){
       t_list* lista_tabla_segmentos = list_get(lista_general,i);
        for(int j=1;j<cant_segmentos;j++){
            t_entrada_tabla_segmentos* entrada = list_get(lista_tabla_segmentos,j);
            if(entrada->tamanio!=0){
                list_add_sorted(lista_temporal,entrada,comparar_bases_entradas);
            }
        }
    } 
}

void compactar_entradas(t_list* lista_temporal, uint32_t* base_nuevo_hueco, uint32_t* tamanio_total_entradas){
    t_entrada_tabla_segmentos* entrada_anterior = list_get(lista_temporal,0);
    //===============================LEER=================================
    char* valor_leido = malloc(entrada_anterior->tamanio);
    memcpy(valor_leido,(memoria + entrada_anterior->base),(entrada_anterior->tamanio));//destino,origen,tamaño
    //===============================LEER=================================
    entrada_anterior->base = tam_segmento_cero;
    *tamanio_total_entradas += entrada_anterior->tamanio;
    *base_nuevo_hueco = entrada_anterior->base + entrada_anterior->tamanio;//Por si entrada_anterior es la única entrada
    //===============================ESCRIBIR=============================
    memcpy((memoria + entrada_anterior->base),valor_leido,(entrada_anterior->tamanio));//destino,origen,tamaño
    free(valor_leido);
    //===============================ESCRIBIR=============================
    log_info(logger, "PID: %d - Segmento: %d - Base: %d - TAMAÑO: %d",entrada_anterior->pid,entrada_anterior->sid,entrada_anterior->base,entrada_anterior->tamanio);
    uint32_t tamanio_lista_temporal = list_size(lista_temporal);
    for(int i=1;i<tamanio_lista_temporal;i++){
        t_entrada_tabla_segmentos* entrada_actual = list_get(lista_temporal, i);
        //===============================LEER=================================
        valor_leido = malloc(entrada_actual->tamanio);
        memcpy(valor_leido,(memoria + entrada_actual->base),(entrada_actual->tamanio));//destino,origen,tamaño
        //===============================LEER=================================
        entrada_actual->base = entrada_anterior->base + entrada_anterior->tamanio;
        *tamanio_total_entradas += entrada_actual->tamanio;
        //===============================ESCRIBIR=============================
        memcpy((memoria + entrada_actual->base),valor_leido,(entrada_actual->tamanio));//destino,origen,tamaño
        free(valor_leido);
        //===============================ESCRIBIR=============================
        if(i==(tamanio_lista_temporal-1)){
            //entrada_actual deberia estar apuntando a la última entrada
            *base_nuevo_hueco = entrada_actual->base + entrada_actual->tamanio;
        }
        log_info(logger, "PID: %d - Segmento: %d - Base: %d - TAMAÑO: %d",entrada_actual->pid,entrada_actual->sid,entrada_actual->base,entrada_actual->tamanio);        
        entrada_anterior = entrada_actual;
    }   
}

void borrar_todos_los_huecos_libres(){
    uint32_t tamanio_lista = list_size(lista_huecos_libres);
    for(int i=0;i<tamanio_lista;i++){
        t_hueco_libre* hueco = list_remove(lista_huecos_libres,0);
        free(hueco);
    }
}

void crear_nuevo_hueco(uint32_t base_nuevo_hueco, uint32_t tamanio_total_entradas){
    t_hueco_libre* hueco = malloc(sizeof(t_hueco_libre));
    hueco->base = base_nuevo_hueco;
    hueco->tamanio = tam_memoria - (tam_segmento_cero + tamanio_total_entradas);
    list_add(lista_huecos_libres, hueco);
}

void enviar_claves_y_listas_tabla_segmentos(){
    t_list* lista_tablas = dictionary_elements(tabla_general);

    uint32_t cant_tablas = (uint32_t) list_size(lista_tablas);
    uint32_t size_stream = 2*sizeof(uint32_t) + cant_tablas*sizeof(uint32_t) + cant_tablas*sizeof(t_entrada_tabla_segmentos)*cant_segmentos;
    t_list* lista_claves = dictionary_keys(tabla_general);
    for(int i=0;i<list_size(lista_claves);i++){
        char* elem = list_get(lista_claves,i);
        size_stream += strlen(elem)+1;
    }
    void* stream = malloc(size_stream);
    int offset = 0;

    memcpy(stream+offset,&cant_tablas,sizeof(uint32_t));
    offset += sizeof(uint32_t);
    uint32_t cantidad_segmentos = (uint32_t) cant_segmentos;
    memcpy(stream+offset,&cantidad_segmentos,sizeof(uint32_t));
    offset += sizeof(uint32_t);

    for(int i=0;i<list_size(lista_claves);i++){
        char* elem = list_get(lista_claves,i);
        uint32_t size_elem = strlen(elem)+1;

        memcpy(stream+offset,&size_elem,sizeof(uint32_t));
        offset += sizeof(uint32_t);
        
        memcpy(stream+offset,elem,size_elem);
        offset += size_elem;
    }

    for(int i=0;i<cant_tablas;i++){
        t_list* tabla_segmentos= list_get(lista_tablas,i);
        for(int j=0;j<cantidad_segmentos;j++){
            t_entrada_tabla_segmentos* elem = list_get(tabla_segmentos,j);
            memcpy(stream+offset,elem,sizeof(t_entrada_tabla_segmentos));
            offset += sizeof(t_entrada_tabla_segmentos);
        }
    }

    send(socket_kernel,stream,size_stream,0);
    list_destroy(lista_claves);
    list_destroy(lista_tablas);
    free(stream);
}