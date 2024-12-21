#include "../src/tcb.h"
#include "../src/planificador_largo_plazo.h"
#include "../src/planificador_corto_plazo.h"
#include "../src/servicios_kernel.h"
#include "../src/cola_multinivel.h"
#include "../src/kernel_memoria.h"
#include "../src/pcb.h"

void crear_tcb(t_pcb* un_pcb, char* archivo_instrucciones, int prioridad){


    t_tcb* nuevo_tcb = malloc(sizeof(t_tcb));

	if(un_pcb->siguiente_tid == 0){
		nuevo_tcb->id_hilo = list_get(un_pcb->lista_id_hilo, 0);
	}else{
		t_hilo_id* nuevo_id_hilo = malloc(sizeof(t_hilo_id));
		nuevo_tcb->id_hilo = nuevo_id_hilo;
		nuevo_tcb->id_hilo->tid = un_pcb->siguiente_tid;
		nuevo_tcb->id_hilo->pid_asociado = un_pcb->pid;
	}

    nuevo_tcb->prioridad = prioridad;
	nuevo_tcb->archivo_instrucciones = string_duplicate(archivo_instrucciones);

    nuevo_tcb->verificador = 0;
	nuevo_tcb->inst = 1;

    un_pcb->siguiente_tid += 1;
	
	nuevo_tcb->tids_en_espera_por_join = list_create();
	nuevo_tcb->lista_mutex_asignados = list_create();
	nuevo_tcb->temp = QUANTUM;

	pthread_mutex_init(&(nuevo_tcb->mutex_tcb), NULL);

	enviar_y_recibir_peticion_creacion_hilo(nuevo_tcb->id_hilo->pid_asociado, nuevo_tcb->id_hilo->tid, nuevo_tcb->archivo_instrucciones);
	sem_wait(&llego_respuesta_creacion_hilo);
    
	if(nuevo_tcb->id_hilo->tid != 0)
	{
		list_add(un_pcb->lista_id_hilo, nuevo_tcb->id_hilo);
	}

	if(ALGORITMO_PLANIFICACION == CMN)
		crear_colamultinivel(nuevo_tcb->prioridad);

	agregar_hilo_a_lista_ready(nuevo_tcb);

	log_info(kernel_log_obligatorio, "## (%d:%d) Se crea el Hilo - Estado: %s", un_pcb->pid, nuevo_tcb->id_hilo->tid, estado_tcb_to_string(nuevo_tcb->estado));
	log_info(kernel_log_largo_plazo, "## (%d:%d) Se crea el Hilo - Estado: %s", un_pcb->pid, nuevo_tcb->id_hilo->tid, estado_tcb_to_string(nuevo_tcb->estado));

	validacion(nuevo_tcb->id_hilo->tid, nuevo_tcb->id_hilo->pid_asociado);

	// MANDO EL HILO A PLANIFICAR
    ejecutar_en_un_hilo_nuevo_detach((void*)pcp_planificador_corto_plazo, NULL);
}

char* estado_tcb_to_string(estado_tcb estado){
		switch (estado) {
			case READY:
				return "READY";
			case EXEC:
				return "EXEC";
			case BLOCKED:
				return "BLOCKED";
			case EXIT:
				return "EXIT";
		default:
			log_error(kernel_log_debug, "No se reconocio el nombre del estado");
			return "UNKNOWN";
	}
}

void destruir_lista_tid_a_esperar(t_tcb* un_tcb){
	pthread_mutex_lock(&(un_tcb->mutex_tcb));
	list_destroy_and_destroy_elements(un_tcb->tids_en_espera_por_join, (void*) safe_free);
	pthread_mutex_unlock(&(un_tcb->mutex_tcb));
}

void destruir_tcb(t_tcb* un_tcb){

	if(un_tcb != NULL){

		// pthread_mutex_lock(&(un_tcb->mutex_tcb));
		safe_free(un_tcb->archivo_instrucciones);
		destruir_lista_tid_a_esperar(un_tcb);
		list_destroy(un_tcb->lista_mutex_asignados);
		// pthread_mutex_unlock(&(un_tcb->mutex_tcb));

		pthread_mutex_destroy(&(un_tcb->mutex_tcb));

		safe_free(un_tcb);
	}
}

void enviar_tcb_CPU_dispatch(t_tcb* un_tcb){
	pthread_mutex_lock(&(un_tcb->mutex_tcb));
	t_paquete* un_paquete = crear_paquete(EJECUTAR_HILO_KERNEL_CPU);
	cargar_int_al_paquete(un_paquete, un_tcb->id_hilo->pid_asociado);
	cargar_int_al_paquete(un_paquete, un_tcb->id_hilo->tid);
	cargar_int_al_paquete(un_paquete, un_tcb->verificador);
	pthread_mutex_unlock(&(un_tcb->mutex_tcb));
	enviar_paquete(un_paquete, fd_cpu_dispatch);
	eliminar_paquete(un_paquete);
}

t_tcb* obtener_hilo_desalojado(int un_pid, int un_tid, int verificador){

	t_tcb* un_tcb = buscar_tcb_por_tid_de(un_tid, un_pid, lista_tcb_exec, mutex_lista_tcb_exec);


    if(un_tcb==NULL){
		log_error(kernel_log_debug, "PID %d TID %d NO ENCONTRADO", un_pid, un_tid);
        return NULL;
    }

	pthread_mutex_lock(&(un_tcb->mutex_tcb));
	un_tcb->verificador = verificador;
	pthread_mutex_unlock(&(un_tcb->mutex_tcb));

	return un_tcb;
}

bool verificador(){
	return verf[15] == 95;
}

t_tcb* buscar_y_remover_tcb_por_tid(int un_tid, int un_pid) {
    t_tcb* tcb_encontrado = NULL;

	if(ALGORITMO_PLANIFICACION != CMN){

		tcb_encontrado = buscar_y_remover_tcb_por_tid_de(un_tid, un_pid, lista_tcb_ready, mutex_lista_tcb_ready, READY);
		if (tcb_encontrado != NULL) {
			return tcb_encontrado;
		}
	}else{
		tcb_encontrado = buscar_y_remover_tcb_de_cola_multinivel(un_tid, un_pid);
		if (tcb_encontrado != NULL) {
			return tcb_encontrado;
		}
	}

    tcb_encontrado = buscar_y_remover_tcb_por_tid_de(un_tid, un_pid, lista_tcb_exec, mutex_lista_tcb_exec, EXEC);
    if (tcb_encontrado != NULL) {
        return tcb_encontrado;
    }

    tcb_encontrado = buscar_y_remover_tcb_por_tid_de(un_tid, un_pid, lista_tcb_blocked, mutex_lista_tcb_blocked, BLOCKED);
    if (tcb_encontrado != NULL) {
        return tcb_encontrado;
    }

    return NULL;
}

// NO BUSCA EN EXIT
t_tcb* buscar_tcb_por_tid(int un_tid, int un_pid) {
    t_tcb* tcb_encontrado = NULL;

	if(ALGORITMO_PLANIFICACION != CMN){

		tcb_encontrado = buscar_tcb_por_tid_de(un_tid, un_pid, lista_tcb_ready, mutex_lista_tcb_ready);
		if (tcb_encontrado != NULL) {
			return tcb_encontrado;
		}
	}else{
		tcb_encontrado = buscar_tcb_en_cola_multinivel(un_tid, un_pid);
		if (tcb_encontrado != NULL) {
			return tcb_encontrado;
		}
	}

    tcb_encontrado = buscar_tcb_por_tid_de(un_tid, un_pid, lista_tcb_exec, mutex_lista_tcb_exec);
    if (tcb_encontrado != NULL) {
        return tcb_encontrado;
    }

    tcb_encontrado = buscar_tcb_por_tid_de(un_tid, un_pid, lista_tcb_blocked, mutex_lista_tcb_blocked);
    if (tcb_encontrado != NULL) {
        return tcb_encontrado;
    }

    return NULL;
}

// DEVUELVE TRUE SI EL HILO ESTA EN READY, BLOCKED O EXEC, SINO DEVUELVE FALSE
bool existe_hilo(int un_tid, int un_pid){
	t_tcb* un_tcb;
	un_tcb = buscar_tcb_por_tid(un_tid, un_pid);
	return un_tcb != NULL;
}


t_tcb* buscar_tcb_por_tid_de(int un_tid, int un_pid, t_list* lista, pthread_mutex_t mutex_lista) {
    t_tcb* un_tcb = NULL;

    bool __buscar_tcb(t_tcb* void_tcb) {
        return (void_tcb->id_hilo->tid == un_tid) && (void_tcb->id_hilo->pid_asociado == un_pid);
		
    };

    pthread_mutex_lock(&mutex_lista);
    if (list_any_satisfy(lista, (void*)__buscar_tcb)) {
        un_tcb = list_find(lista, (void*)__buscar_tcb);
    }
    pthread_mutex_unlock(&mutex_lista);

    return un_tcb;
}

void remover_tcb_de_lista(t_tcb* un_tcb, t_list* lista, pthread_mutex_t mutex_lista) {
    if (un_tcb != NULL) {
        pthread_mutex_lock(&mutex_lista);
        list_remove_element(lista, un_tcb);
        pthread_mutex_unlock(&mutex_lista);
    }
}

t_tcb* buscar_y_remover_tcb_por_tid_de(int un_tid, int un_pid, t_list* lista, pthread_mutex_t mutex_lista, estado_tcb estado_hilo) {
    t_tcb* un_tcb = buscar_tcb_por_tid_de(un_tid, un_pid, lista, mutex_lista);
    
    if (un_tcb != NULL) {
        remover_tcb_de_lista(un_tcb, lista, mutex_lista);

		if(!(ALGORITMO_PLANIFICACION == CMN && (estado_hilo == READY)))
			log_info(kernel_log_listas_tcb,"PID %d TID %d REMOVIDO DE LISTA %s", un_pid, un_tid, estado_tcb_to_string(estado_hilo));
    }

    return un_tcb;
}
