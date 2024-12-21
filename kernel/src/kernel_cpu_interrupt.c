#include "../src/kernel_cpu_interrupt.h"
#include "../src/servicios_kernel.h"
#include "../src/planificador_corto_plazo.h"

void gestionar_interrupt_quantum(){
	while(1){
		sem_wait(&enviar_interrupcion_quantum);

		pthread_mutex_lock(&mutex_lista_tcb_exec);
		if(lista_tcb_exec != NULL) {
			t_tcb* tcb_execute = list_get(lista_tcb_exec, 0);
			pthread_mutex_unlock(&mutex_lista_tcb_exec);

			t_paquete* un_paquete = crear_paquete(FORZAR_DESALOJO_CPU_KERNEL);
			cargar_int_al_paquete(un_paquete, tcb_execute->id_hilo->pid_asociado);
			cargar_int_al_paquete(un_paquete, tcb_execute->id_hilo->tid);
			cargar_int_al_paquete(un_paquete, tcb_execute->verificador);
			enviar_paquete(un_paquete, fd_cpu_interrupt);
			eliminar_paquete(un_paquete);

			pthread_mutex_lock(&(tcb_execute->mutex_tcb));
			if(tcb_execute != NULL)
				log_trace(kernel_log_debug, "ENVIO PID %d TID %d VERIFICADOR %d A CPU POR FIN DE QUANTUM", tcb_execute->id_hilo->pid_asociado, tcb_execute->id_hilo->tid , tcb_execute->verificador);
			pthread_mutex_unlock(&(tcb_execute->mutex_tcb));
		}
		else
			pthread_mutex_unlock(&mutex_lista_tcb_exec);
    }
}

void atender_desalojo(t_tcb* un_tcb, bool hubo_quantum){

	pthread_mutex_lock(&(un_tcb->mutex_tcb));
	if(un_tcb != NULL)
		log_info(kernel_log_debug, "REUBICACION DE TCB %d DEL PCB %d DE EXEC A READY", un_tcb->id_hilo->tid, un_tcb->id_hilo->pid_asociado);
	pthread_mutex_unlock(&(un_tcb->mutex_tcb));

	pthread_mutex_lock(&mutex_lista_tcb_exec);

	if(!list_remove_element(lista_tcb_exec, un_tcb)){
		pthread_mutex_unlock(&mutex_lista_tcb_exec);

		pthread_mutex_lock(&(un_tcb->mutex_tcb));
		// log_warning(kernel_log_debug ,"TCB %d DEL PCB %d NO ESTE EN EXECUTE", un_tcb->id_hilo->tid, un_tcb->id_hilo->pid_asociado);
		pthread_mutex_unlock(&(un_tcb->mutex_tcb));
		// exit(EXIT_FAILURE);
	}else
		pthread_mutex_unlock(&mutex_lista_tcb_exec);

	agregar_hilo_a_lista_ready(un_tcb);

	if(hubo_quantum) {
		pthread_mutex_lock(&(un_tcb->mutex_tcb));
		log_info(kernel_log_obligatorio, "## (%d:%d) - Desalojado por fin de Quantum", un_tcb->id_hilo->pid_asociado, un_tcb->id_hilo->tid);
		pthread_mutex_unlock(&(un_tcb->mutex_tcb));
	}

	

	pcp_planificador_corto_plazo();
}
