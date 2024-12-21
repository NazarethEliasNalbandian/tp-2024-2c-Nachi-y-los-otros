#ifndef FINALIZAR_KERNEL_H_
#define FINALIZAR_KERNEL_H_

#include "k_gestor.h"

void finalizar_kernel();
void _finalizar_logger();
void _finalizar_config();
void eliminar_tcbs();
void _destruir_conexiones();
void _finalizar_semaforos();
void _finalizar_pthread();
void eliminar_pcbs();

#endif /* FINALIZAR_KERNEL_H_ */
