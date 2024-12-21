#ifndef ESPACIO_USUARIO_H_
#define ESPACIO_USUARIO_H_

#include "m_gestor.h"
#include "servicios_memoria.h"

void escribir_uint32_en_dir_fisica(int pid, int tid, int dir_fisica, uint32_t* valor);
uint32_t leer_uint32_de_dir_fisica(int pid, int tid, int dir_fisica);
void* leer_data_de_dir_fisica(int pid, int tid, int dir_fisica, size_t tamanio);

#endif 
