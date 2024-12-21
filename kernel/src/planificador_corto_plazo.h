#ifndef PLANIFICADOR_CORTO_PLAZO_H_
#define PLANIFICADOR_CORTO_PLAZO_H_

#include "k_gestor.h"

void pcp_planificador_corto_plazo();
void atender_FIFO();
void atender_PRIORIDADES();
void atender_CMN();
void _programar_interrupcion_por_quantum(t_tcb* un_tcb);
void bloquear_hilo_en_ejecucion(int tid, int pid, t_motivo_blocked motivo_bloqueo);
void desbloquear_hilo(t_hilo_id* puntero_a_id_hilo_a_liberar);
void liberar_tcbs_en_espera_por_join(t_tcb* un_tcb);
void liberar_recursos_tcb(t_tcb* un_tcb);
t_tcb* __maxima_prioridad(t_tcb* void_1, t_tcb* void_2);

#endif 