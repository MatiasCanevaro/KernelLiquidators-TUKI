#include <memoria.h>

int main(int argc, char* argv[]) {
    // INIT
    iniciar_logger();
    leer_config(argv[1]);
    
    // CONEXIONES
    iniciar_servidor_memoria();

    // CPU
    int conexion_ok = conectarse_a_cpu();
    if(!conexion_ok) {
        finalizar_memoria();
        return 1;
    }

    // FILESYSTEM
    conexion_ok = conectarse_a_filesystem();
    if(!conexion_ok) {
        finalizar_memoria();
        return 1;
    }

    // KERNEL
    conexion_ok = conectarse_a_kernel();
    if(!conexion_ok) {
        finalizar_memoria();
        return 1;
    }

    iniciar_estructuras_administrativas();

    // DEPLOY DE HILOS
    pthread_t hilo_kernel;
    pthread_create(&hilo_kernel, NULL, esperar_kernel, NULL);

    pthread_t hilo_cpu;
    pthread_create(&hilo_cpu, NULL, esperar_cpu, NULL);
    pthread_detach(&hilo_cpu);
    
    pthread_t hilo_fs;
    pthread_create(&hilo_fs, NULL, esperar_fs, NULL);
    pthread_detach(&hilo_fs);
    
    pthread_join(hilo_kernel,NULL);

    // FINISH
    finalizar_memoria();
    
    return 0;
}