#ifndef PROCESO_H_
#define PROCESO_H_

#include "m_gestor.h"

void crear_proceso_esquema_fijo(int pid, t_particion* particion_asignada);
void crear_proceso_esquema_dinamico(int pid, t_particion* particion_asignada, int tamanio);
void finalizar_proceso(t_particion* particion, t_proceso* proceso);
t_proceso* buscar_proceso(int pid);
bool existe_proceso(int pid);

#endif 