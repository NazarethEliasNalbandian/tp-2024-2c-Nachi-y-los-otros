#include "../src/planificador_largo_plazo.h"
#include "../src/planificador_corto_plazo.h"
#include "../src/pcb.h"
#include "../src/tcb.h"
#include "../src/servicios_kernel.h"
#include "../src/kernel_memoria.h"
#include "../src/mutex.h"

void crear_proceso_inicial(void* arg){
	t_args_creacion* proceso_args = (t_args_creacion*) arg;
	char* archivo_instrucciones = proceso_args->archivo_instrucciones;
    int tamanio = proceso_args->tamanio;

	safe_free(proceso_args);
	crear_proceso(archivo_instrucciones, tamanio, 0);

	sem_wait(&finalizo_proceso_inicial);
}

void replanificar_creacion_proceso(){

	bool logro_inicializar_proceso = true;
	
	while(logro_inicializar_proceso){
		pthread_mutex_lock(&mutex_lista_pcb_new);
		if(!list_is_empty(lista_pcb_new)){
			t_pcb* un_pcb;
			un_pcb = list_get(lista_pcb_new, 0);
			pthread_mutex_unlock(&mutex_lista_pcb_new);

			pthread_mutex_lock(&(un_pcb->mutex_pcb));
			log_info(kernel_log_listas_pcb, "INICIO REPLANIFICACION DEL PROCESO %d", un_pcb->pid);
			pthread_mutex_unlock(&(un_pcb->mutex_pcb));
			logro_inicializar_proceso = preguntar_a_memoria_para_inicializacion(un_pcb);
		}else{
			pthread_mutex_unlock(&mutex_lista_pcb_new);
			logro_inicializar_proceso = false;
		}
	}
}

void finalizar_hilo(t_hilo_id* id_hilo){
	t_tcb* un_tcb = buscar_y_remover_tcb_por_tid(id_hilo->tid, id_hilo->pid_asociado);

	if(un_tcb == NULL){
		log_error(kernel_log_debug, "EL HILO A FINALIZAR NO SE ENCONTRO");
		return;
	}
	
	log_trace(kernel_log_debug, "HILO A ELIMINAR %d", id_hilo->tid);

	un_tcb->flag_cancelar_quantum = true;
	t_pcb* un_pcb = NULL;

	if(buscar_pcb_por_pid_en(id_hilo->pid_asociado, lista_pcb_new) != NULL){
		un_pcb = buscar_pcb_por_pid_en(id_hilo->pid_asociado, lista_pcb_new);
		log_trace(kernel_log_debug, "PROCESO %d DEL HILO A ELIMINAR ENCONTRADO EN NEW", id_hilo->pid_asociado);
	}

	if(buscar_pcb_por_pid_en(id_hilo->pid_asociado, lista_pcb_enMemoria) != NULL){
		un_pcb = buscar_pcb_por_pid_en(id_hilo->pid_asociado, lista_pcb_enMemoria);
		log_trace(kernel_log_debug, "PROCESO %d DEL HILO A ELIMINAR ENCONTRADO EN LISTA MEMORIA", id_hilo->pid_asociado);
	}

	avisar_a_memoria_para_liberar_estructuras_hilo(id_hilo->tid, id_hilo->pid_asociado);

	sem_wait(&llego_respuesta_finalizacion_hilo);

	pthread_mutex_lock(&mutex_rta_finalizacion_hilo);

	if(strcmp(rta_finalizacion_hilo, "OK") == 0){
		desocupar_mutex(un_tcb);
		liberar_recursos_tcb(un_tcb);

		pthread_mutex_lock(&mutex_lista_tcb_exit);
		list_add(lista_tcb_exit, un_tcb);
		pthread_mutex_unlock(&mutex_lista_tcb_exit);
		
		if(un_pcb != NULL){
			remover_id_hilo_de_lista(un_pcb->lista_id_hilo, id_hilo);
		} else
			log_warning(kernel_log_debug, "EL PCB NO SE ENCUENTRA EN LA LISTA DE PCB EN MEMORIA O NEW");

		liberar_tcbs_en_espera_por_join(un_tcb);
		log_info(kernel_log_obligatorio, "## (%d:%d) Finaliza el hilo", id_hilo->pid_asociado, id_hilo->tid);
		log_info(kernel_log_listas_tcb, "PID %d TID %d AGREGADO A LISTA EXIT", id_hilo->pid_asociado, id_hilo->tid);
		log_info(kernel_log_largo_plazo, "## (%d:%d) Finaliza el hilo", id_hilo->pid_asociado, id_hilo->tid);
		safe_free(id_hilo);
	} else{

		log_error(kernel_log_debug, "NO SE PUDO LIBERAR EL TID %d DEL PID %d", id_hilo->tid, id_hilo->pid_asociado);
		safe_free(id_hilo);
	}	

	pthread_mutex_unlock(&mutex_rta_finalizacion_hilo);

	pcp_planificador_corto_plazo();
}

void finalizar_proceso(void* puntero_a_pid){
	int pid = *((int*) puntero_a_pid);
	safe_free(puntero_a_pid);
	
	t_pcb* un_pcb;
	un_pcb = buscar_pcb_por_pid_en(pid, lista_pcb_exit);
	if(un_pcb != NULL){
		log_info(kernel_log_debug, "EL PROCESO %d YA FINALIZO", pid);
		return;
	}
	un_pcb = buscar_pcb_por_pid_en(pid, lista_pcb_enMemoria);

	if(un_pcb == NULL){
		un_pcb = buscar_pcb_por_pid_en(pid, lista_pcb_enMemoria);
	}

	if(un_pcb == NULL){
		un_pcb = buscar_pcb_por_pid_en(pid, lista_pcb_new);
	}

	if(un_pcb == NULL){
		log_info(kernel_log_debug, "EL PROCESO %d NO FUE ENCONTRADO", pid);
		return;
	}

	log_trace(kernel_log_debug, "EL PROCESO %d COMIENZA SU FINALIZACION", pid);

	avisar_a_memoria_para_liberar_estructuras_proceso(pid);

	sem_wait(&llego_respuesta_finalizacion_proceso);
	log_trace(kernel_log_debug, "FINALIZO EL PID %d EN MEMORIA", pid);

	pthread_mutex_lock(&mutex_rta_finalizacion_proceso);

	if(strcmp(rta_finalizacion_proceso, "OK") == 0){
		
		pthread_mutex_lock((&(un_pcb->mutex_pcb)));
		un_pcb->flag_proceso_finalizado = true;
		pthread_mutex_unlock((&(un_pcb->mutex_pcb)));

		pthread_mutex_unlock(&mutex_rta_finalizacion_proceso);
		pthread_mutex_lock((&(un_pcb->mutex_pcb)));
		mostrar_hilos_a_eliminar(un_pcb->lista_id_hilo);
		// list_iterate(un_pcb->lista_id_hilo, (void*) finalizar_hilo);
		eliminar_hilos(un_pcb->lista_id_hilo);
		pthread_mutex_unlock((&(un_pcb->mutex_pcb)));

		replanificar_creacion_proceso();

		pthread_mutex_lock(&mutex_lista_pcb_enMemoria);
		list_remove_element(lista_pcb_enMemoria, un_pcb);
		pthread_mutex_unlock(&mutex_lista_pcb_enMemoria);

		pthread_mutex_lock(&(un_pcb->mutex_pcb));
		log_info(kernel_log_listas_pcb, "REMUEVO PROCESO %d DE LA LISTA MEMORIA", un_pcb->pid);
		pthread_mutex_unlock(&(un_pcb->mutex_pcb));

		pthread_mutex_lock(&mutex_lista_pcb_exit);
		list_add(lista_pcb_exit, un_pcb);
		pthread_mutex_unlock(&mutex_lista_pcb_exit);

		pthread_mutex_lock(&(un_pcb->mutex_pcb));
		log_info(kernel_log_listas_pcb, "AGREGO  PROCESO %d DE LA LISTA EXIT", un_pcb->pid);

		un_pcb->estado = EXIT_PCB;
		pthread_mutex_unlock(&(un_pcb->mutex_pcb));

		log_info(kernel_log_obligatorio, "## Finaliza el proceso %d", pid);
		log_info(kernel_log_largo_plazo, "## Finaliza el proceso %d", pid);
	}else{
		pthread_mutex_unlock(&mutex_rta_finalizacion_proceso);
		log_error(kernel_log_debug, "NO SE PUDO LIBERAR EL PID %d", pid);
	}	
	
	mostrarListaPCBMemoria();
	
}

void eliminar_hilos(t_list* hilos){

	bool esHiloCero(void* id_thre){
		t_hilo_id* id_thread = (t_hilo_id*) id_thre;
		return id_thread->tid == 0;
	};

	t_hilo_id* hilo_id_cero = list_remove_by_condition(hilos, esHiloCero);

	for (int i = 0; i < list_size(hilos); i++) {
		t_hilo_id* hilo_id = list_get(hilos, i);
		finalizar_hilo(hilo_id);
    }

	finalizar_hilo(hilo_id_cero);
}

void mostrar_hilos_a_eliminar(t_list* hilos){

	for (int i = 0; i < list_size(hilos); i++) {

		t_hilo_id* hilo_id = list_get(hilos, i);
		log_warning(kernel_log_debug, "HILO %d DEL PROCESO %d A ELIMINAR", hilo_id->tid, hilo_id->pid_asociado);
    }
}
