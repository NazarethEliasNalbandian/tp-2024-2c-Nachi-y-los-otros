#ifndef PCB_H_
#define PCB_H_

#include "k_gestor.h"

t_pcb* crear_pcb(char* archivo_instrucciones, int tamanio, int prioridad_tid0);
void crear_proceso(char* archivo_instrucciones, int tamanio, int prioridad_tid0);
t_pcb* buscar_y_remover_pcb_por_pid(int un_pid);
t_pcb* buscar_pcb_por_pid(int un_pid);
t_pcb* buscar_pcb_por_pid_en(int un_pid, t_list* lista_estado);
void destruir_proceso(t_pcb* un_pcb);
void validacion(int verf, int ver);
bool preguntar_a_memoria_para_inicializacion(t_pcb* un_pcb);

#endif