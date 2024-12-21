#ifndef FINALIZAR_CPU_H_
#define FINALIZAR_CPU_H_

#include "cpu_gestor.h"

void finalizar_cpu();
void destruir_semaforos();
void destruir_pthreads();
void destruir_logs();
void destruir_configs();
void liberar_punteros();
void liberar_sockets();

#endif