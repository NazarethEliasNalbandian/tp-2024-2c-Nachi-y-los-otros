#ifndef HILO_H_
#define HILO_H_

#include "m_gestor.h"

void crear_hilo(int pid, int tid, char* archivo_instrucciones);
t_hilo* buscar_hilo(int pid, int tid);
void actualizar_contexto_hilo(t_hilo* un_hilo, t_contexto* un_contexto);
char* obtener_instruccion_por_indice(t_hilo* un_hilo, uint32_t indice_instruccion);
void destruir_hilo(t_hilo* un_hilo);

#endif