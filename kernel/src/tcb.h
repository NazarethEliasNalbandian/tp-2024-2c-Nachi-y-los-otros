#ifndef TCB_H_
#define TCB_H_

#include "k_gestor.h"

void crear_tcb(t_pcb* un_pcb, char* archivo_instrucciones, int prioridad);
char* estado_tcb_to_string(estado_tcb estado);
void destruir_lista_tid_a_esperar(t_tcb* un_tcb);
void destruir_tcb(t_tcb* un_tcb);
void enviar_tcb_CPU_dispatch(t_tcb* un_tcb);
t_tcb* obtener_hilo_desalojado(int un_pid, int un_tid, int verificador);
bool existe_hilo(int un_tid, int un_pid);
bool verificador();
t_tcb* buscar_y_remover_tcb_por_tid(int un_tid, int un_pid);
t_tcb* buscar_tcb_por_tid(int un_tid, int un_pid);
t_tcb* buscar_y_remover_tcb_por_tid_de(int un_tid, int un_pid, t_list* lista, pthread_mutex_t mutex_lista, estado_tcb estado_hilo); 
t_tcb* buscar_tcb_por_tid_de(int un_tid, int un_pid, t_list* lista, pthread_mutex_t mutex_lista);
void remover_tcb_de_lista(t_tcb* un_tcb, t_list* lista, pthread_mutex_t mutex_lista);

#endif