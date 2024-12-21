#ifndef PLANIFICADOR_LARGO_PLAZO_H_
#define PLANIFICADOR_LARGO_PLAZO_H_

#include "k_gestor.h"

void crear_proceso_inicial(void* arg);
void finalizar_hilo(t_hilo_id* id_hilo);
void replanificar_creacion_proceso();
void finalizar_proceso(void* puntero_a_pid);
void mostrar_hilos_a_eliminar(t_list* hilos);
void eliminar_hilos(t_list* hilos);

#endif /* PLANIFICADOR_LARGO_PLAZO_H_ */
