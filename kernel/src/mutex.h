#ifndef MUTEX_H_
#define MUTEX_H_

#include "k_gestor.h"

t_mutex_pcb* crear_mutex(char* nombre_recurso, int pid);
t_mutex_pcb* obtener_mutex(char* nombre_recurso, int pid);
bool esta_tomado_por(t_mutex_pcb* un_mutex, int tid, int pid);
bool esta_tomado(t_mutex_pcb* un_mutex);
void asignar_mutex(t_mutex_pcb* un_mutex, int tid, int pid);
void liberar_mutex(t_mutex_pcb* un_mutex);
void destruir_lista_bloqueados(t_mutex_pcb* un_mutex);
void destruir_mutex(t_mutex_pcb* un_mutex);
void remover_recurso_asignado(t_mutex_pcb* un_mutex, t_tcb* tcb_que_libera_recurso);
void atender_liberacion_de_recurso(t_mutex_pcb* un_mutex, t_tcb* tcb_que_libera_recurso);
void protocolo_liberacion_recurso(t_mutex_pcb* un_mutex);
void remover_hilo_de_mutex(t_mutex_pcb* un_mutex, t_hilo_id* id_hilo);
void desocupar_mutex(t_tcb* un_tcb);
bool remover_id_hilo_de_lista(t_list* lista, t_hilo_id* id_hilo);

#endif