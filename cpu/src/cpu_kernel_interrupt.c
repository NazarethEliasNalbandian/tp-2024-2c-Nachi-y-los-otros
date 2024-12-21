#include "../src/cpu_kernel_interrupt.h"
#include "../src/cpu_kernel_dispatch.h"

void atender_cpu_kernel_interrupt(){
    bool control_key=1;
	
    while (control_key) {
		int cod_op = recibir_operacion(fd_kernel_interrupt);
		t_buffer* unBuffer = NULL;
		switch (cod_op) {
			case HANDSHAKE_KERNEL_CPU_INTERRUPT:
                unBuffer = recibir_paquete(fd_kernel_interrupt);
                atender_handshake_kernel(unBuffer);
                break;
			case FORZAR_DESALOJO_CPU_KERNEL:
				unBuffer =  recibir_paquete(fd_kernel_interrupt); 
				manejar_interrupcion(unBuffer);
				break;
			case -1:
				printf("Desconexión de Kernel Interrupt\n");
                sem_post(&se_desconecto_kernel);
				control_key=0;
				break;
			default:
				log_warning(cpu_log_debug,"Operacion desconocida de Kernel Interrupt.");
				break;
		}
	}
}

void manejar_interrupcion(t_buffer * unBuffer){

	sem_wait(&llego_contexto_una_vez);
	interrupt_pid = recibir_int_del_buffer(unBuffer);
	interrupt_tid = recibir_int_del_buffer(unBuffer);

	log_trace(cpu_log_debug, "CONTEXTO PID %d TID %d -- INTERRUPCIÓN PID %d TID %d", un_contexto->pid, un_contexto->tid, interrupt_pid, interrupt_tid);

	if((interrupt_pid == un_contexto->pid && interrupt_tid == un_contexto->tid)){
		
		interrupt_verificador = recibir_int_del_buffer(unBuffer);

		destruir_buffer(unBuffer);

		log_info(cpu_log_obligatorio, "## Llega interrupción al puerto Interrupt");
		
		log_info(cpu_log_debug, "## INTERRUPCION QUANTUM A PID %d TID %d", interrupt_pid, interrupt_tid);

		pthread_mutex_lock(&mutex_interruptFlag);
		interrupt_flag = 1;
		pthread_mutex_unlock(&mutex_interruptFlag);

		pthread_mutex_lock(&(mutex_hubo_quantum));
		hubo_quantum = true;
		pthread_mutex_unlock(&(mutex_hubo_quantum));
	}
	else{
		log_error(cpu_log_debug, "INTERRUPCION RECHAZADA");
		destruir_buffer(unBuffer);
	}
	
}