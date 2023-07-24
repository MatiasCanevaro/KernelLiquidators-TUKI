#ifndef CICLO_INSTRUCCION_H_
#define CICLO_INSTRUCCION_H_

#include <cpu.h>
#include <tads/pcb.h>
#include <paquete.h>
#include <math.h>
#include <string.h>

void conexion_kernel();
void ciclo_instruccion(t_pcb_cpu* pcb);
void ejecutar_set(t_instruccion* instruccion);
void devolver_contexto(t_pcb_cpu* pcb, uint32_t motivo_desalojo,t_list* parametros);
t_buffer* paquete_motivo_desalojo(uint8_t motivo, t_list* lista_strings); 
void ejecutar_read_o_write(t_pcb_cpu* pcb, t_list* parametros, uint8_t codigo_operacion);
char* encontrar_registro(char* nombre_registro,t_pcb_cpu* pcb);
#endif