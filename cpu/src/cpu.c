#include <cpu.h>

int main(int argc, char* argv[]) {
    
    // INIT
    iniciar_logger();
    leer_config(argv[1]);

    //Esto solo para que no tire error valgrind;

    // CONEXIONES
    iniciar_servidor_cpu();

    // MEMORIA
    int conexion_ok = conectarse_a_memoria();
    if(!conexion_ok) {
        finalizar_cpu();
        return 1;
    }

    // KERNEL
    conexion_ok = conectarse_a_kernel();
    if(!conexion_ok) {
        finalizar_cpu();
        return 1;
    }

    // DESTRUIMOS LA CONFIG
    //config_destroy(config);

    // DEPLOY HILOS
    pthread_t hilo_kernel;
    pthread_create(&hilo_kernel, NULL, conexion_kernel, NULL);

    //t_registros_cpu* registros_cpu = crear_registros_cpu();

    pthread_join(hilo_kernel, NULL);

    // FINISH
    finalizar_cpu();

    return 0;
}