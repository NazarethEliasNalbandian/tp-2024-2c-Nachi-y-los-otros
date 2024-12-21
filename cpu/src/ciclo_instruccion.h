#ifndef CICLO_INSTRUCCION_H_
#define CICLO_INSTRUCCION_H_

#include "cpu_gestor.h"

void fetch();
void decode();
uint32_t solicitar_memoria(int dir_logica);
int escribir_valor_memoria(uint32_t dir_logica, uint32_t valorAEscribir);
void execute();

#endif