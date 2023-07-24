#ifndef HILO_PLANIFICADOR_CORTO_PLAZO_H_
#define HILO_PLANIFICADOR_CORTO_PLAZO_H_

#include <kernel.h>

void* planificador_corto_plazo();
void ejecutar_pcb(t_pcb* pcb);
t_pcb* obtenerPCB();
float calcularRR(t_pcb* pcb,t_temporal* tiempo);
uint32_t estimadoProximaRafaga(t_temporal* tiempo_envio_a_cpu, uint32_t est_anterior);
void manejar_motivo_desalojo(t_pcb* pcb,t_list* motivo_desalojo);
void enviar_a_exit(t_pcb* pcb);
void desbloquear_proceso_rec(t_recurso* recurso);
t_list* recibir_motivo_desalojo();


#endif 