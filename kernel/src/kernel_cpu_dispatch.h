#ifndef KERNEL_CPU_DISPATCH_H_
#define KERNEL_CPU_DISPATCH_H_

#include "k_gestor.h"

void atender_kernel_cpu_dispatch();
void atender_io(t_sleep * tcb_sleep);
void manejar_sleep(void* arg);
void atender_dump_memory(void* tcb);

#endif /* KERNEL_CPU_DISPATCH_H_ */
