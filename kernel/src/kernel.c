#include <kernel.h>

int main(int argc, char* argv[]) {

    // INIT
    iniciar_logger();
    leer_config(argv[1]);
    iniciar_lista_recursos();
    iniciar_colas_planificador();
    iniciar_semaforos();
    pid_actual = 0;
    tabla_archivos = dictionary_create();
    iniciar_diccionario_tds();
    // CONEXIONES
    iniciar_servidor_kernel();

    // CPU
    int conexion_ok = conectarse_a_cpu();
    if(!conexion_ok) {
        finalizar_kernel();
        return 1;
    }

    // MEMORIA
    conexion_ok = conectarse_a_memoria();
    if(!conexion_ok) {
        finalizar_kernel();
        return 1;
    }

    // FILESYSTEM
    conexion_ok = conectarse_a_filesystem();
    if(!conexion_ok) {
        finalizar_kernel();
        return 1;
    }

    // DESTRUIMOS LA CONFIG
    config_destroy(config);
    
    // DEPLOY DE HILOS
    pthread_t hilo_consolas;
    pthread_create(&hilo_consolas, NULL, esperar_consolas, NULL);
     
    pthread_t hilo_ready;
    pthread_create(&hilo_ready, NULL, agregar_ready, NULL);
    pthread_detach(&hilo_ready);
    
    pthread_t hilo_planificador_corto_plazo;
    pthread_create(&hilo_planificador_corto_plazo, NULL, planificador_corto_plazo, NULL);
    pthread_detach(&hilo_planificador_corto_plazo);

    pthread_t hilo_fs;
    pthread_create(&hilo_fs, NULL, manejar_fs, NULL);
    pthread_detach(&hilo_fs);

    pthread_join(hilo_consolas, NULL);

    // FINISH
    finalizar_kernel();

    return 0;
}