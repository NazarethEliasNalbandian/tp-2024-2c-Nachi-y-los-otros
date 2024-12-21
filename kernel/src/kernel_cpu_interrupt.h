#ifndef KERNEL_CPU_INTERRUPT_H_
#define KERNEL_CPU_INTERRUPT_H_

#include "k_gestor.h"

void gestionar_interrupt_quantum();
void atender_desalojo(t_tcb* un_tcb, bool hubo_quantum);



#endif /* KERNEL_CPU_INTERRUPT_H_ */
