#ifndef INICIALIZAR_MEMORIA_H_
#define INICIALIZAR_MEMORIA_H_

#include "m_gestor.h"

void inicializar_memoria(char* archivo_config);
void inicializar_configs(char* archivo_config);
void inicializar_logs();
void iniciar_estructuras();
void iniciar_particiones();
void iniciar_semaforos();
void iniciar_pthreads();

#endif /* INICIALIZAR_MEMORIA_H_ */
