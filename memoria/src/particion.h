#ifndef PARTICION_H_
#define PARTICION_H_

#include "m_gestor.h"

void crear_particion_a_demanda(int tamanio);
t_particion* buscar_particion_por_id(int particion_id);
t_particion* obtener_primera_particion_libre(int tamanio_solicitado);
t_particion* obtener_peor_particion_libre(int tamanio_solicitado);
t_particion* obtener_mejor_particion_libre(int tamanio_solicitado);
t_particion* obtener_particion_a_asignar(int tamanio);
void ordenar_particiones_por_base();
void unificar_particiones_libres_contiguas();
void mostrarParticiones();
t_particion* buscar_particion_por_base_y_limite(int base, int limite);
void destruir_particion(t_particion* una_particion);

#endif