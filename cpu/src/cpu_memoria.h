#ifndef CPU_MEMORIA_H_
#define CPU_MEMORIA_H_

#include "cpu_gestor.h"

void atender_cpu_memoria();
void solicitar_contexto_a_memoria(int pid_en_ejecucion,int tid_en_ejecucion);
void solicitar_base_actual_a_memoria(int pid_en_ejecucion,int tid_en_ejecucion);

#endif