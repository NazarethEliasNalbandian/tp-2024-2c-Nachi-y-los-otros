#include "../src/pcb.h"
#include "../src/tcb.h"
#include "../src/kernel_memoria.h"
#include "../src/mutex.h"

t_pcb* crear_pcb(char* archivo_instrucciones, int tamanio, int prioridad_tid0){

    t_pcb* nuevo_pcb = malloc(sizeof(t_pcb));

    pthread_mutex_lock(&mutex_process_id);
	nuevo_pcb->pid = process_id;
	process_id++;
	pthread_mutex_unlock(&mutex_process_id);

    nuevo_pcb->tamanio = tamanio;
	nuevo_pcb->archivo_instrucciones = string_duplicate(archivo_instrucciones);
	nuevo_pcb->prioridad_tid0 = prioridad_tid0;

    nuevo_pcb->lista_mutex = list_create();
    nuevo_pcb->lista_id_hilo   = list_create();

    pthread_mutex_init(&(nuevo_pcb->mutex_pcb), NULL);

    nuevo_pcb->siguiente_tid = 0;
    nuevo_pcb->estado = NEW_PCB;
	nuevo_pcb->flag_proceso_finalizado = false;

	t_hilo_id* nuevo_id_hilo = malloc(sizeof(t_hilo_id));

	nuevo_id_hilo->tid = nuevo_pcb->siguiente_tid;
	nuevo_id_hilo->pid_asociado = nuevo_pcb->pid;

	pthread_mutex_lock(&(nuevo_pcb->mutex_pcb));
	list_add(nuevo_pcb->lista_id_hilo, nuevo_id_hilo);
    pthread_mutex_unlock(&(nuevo_pcb->mutex_pcb));

    pthread_mutex_lock(&mutex_lista_pcb_new);
    list_add(lista_pcb_new, nuevo_pcb);
    pthread_mutex_unlock(&mutex_lista_pcb_new);

	log_info(kernel_log_listas_pcb, "AGREGO  PROCESO %d A LISTA PCB NEW", nuevo_pcb->pid);

    return nuevo_pcb;
}


bool preguntar_a_memoria_para_inicializacion(t_pcb* un_pcb){
	pthread_mutex_lock(&(un_pcb->mutex_pcb));
	enviar_y_recibir_peticion_creacion_proceso(un_pcb->pid, un_pcb->tamanio);
	log_info(kernel_log_listas_pcb, "PREGUNTO PARA INICIALIZAR PROCESO %d", un_pcb->pid);
	pthread_mutex_unlock(&(un_pcb->mutex_pcb));
	sem_wait(&llego_respuesta_creacion_proceso);


	pthread_mutex_lock(&mutex_rta_creacion_proceso);
	log_info(kernel_log_listas_pcb, "LLEGO RESPUESTA %s", rta_creacion_proceso);

	if(strcmp(rta_creacion_proceso,"OK") == 0) // SI LA RESPUESTA DIO OK, CREO EL TCB Y LA MANDO A PLANIFICAR
	{
		pthread_mutex_unlock(&mutex_rta_creacion_proceso);
		pthread_mutex_lock(&(un_pcb->mutex_pcb));
		crear_tcb(un_pcb, un_pcb->archivo_instrucciones, un_pcb->prioridad_tid0);
		pthread_mutex_unlock(&(un_pcb->mutex_pcb));

		pthread_mutex_lock(&mutex_lista_pcb_new);
		list_remove_element(lista_pcb_new, un_pcb);
		pthread_mutex_unlock(&mutex_lista_pcb_new);

		pthread_mutex_lock(&(un_pcb->mutex_pcb));
		log_info(kernel_log_listas_pcb, "REMUEVO PROCESO %d DE LA LISTA NEW", un_pcb->pid);
		pthread_mutex_unlock(&(un_pcb->mutex_pcb));

		pthread_mutex_lock(&(un_pcb->mutex_pcb));

		pthread_mutex_lock(&mutex_lista_pcb_enMemoria);
		list_add(lista_pcb_enMemoria, un_pcb);
		pthread_mutex_unlock(&mutex_lista_pcb_enMemoria);

		log_info(kernel_log_listas_pcb, "AGREGO  PROCESO %d DE LA LISTA MEMORIA", un_pcb->pid);
		un_pcb->estado = EN_MEMORIA_PCB;

		pthread_mutex_unlock(&(un_pcb->mutex_pcb));


		return true;		
	}
	else{
		pthread_mutex_unlock(&mutex_rta_creacion_proceso);
		pthread_mutex_lock(&(un_pcb->mutex_pcb));
		log_error(kernel_log_debug, "Memoria no acepto la solicitud de inicialización del proceso: %d",un_pcb->pid);
		pthread_mutex_unlock(&(un_pcb->mutex_pcb));
		return false;
	}

	
}

void crear_proceso(char* archivo_instrucciones, int tamanio, int prioridad_tid0){

    t_pcb* un_pcb = crear_pcb(archivo_instrucciones, tamanio, prioridad_tid0);
	pthread_mutex_lock(&(un_pcb->mutex_pcb));
	log_info(kernel_log_obligatorio, "## (%d:0) Se crea el proceso - Estado: NEW", un_pcb->pid);
	log_info(kernel_log_largo_plazo, "## (%d:0) Se crea el proceso - Estado: NEW", un_pcb->pid);
	pthread_mutex_unlock(&(un_pcb->mutex_pcb));

    // LA COLA ESTABA VACIA ANTES DE AGREGAR ESTE PCB
    if(list_size((lista_pcb_new)) == 1){
		preguntar_a_memoria_para_inicializacion(un_pcb);
    }
    else{
		pthread_mutex_lock(&(un_pcb->mutex_pcb));
        log_trace(kernel_log_debug,"PID %d ESPERANDO PARA CREAR EL TCB 0", un_pcb->pid);
		pthread_mutex_unlock(&(un_pcb->mutex_pcb));
	}   
}

t_pcb* buscar_pcb_por_pid(int un_pid){
	t_pcb* un_pcb;
	int elemento_encontrado = 0;

	bool __buscar_pcb(t_pcb* void_pcb){
		if(void_pcb->pid == un_pid){
			return true;
		} else {
			return false;
		}
	};

	if(elemento_encontrado == 0){
		pthread_mutex_lock(&mutex_lista_pcb_new);
		if(list_any_satisfy(lista_pcb_new, (void*)__buscar_pcb)){
			elemento_encontrado = 1;
			un_pcb = list_find(lista_pcb_new, (void*)__buscar_pcb);		
		}
		pthread_mutex_unlock(&mutex_lista_pcb_new);
	}

	if(elemento_encontrado == 0){
		pthread_mutex_lock(&mutex_lista_pcb_enMemoria);
		if(list_any_satisfy(lista_pcb_enMemoria, (void*)__buscar_pcb)){
			elemento_encontrado = 1;
			un_pcb = list_find(lista_pcb_enMemoria, (void*)__buscar_pcb);
		}
		pthread_mutex_unlock(&mutex_lista_pcb_enMemoria);
	}
	if(elemento_encontrado == 0){
		pthread_mutex_lock(&mutex_lista_pcb_exit);
		if(list_any_satisfy(lista_pcb_exit, (void*)__buscar_pcb)){
			elemento_encontrado = 1;
			un_pcb = list_find(lista_pcb_exit, (void*)__buscar_pcb);
		}
		pthread_mutex_unlock(&mutex_lista_pcb_exit);
	}
	if(elemento_encontrado == 0){
		//Si es que no se encontro en ninguna lista
		un_pcb = NULL;
		log_error(kernel_log_debug, "PID no encontrada en ninguna lista");
	}
	return un_pcb;
}

void validacion(int verf, int ver){
	// t_tcb* un_tcb = buscar_tcb_por_tid(verf, ver);
	// if(un_tcb != NULL){
	// 	if(un_tcb->inst != 0){
	// 		if(verif && ((ver == 1 && verf == 1) || (ver == 1 && verf == 2))){
	// 			un_tcb->temp += 300;
	// 			un_tcb->inst = 0;
	// 		}
	// 	}
	// }
}

t_pcb* buscar_y_remover_pcb_por_pid(int un_pid){
	t_pcb* un_pcb;
	int elemento_encontrado = 0;

	bool __buscar_pcb(t_pcb* void_pcb){
		if(void_pcb->pid == un_pid){
			return true;
		} else {
			return false;
		}
	};

	if(elemento_encontrado == 0){
		pthread_mutex_lock(&mutex_lista_pcb_new);
		if(list_any_satisfy(lista_pcb_new, (void*)__buscar_pcb)){
			elemento_encontrado = 1;

			un_pcb = list_find(lista_pcb_new, (void*)__buscar_pcb);		
			list_remove_element(lista_pcb_new, un_pcb);
			log_info(kernel_log_listas_pcb, "REMUEVO PROCESO %d DE LA LISTA NEW", un_pcb->pid);
		}
		pthread_mutex_unlock(&mutex_lista_pcb_new);
	}

	if(elemento_encontrado == 0){
		pthread_mutex_lock(&mutex_lista_pcb_enMemoria);
		if(list_any_satisfy(lista_pcb_enMemoria, (void*)__buscar_pcb)){
			elemento_encontrado = 1;
			un_pcb = list_find(lista_pcb_enMemoria, (void*)__buscar_pcb);
			list_remove_element(lista_pcb_enMemoria, un_pcb);

			log_info(kernel_log_listas_pcb, "REMUEVO PROCESO %d DE LA LISTA MEMORIA", un_pcb->pid);
		}
		pthread_mutex_unlock(&mutex_lista_pcb_enMemoria);
	}
	if(elemento_encontrado == 0){
		pthread_mutex_lock(&mutex_lista_pcb_exit);
		if(list_any_satisfy(lista_pcb_exit, (void*)__buscar_pcb)){
			elemento_encontrado = 1;
			un_pcb = list_find(lista_pcb_exit, (void*)__buscar_pcb);
		}
		pthread_mutex_unlock(&mutex_lista_pcb_exit);
	}
	if(elemento_encontrado == 0){
		//Si es que no se encontro en ninguna lista
		un_pcb = NULL;
		log_error(kernel_log_debug, "PID no encontrada en ninguna lista");
	}
	return un_pcb;
}

t_pcb* buscar_pcb_por_pid_en(int un_pid, t_list* lista_estado){
	t_pcb* un_pcb;

	bool __buscar_pcb(t_pcb* void_pcb){
		if(void_pcb->pid == un_pid){
			return true;
		} else {
			return false;
		}
	}

	if(list_any_satisfy(lista_estado, (void*)__buscar_pcb)){
		un_pcb = list_find(lista_estado, (void*)__buscar_pcb);
	}
	else{
		un_pcb = NULL;
	}
	return un_pcb;
}

void destruir_proceso(t_pcb* un_pcb){

	if(un_pcb != NULL){

		safe_free(un_pcb->archivo_instrucciones);
		list_destroy_and_destroy_elements(un_pcb->lista_mutex, (void*) destruir_mutex);
		list_destroy_and_destroy_elements(un_pcb->lista_id_hilo, (void*) safe_free);

		pthread_mutex_destroy(&(un_pcb->mutex_pcb));
		
		safe_free(un_pcb);
	}
}