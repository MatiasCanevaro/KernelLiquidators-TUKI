#include <init_kernel.h>
//CONFIG
t_config* config;
// LOGGER
t_log* logger;

// RECIBIDOS DE CONFIG
char* ip_memoria;
char* puerto_memoria;
char* ip_filesystem;
char* puerto_filesystem;
char* ip_cpu;
char* puerto_cpu;
char* puerto_escucha;
char* algoritmo_planificacion;
int estimacion_inicial;
double hrrn_alpha;
int grado_max_multiprogramacion;
char** recursos;
char** instancias_recursos;

// SOCKETS
int socket_servidor;
int socket_cpu;
int socket_memoria;
int socket_filesystem;

// VARIABLES AUXILIARES
int pid_actual;
t_list* lista_recursos_compartidos;
t_dictionary* diccionario_tds;
// ESTADOS
t_queue* cola_new;
t_queue* cola_ready;
t_queue* cola_fs;

// SEMAFOROS
sem_t sem_grado_multiprogramacion;
sem_t sem_cola_new_vacia;
sem_t sem_cola_ready_vacia;
sem_t sem_cola_fs;
sem_t sem_memoria_fs;

pthread_mutex_t mutex_cola_new;
pthread_mutex_t mutex_cola_ready;
pthread_mutex_t mutex_pid;
pthread_mutex_t mutex_lista_recursos;
pthread_mutex_t mutex_cola_fs;
pthread_mutex_t mutex_memoria;
pthread_mutex_t mutex_send_fs;
pthread_mutex_t mutex_diccionario_tds;

// TABLA DE ARCHIVOS
t_dictionary* tabla_archivos;



void iniciar_logger() {
    logger = crear_logger("cfg/kernel.log", "log_kernel");
    log_info(logger, "INICIO KERNEL");
}

void leer_config(char* archivo_config){
    t_config* config_ips = config_create("cfg/ips.config");
    
    ip_memoria = config_get_string_value(config_ips, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config_ips, "PUERTO_MEMORIA");
    ip_filesystem = config_get_string_value(config_ips, "IP_FILESYSTEM");
    puerto_filesystem = config_get_string_value(config_ips, "PUERTO_FILESYSTEM");
    ip_cpu = config_get_string_value(config_ips, "IP_CPU");
    puerto_cpu = config_get_string_value(config_ips, "PUERTO_CPU");
    puerto_escucha = config_get_string_value(config_ips, "PUERTO_ESCUCHA");

    
    config = config_create(archivo_config);
    
    algoritmo_planificacion=malloc(5);

    memcpy(algoritmo_planificacion, config_get_string_value(config, "ALGORITMO_PLANIFICACION"), 5);
    estimacion_inicial = config_get_int_value(config, "ESTIMACION_INICIAL");
    hrrn_alpha = config_get_double_value(config,"HRRN_ALFA");
    grado_max_multiprogramacion = config_get_int_value(config, "GRADO_MAX_MULTIPROGRAMACION");
    recursos = config_get_array_value(config, "RECURSOS");
    instancias_recursos = config_get_array_value(config, "INSTANCIAS_RECURSOS");
    
    //config_destroy(config); //TODO rompe
}

int countElements(char** array) {
    int count = 0;
    while (array[count] != NULL) {
        count++;
    }
    return count;
}

void iniciar_lista_recursos(){
    lista_recursos_compartidos= list_create();
    
    for(int i=0; i < countElements(recursos); i++){
        t_recurso* recurso=malloc(sizeof(t_recurso));
        recurso->nombre = recursos[i];
        recurso->cant_instancias = atoi(instancias_recursos[i]);
        recurso->cant_instancias_max =recurso->cant_instancias;
        recurso->cola_uso_recurso = queue_create();
        recurso->cola_bloqueados_recurso= queue_create();
        
        list_add(lista_recursos_compartidos, recurso);       
    }
}

void iniciar_colas_planificador() {
    cola_new = queue_create();
    cola_ready = queue_create();
    cola_fs= queue_create();
}

void iniciar_semaforos() {
    sem_init(&sem_cola_new_vacia, 0, 0);
    sem_init(&sem_cola_ready_vacia, 0, 0);
    sem_init(&sem_grado_multiprogramacion, 0, grado_max_multiprogramacion);
    sem_init(&sem_cola_fs, 0, 0);
    sem_init(&sem_memoria_fs, 0, 1);

    pthread_mutex_init(&mutex_cola_new,NULL);
    pthread_mutex_init(&mutex_cola_ready,NULL);
    pthread_mutex_init(&mutex_pid,NULL);
    pthread_mutex_init(&mutex_lista_recursos,NULL);
    pthread_mutex_init(&mutex_cola_fs,NULL);
    pthread_mutex_init(&mutex_memoria,NULL);
    pthread_mutex_init(&mutex_send_fs,NULL);
    pthread_mutex_init(&mutex_diccionario_tds,NULL);
}
void iniciar_diccionario_tds(){
    diccionario_tds = dictionary_create();
}
void iniciar_servidor_kernel() {
    socket_servidor = iniciar_servidor(puerto_escucha);
    log_info(logger, "Servidor iniciado en el puerto %s", puerto_escucha);
}

int conectarse_a_cpu() {
    socket_cpu = crear_conexion(ip_cpu, puerto_cpu);

    if(socket_cpu < 0) {
        log_error(logger, "No se pudo conectar con la CPU");
        return 0;
    }
    // HANDSHAKE
    int hs = handshake_cliente(socket_cpu);
        if(hs<0){
            log_error(logger,"Resultado del handshake incorrecto");
            return 0;
        }

    log_info(logger, "Kernel conectado con la CPU");

    return 1;
}

int conectarse_a_memoria() {
    socket_memoria = crear_conexion(ip_memoria, puerto_memoria);
    
    // HANDSHAKE
    int hs = handshake_cliente(socket_memoria);
        if(hs<0){
            log_error(logger,"Resultado del handshake incorrecto");
            return 0;
        }

    log_info(logger, "Kernel conectado con la Memoria");

    return 1;
}

int conectarse_a_filesystem() {
    socket_filesystem = crear_conexion(ip_filesystem, puerto_filesystem);
    
    // HANDSHAKE
    int hs = handshake_cliente(socket_filesystem);
        if(hs<0){
            log_error(logger,"Resultado del handshake incorrecto");
            return 0;
        }

    log_info(logger, "Kernel conectado con Filesystem");

    return 1;
}