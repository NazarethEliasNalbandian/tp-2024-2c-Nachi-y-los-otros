#ifndef COLA_MULTINIVEL_H_
#define COLA_MULTINIVEL_H_

#include "k_gestor.h"

void crear_colamultinivel(int prioridad_cola);
bool existe_cola_con_esa_prioridad(int prioridad);
t_cola_multinivel* obtener_cola_por_prioridad(int prioridad);
t_tcb* obtener_siguiente_tcb_a_ejecutar_en_CMN();
bool todas_las_colas_estan_vacias();
t_cola_multinivel* obtener_cola_por_prioridad(int prioridad);
t_tcb* buscar_y_remover_tcb_de_cola_multinivel(int un_tid, int un_pid);
t_tcb* buscar_tcb_en_cola_multinivel(int un_tid, int un_pid);
void destruir_cola_multinivel(t_cola_multinivel* cola_multinivel);


#endif 