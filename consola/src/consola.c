#include <consola.h>

int main(int argc, char* argv[]) {
    
    // INIT
    iniciar_logger();
    leer_config();
    t_list* lista_instrucciones = obtener_instrucciones_de_archivo(argv[1]);

    // CONEXIONES
    int conexion_ok = conectarse_a_kernel();
    if(!conexion_ok) {
        finalizar_consola();
        return 1;
    }
    //DESTRUIMOS EL CONFIG
    config_destroy(config);

    // ENVIAMOS EL PAQUETE CON LAS INTRUCCIONES
    enviar_paquete_instrucciones(socket_kernel, lista_instrucciones);
    destruir_lista_instrucciones(lista_instrucciones);

    // ESPERAMOS QUE EL KERNEL CONFIRME QUE SE RECIBIO EL PAQUETE
    uint32_t paquete_recibido_ok;
    recv(socket_kernel, &paquete_recibido_ok, sizeof(uint32_t), MSG_WAITALL);
    if(paquete_recibido_ok != 200) {
        log_info(logger, "Error al enviar el paquete de instrucciones");
        finalizar_consola();

        return 1;
    }

    log_info(logger, "Paquete de instrucciones enviado con exito");
    
    // ESPERAMOS QUE KERNEL INDIQUE QUE TERMINO EL PROCESO
    uint32_t proceso_finalizado;
    recv(socket_kernel, &proceso_finalizado, sizeof(uint32_t), MSG_WAITALL);
    log_info(logger, "Proceso finalizado");

    // FINISH
    finalizar_consola();
   
    return 0;
}