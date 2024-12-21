#ifndef CPU_KERNEL_DISPATCH_H_
#define CPU_KERNEL_DISPATCH_H_

#include "cpu_gestor.h"

void atender_cpu_kernel_dispatch();
void atender_handshake_kernel(t_buffer* unBuffer);
uint32_t *detectar_registro(t_registros RX);
bool check_interrupt();
void ciclo_de_instruccion();
void atender_hilo_kernel(t_buffer* unBuffer);

#endif 