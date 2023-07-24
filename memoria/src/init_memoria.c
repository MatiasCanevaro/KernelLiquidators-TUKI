#include <init_memoria.h>

// LOGGER
t_log* logger;

// CONFIG
t_config* config;

// RECIBIDOS DE CONFIG
char* ip_kernel;
char* puerto_kernel;
char* ip_filesystem;
char* puerto_filesystem;
char* ip_cpu;
char* puerto_cpu;
char* puerto_escucha;
int tam_memoria;
int tam_segmento_cero;
int cant_segmentos;
int retardo_memoria;
int retardo_compactacion;
char* algoritmo_asignacion;

// VARIABLES AUXILIARES
void* memoria;
t_entrada_tabla_segmentos* segmento_cero;
t_list* lista_huecos_libres; //lista de t_hueco_libre*
t_dictionary* tabla_general; //clave: pid | valor: lista_tabla_segmentos

// SOCKETS
int socket_servidor;
int socket_cpu;
int socket_kernel;
int socket_filesystem;

// semaforos
pthread_mutex_t mutex_memoria;

void iniciar_logger() {
    logger = crear_logger("cfg/memoria.log", "log_memoria");
    log_info(logger, "INICIO MEMORIA");
}

void leer_config(char* archivo_config){
    t_config* config_ips = config_create("cfg/ips.config");
    puerto_escucha = config_get_string_value(config_ips, "PUERTO_ESCUCHA");

    config = config_create(archivo_config);
    
    tam_memoria = config_get_int_value(config, "TAM_MEMORIA");
    tam_segmento_cero = config_get_int_value(config, "TAM_SEGMENTO_0");
    cant_segmentos = config_get_int_value(config, "CANT_SEGMENTOS");
    retardo_memoria = config_get_int_value(config, "RETARDO_MEMORIA");
    retardo_compactacion = config_get_int_value(config, "RETARDO_COMPACTACION");
    algoritmo_asignacion = config_get_string_value(config, "ALGORITMO_ASIGNACION");

    //TODO: config_destroy(config); 
}

void iniciar_servidor_memoria() {
    socket_servidor = iniciar_servidor(puerto_escucha);
    log_info(logger, "Servidor iniciado en el puerto %s", puerto_escucha);
}

int conectarse_a_cpu() {
    socket_cpu = esperar_cliente(socket_servidor);
    // socket_cpu = crear_conexion(ip_cpu, puerto_cpu);

    if(socket_cpu < 0) {
        log_error(logger, "Error al conectarse a la CPU");
        return 0;
    }
    // HANDSHAKE
    int hs = handshake_servidor(socket_cpu);
        if(hs<0){
            log_error(logger,"Resultado del handshake incorrecto");
            return 0;
        }

    log_info(logger, "Memoria conectada con CPU");

    return 1;
}

int conectarse_a_kernel() {
    socket_kernel = esperar_cliente(socket_servidor);
    // socket_kernel = crear_conexion(ip_kernel, puerto_kernel);
    
    // HANDSHAKE
    int hs = handshake_servidor(socket_kernel);
        if(hs<0){
            log_error(logger,"Resultado del handshake incorrecto");
            return 0;
        }

    log_info(logger, "Memoria conectada con Kernel");

    return 1;
}

int conectarse_a_filesystem() {
    socket_filesystem = esperar_cliente(socket_servidor); 
    // socket_filesystem = crear_conexion(ip_filesystem, puerto_filesystem);
    
    // HANDSHAKE
    int hs = handshake_servidor(socket_filesystem);
        if(hs<0){
            log_error(logger,"Resultado del handshake incorrecto");
            return 0;
        }

    log_info(logger, "Memoria conectada con Filesystem");

    return 1;
}

void iniciar_estructuras_administrativas(){
    memoria = (void *) malloc(tam_memoria);
    tabla_general = dictionary_create();
    inicializar_segmento_cero();
    inicializar_huecos_libres();
    pthread_mutex_init(&mutex_memoria,NULL);
}

void inicializar_segmento_cero(){
    log_info(logger, "Crear Segmento: %d - Base: %d - Tamanio: %d", 0, 0, tam_segmento_cero);
    segmento_cero = malloc(sizeof(t_entrada_tabla_segmentos));
    segmento_cero->sid = 0;
    segmento_cero->base = 0;
    segmento_cero->tamanio = tam_segmento_cero;
}

void inicializar_huecos_libres(){
    lista_huecos_libres = list_create();
    t_hueco_libre* hueco = malloc(sizeof(t_hueco_libre));
    hueco->base = tam_segmento_cero;
    hueco->tamanio = tam_memoria - tam_segmento_cero;
    list_add(lista_huecos_libres,hueco);
}