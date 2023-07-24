#ifndef HILO_KERNEL
#define HILO_KERNEL

#include <memoria.h>

void* esperar_kernel(void);
void recibir_parametros_kernel(t_list* parametros_kernel);
void crear_tabla_segmentos(char* pid);
void liberar_lista_tabla_segmentos_y_sus_entradas(char* pid);
void enviar_ok_al_kernel(void);
void administrar_delete_segment(char* pid, uint32_t sid);
void consolidar(uint32_t base, uint32_t tamanio);
t_hueco_libre* obtener_hueco_libre_a_la_izquierda(uint32_t base);
t_hueco_libre* obtener_hueco_libre_a_la_derecha(uint32_t base, uint32_t tamanio);
uint32_t comparar_bases_huecos(t_hueco_libre* hueco1, t_hueco_libre* hueco2);
void administrar_create_segment(char* pid, uint32_t sid_a_crear, uint32_t tamanio_segmento_a_crear);
uint32_t algun_hueco_tiene_tamanio_suficiente(uint32_t tamanio_segmento_a_crear);
t_entrada_tabla_segmentos* obtener_entrada(char* pid_buscado,uint32_t sid_a_crear);
void reducir_o_borrar_hueco(uint32_t df_base_hueco,uint32_t tamanio_segmento_a_crear);
void enviar_df_al_kernel(uint32_t df_base);
t_hueco_libre* obtener_hueco_de_base(uint32_t df_base_hueco);
int obtener_posicion_hueco(t_hueco_libre* hueco);
uint32_t obtener_df_base(uint32_t tamanio_segmento_a_crear);
t_hueco_libre* algoritmo_first(uint32_t tamanio_segmento_a_crear);
void filtrar_lista(t_list* lista_filtrada,uint32_t tamanio_segmento_a_crear);
t_hueco_libre* algoritmo_best(t_list* lista_filtrada);
t_hueco_libre* algoritmo_worst(t_list* lista_filtrada);
uint32_t espacioTotal(void);
void administrar_compactacion(void);
uint32_t comparar_bases_entradas(t_entrada_tabla_segmentos* entrada1, t_entrada_tabla_segmentos* entrada2);
void agregar_entradas(t_list* lista_temporal);
void compactar_entradas(t_list* lista_temporal, uint32_t* base_nuevo_hueco, uint32_t* tamanio_total_entradas);
void borrar_todos_los_huecos_libres(void);
void crear_nuevo_hueco(uint32_t base_nuevo_hueco, uint32_t tamanio_total_entradas);
void enviar_claves_y_listas_tabla_segmentos(void);

#endif