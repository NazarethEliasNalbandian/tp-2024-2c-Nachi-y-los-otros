#include "../src/planificador_corto_plazo.h"
#include "../src/planificador_largo_plazo.h"
#include "../src/tcb.h"
#include "../src/servicios_kernel.h"
#include "../src/mutex.h"
#include "../src/cola_multinivel.h"

void pcp_planificador_corto_plazo(){
	int flag_lista_ready_vacia = 0;

	pthread_mutex_lock(&mutex_lista_tcb_ready);
	if(list_is_empty(lista_tcb_ready) && (ALGORITMO_PLANIFICACION != CMN)){
		flag_lista_ready_vacia = 1;

	}
	pthread_mutex_unlock(&mutex_lista_tcb_ready);

	pthread_mutex_lock(&mutex_lista_cola_multinivel);
	if((ALGORITMO_PLANIFICACION == CMN) && todas_las_colas_estan_vacias()){
		flag_lista_ready_vacia = 1;
	}
	pthread_mutex_unlock(&mutex_lista_cola_multinivel);

	

	if(flag_lista_ready_vacia == 0){
		switch (ALGORITMO_PLANIFICACION) {
			case FIFO:
				atender_FIFO();
				break;
			case PRIORIDADES:
				atender_PRIORIDADES();
				break;
			case CMN:
				atender_CMN();
				break;
			default:
				log_error(kernel_log_debug, "ALGORITMO DE CORTO PLAZO DESCONOCIDO");
				exit(EXIT_FAILURE);
				break;
		}
	}else	
		log_trace(kernel_log_debug, "NO HAY HILOS PARA PLANIFICAR");
}

void atender_FIFO(){
	log_trace(kernel_log_debug, "ENTRE A ATENDER FIFO");

	// Verificar que la lista de EXECUTE esté vacía
	pthread_mutex_lock(&mutex_lista_tcb_exec);
	if(list_is_empty(lista_tcb_exec)){
		t_tcb* un_tcb = NULL;

		//Verificar que haya elementos en la lista de READY
		pthread_mutex_lock(&mutex_lista_tcb_ready);
		if(!list_is_empty(lista_tcb_ready)){
			un_tcb = list_remove(lista_tcb_ready, 0);
		}
		pthread_mutex_unlock(&mutex_lista_tcb_ready);

		if(un_tcb != NULL){
			list_add(lista_tcb_exec, un_tcb);
			pthread_mutex_lock(&(un_tcb->mutex_tcb));
			un_tcb->estado = EXEC;
			log_trace(kernel_log_debug, "PID:%d TID: %d - Estado Actual: EXEC",un_tcb->id_hilo->pid_asociado, un_tcb->id_hilo->tid);
			log_info(kernel_log_listas_tcb, "PID %d TID %d AGREGADO A LISTA EXEC",un_tcb->id_hilo->pid_asociado, un_tcb->id_hilo->tid);
			pthread_mutex_unlock(&(un_tcb->mutex_tcb));

			enviar_tcb_CPU_dispatch(un_tcb);
			log_info(kernel_log_debug,"ENVIE TCB A CPU");
		}else{
			log_warning(kernel_log_debug, "Lista de READY vacía");
		}
	}
	pthread_mutex_unlock(&mutex_lista_tcb_exec);
}

void atender_PRIORIDADES(){
	t_tcb* un_tcb = NULL;

	// Tomo el elemento de mayor prioridad
	pthread_mutex_lock(&mutex_lista_tcb_ready);
	if(list_size(lista_tcb_ready) == 1){
		un_tcb = list_get(lista_tcb_ready, 0);
	}else{
		un_tcb = list_get_maximum(lista_tcb_ready, (void*)__maxima_prioridad);
	}

	pthread_mutex_lock(&mutex_lista_tcb_exec);

	// NO HAY NADIE EN EXEC Y HAY ALGUIEN EN READY PARA EJECUTAR
	if((list_is_empty(lista_tcb_exec) && !list_is_empty(lista_tcb_ready))){
		if(list_remove_element(lista_tcb_ready, un_tcb)){

			list_add(lista_tcb_exec, un_tcb);
			pthread_mutex_lock(&(un_tcb->mutex_tcb));
			un_tcb->estado = EXEC;
			log_trace(kernel_log_debug, "PID: %d TID: %d - Estado Actual: EXEC",un_tcb->id_hilo->pid_asociado, un_tcb->id_hilo->tid);
			log_info(kernel_log_listas_tcb, "PID %d TID %d AGREGADO A LISTA EXEC",un_tcb->id_hilo->pid_asociado, un_tcb->id_hilo->tid);
			pthread_mutex_unlock(&(un_tcb->mutex_tcb));

			enviar_tcb_CPU_dispatch(un_tcb);

		}else{
			log_error(kernel_log_debug, "No se encontro el PCB con mayor PRIORIDAD");
			exit(EXIT_FAILURE);
		}
	}
	
	pthread_mutex_unlock(&mutex_lista_tcb_exec);
	pthread_mutex_unlock(&mutex_lista_tcb_ready);
}

t_tcb* __maxima_prioridad(t_tcb* void_1, t_tcb* void_2){
	if(void_1->prioridad <= void_2->prioridad) 
		return void_1;
	else 
		return void_2;
}

void atender_CMN(){
	t_tcb* un_tcb = NULL;

	// NO HAY NADIE EN EXEC Y HAY ALGUIEN EN READY PARA EJECUTAR
	pthread_mutex_lock(&mutex_lista_tcb_exec);
	if(list_is_empty(lista_tcb_exec)){

		un_tcb = obtener_siguiente_tcb_a_ejecutar_en_CMN();

		list_add(lista_tcb_exec, un_tcb);
		pthread_mutex_unlock(&mutex_lista_tcb_exec);
		pthread_mutex_lock(&(un_tcb->mutex_tcb));
		un_tcb->verificador = generar_verificador();
		un_tcb->estado = EXEC;
		log_trace(kernel_log_debug, "PID: %d TID: %d - Estado Actual: EXEC",un_tcb->id_hilo->pid_asociado, un_tcb->id_hilo->tid);
		log_info(kernel_log_listas_tcb, "PID %d TID %d AGREGADO A LISTA EXEC",un_tcb->id_hilo->pid_asociado, un_tcb->id_hilo->tid);
		pthread_mutex_unlock(&(un_tcb->mutex_tcb));

		enviar_tcb_CPU_dispatch(un_tcb);
		log_info(kernel_log_debug,"ENVIE PCB A CPU");

		pthread_mutex_lock(&(un_tcb->mutex_tcb));
		un_tcb->flag_cancelar_quantum = false;
		pthread_mutex_unlock(&(un_tcb->mutex_tcb));
		ejecutar_en_un_hilo_nuevo_detach((void*)_programar_interrupcion_por_quantum, un_tcb);
	}else
	{
		t_tcb* un_tcb = list_get(lista_tcb_exec, 0);
		pthread_mutex_unlock(&mutex_lista_tcb_exec);
		pthread_mutex_lock(&(un_tcb->mutex_tcb));
		log_warning(kernel_log_debug, "EL HILO %d DEL PROCESO %d ESTA EJECUTANDO", un_tcb->id_hilo->tid, un_tcb->id_hilo->pid_asociado);
		pthread_mutex_unlock(&(un_tcb->mutex_tcb));
	}
	
	
}

void _programar_interrupcion_por_quantum(t_tcb* un_tcb){
	pthread_mutex_lock(&(un_tcb->mutex_tcb));
	if(un_tcb != NULL){
		int verificador_referencia = un_tcb->verificador;
		float quantum = un_tcb->temp;
		pthread_mutex_unlock(&(un_tcb->mutex_tcb));

		// log_trace(kernel_log_debug, "ESPERANDO QUANTUM DE PID %d TID %d", un_tcb->id_hilo->pid_asociado, un_tcb->id_hilo->tid);
		usleep(quantum*1000);
		// if(un_tcb != NULL)
		// 	log_trace(kernel_log_debug, "FINALICE QUANTUM DE PID %d TID %d", un_tcb->id_hilo->pid_asociado, un_tcb->id_hilo->tid);

		/* El veriicador se utiliza en caso de que el TCB haya salido de CPU,
		* entrado en READY y haya vuelto a la CPU.
		* Entonces, al despertar este hilo, primero verifique que la TCB objetivo no haya
		* salido de la CPU.
		* Si salio la misma TCB y volvió a entrar, significa que el proceso tiene
		* nuevo verificador
		*/
		
		if(verificador_referencia == var_verificador){
			// EL QUANTUM SE CANCELA CUANDO EL HILO VOLVIO DE CPU Y SE BLOQUEO O FINALIZO ANTES DE QUE TERMINE SU QUANTUM 
			// SI NO HAY NINGUN OTRO PROCESO EN READY, HAGO EL FIN DE QUANTUM?
			
			if(un_tcb != NULL){
				pthread_mutex_lock(&(un_tcb->mutex_tcb));
				if(un_tcb != NULL && !(un_tcb->flag_cancelar_quantum)){
					pthread_mutex_unlock(&(un_tcb->mutex_tcb));
					sem_post(&enviar_interrupcion_quantum);
					// log_info(kernel_log_debug, "FIN DE QUANTUM DEL TID %d CON PID %d", un_tcb->id_hilo->tid, un_tcb->id_hilo->pid_asociado);
				}else{
					pthread_mutex_unlock(&(un_tcb->mutex_tcb));
					// log_info(kernel_log_debug, "QUANTUM DEL PROCESO %d HILO %d CANCELADO", un_tcb->id_hilo->pid_asociado, un_tcb->id_hilo->tid);
				}
			}
		}
	}else
		pthread_mutex_unlock(&(un_tcb->mutex_tcb));
}

void bloquear_hilo_en_ejecucion(int tid, int pid, t_motivo_blocked motivo_bloqueo){
	
	t_tcb* un_tcb = buscar_y_remover_tcb_por_tid_de(tid, pid, lista_tcb_exec, mutex_lista_tcb_exec, EXEC);

	if(un_tcb != NULL){
		pthread_mutex_lock(&mutex_lista_tcb_blocked);
		list_add(lista_tcb_blocked, un_tcb);
		pthread_mutex_unlock(&mutex_lista_tcb_blocked);

		pthread_mutex_lock(&(un_tcb->mutex_tcb));
		un_tcb->flag_cancelar_quantum = true;
		pthread_mutex_unlock(&(un_tcb->mutex_tcb));

		log_info(kernel_log_obligatorio, "## (%d:%d) - Bloqueado por: %s", pid, tid, motivo_blocked_to_string(motivo_bloqueo)); 
		log_info(kernel_log_listas_tcb, "PID %d TID %d AGREGADO A LISTA BLOCKED POR %s", pid, tid, motivo_blocked_to_string(motivo_bloqueo)); 
		pcp_planificador_corto_plazo();
	}else
		log_error(kernel_log_debug, "TCB %d DEL PCB %d NO ENCONTRADO EN CPU", tid, pid);
}

void desbloquear_hilo(t_hilo_id* puntero_a_id_hilo_a_liberar){
	t_tcb* un_tcb = buscar_y_remover_tcb_por_tid_de(puntero_a_id_hilo_a_liberar->tid, puntero_a_id_hilo_a_liberar->pid_asociado, lista_tcb_blocked, mutex_lista_tcb_blocked, BLOCKED);

	log_trace(kernel_log_debug, "TAMAÑO LISTA BLOCKED: %d", list_size(lista_tcb_blocked));

	if(un_tcb != NULL){

		agregar_hilo_a_lista_ready(un_tcb);

		log_trace(kernel_log_debug, "PID %d TID %d PASA DE BLOCKED A READY", puntero_a_id_hilo_a_liberar->pid_asociado, puntero_a_id_hilo_a_liberar->tid);

		pcp_planificador_corto_plazo();
	}else 
		log_error(kernel_log_debug, "TID %d DEL PID %d NO ENCONTRADO EN BLOCKED", puntero_a_id_hilo_a_liberar->tid, puntero_a_id_hilo_a_liberar->pid_asociado);
}

void liberar_tcbs_en_espera_por_join(t_tcb* un_tcb){
	list_iterate(un_tcb->tids_en_espera_por_join, (void*) desbloquear_hilo);
}

void liberar_recursos_tcb(t_tcb* un_tcb){
	list_iterate(un_tcb->lista_mutex_asignados, (void*) protocolo_liberacion_recurso);
}