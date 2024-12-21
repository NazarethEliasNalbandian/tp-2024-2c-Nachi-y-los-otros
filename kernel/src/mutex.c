#include "../src/mutex.h"
#include "../src/pcb.h"
#include "../src/tcb.h"
#include "../src/planificador_corto_plazo.h"

t_mutex_pcb* crear_mutex(char* nombre_recurso, int pid){
    t_mutex_pcb* nuevo_mutex = malloc(sizeof(t_mutex_pcb));

    nuevo_mutex->nombre_recurso = string_duplicate(nombre_recurso);
    nuevo_mutex->lista_bloqueados = list_create();
    nuevo_mutex->id_hilo_asignado = malloc(sizeof(t_hilo_id));
    nuevo_mutex->id_hilo_asignado->tid = -1;
    nuevo_mutex->id_hilo_asignado->pid_asociado = -1;

    pthread_mutex_init(&(nuevo_mutex->mutex_bloqueados), NULL);
    pthread_mutex_init(&(nuevo_mutex->mutex_id_hilo_asignado), NULL);
    pthread_mutex_init(&(nuevo_mutex->mutex_nombre_recurso), NULL);

    t_pcb* un_pcb = buscar_pcb_por_pid_en(pid, lista_pcb_enMemoria);

    pthread_mutex_lock(&(un_pcb->mutex_pcb));
    list_add(un_pcb->lista_mutex, nuevo_mutex);
    pthread_mutex_unlock(&(un_pcb->mutex_pcb));

    return nuevo_mutex;
}

t_mutex_pcb* obtener_mutex(char* nombre_recurso, int pid){

    t_pcb* un_pcb = buscar_pcb_por_pid_en(pid, lista_pcb_enMemoria);
    bool __buscar_mutex(t_mutex_pcb* void_mutex){
        pthread_mutex_lock(&(void_mutex->mutex_nombre_recurso));
		if(strcmp(void_mutex->nombre_recurso, nombre_recurso) == 0){
            pthread_mutex_unlock(&(void_mutex->mutex_nombre_recurso));
			return true;
		} else {
            pthread_mutex_unlock(&(void_mutex->mutex_nombre_recurso));
			return false;
		}
	};

	pthread_mutex_lock(&(un_pcb->mutex_pcb));
	t_mutex_pcb* un_mutex = list_find(un_pcb->lista_mutex, (void*) __buscar_mutex);
	pthread_mutex_unlock(&(un_pcb->mutex_pcb));

    if(un_mutex == NULL){
		log_error(kernel_log_debug, "MUTEX CON RECURSO: %s NO ENCONTRADO", nombre_recurso);
        return NULL;
    }

	return un_mutex;
}

bool esta_tomado_por(t_mutex_pcb* un_mutex, int tid, int pid){
    return un_mutex->id_hilo_asignado->tid == tid && un_mutex->id_hilo_asignado->pid_asociado == pid;
}

bool esta_tomado(t_mutex_pcb* un_mutex){
    return !esta_tomado_por(un_mutex, -1, -1);
}

void asignar_mutex(t_mutex_pcb* un_mutex, int tid, int pid){
    un_mutex->id_hilo_asignado->tid = tid;
    un_mutex->id_hilo_asignado->pid_asociado = pid;
}

void liberar_mutex(t_mutex_pcb* un_mutex){
    un_mutex->id_hilo_asignado->tid = -1;
    un_mutex->id_hilo_asignado->pid_asociado = -1;
}

bool remover_id_hilo_de_lista(t_list* lista, t_hilo_id* id_hilo) {
    bool coincide_hilo(void* elemento) {
        t_hilo_id* hilo = (t_hilo_id*) elemento;
        return (hilo->pid_asociado == id_hilo->pid_asociado) && (hilo->tid == id_hilo->tid);
    };

    t_hilo_id* hilo = list_remove_by_condition(lista, coincide_hilo);

    if(hilo != NULL)
        log_trace(kernel_log_debug, "HILO %d REMOVIDO DE PROCESO %d", hilo->tid, hilo->pid_asociado);
    else
        log_error(kernel_log_debug, "HILO %d DEL PROCESO %d NO PUDO SER REMOVIDO", id_hilo->tid, id_hilo->pid_asociado);

    bool encontrado = false;

    if(hilo != NULL){
        encontrado = true;
    }

    return encontrado;
}


void remover_hilo_de_mutex(t_mutex_pcb* un_mutex, t_hilo_id* id_hilo) {
    pthread_mutex_lock(&(un_mutex->mutex_bloqueados));

    // Utiliza la función auxiliar para remover el hilo de la lista
    bool encontrado = remover_id_hilo_de_lista(un_mutex->lista_bloqueados, id_hilo);

    pthread_mutex_unlock(&(un_mutex->mutex_bloqueados));

    if (encontrado) {
        log_trace(kernel_log_debug, "REMOVI EL PID %d TID %d DEL MUTEX %s", id_hilo->pid_asociado, id_hilo->tid, un_mutex->nombre_recurso);
    }
}


void desocupar_mutex(t_tcb* un_tcb){
    for (int i = 0; i < list_size(un_tcb->lista_mutex_asignados); i++) {
        t_mutex_pcb* un_mutex = list_get(un_tcb->lista_mutex_asignados, i);
        remover_hilo_de_mutex(un_mutex, un_tcb->id_hilo);
    }
}

void protocolo_liberacion_recurso(t_mutex_pcb* un_mutex){

   if(list_is_empty(un_mutex->lista_bloqueados)){ // SI NO HAY OTRO TCB QUE HAYA PEDIDO EL RECURSO, LO SETEO COMO NO ASIGNADO
        liberar_mutex(un_mutex);
    }else{ // SI HAY OTRO TCB QUE HAYA PEDIDO EL RECURSO, SE LO ASIGNO AL PRIMERO QUE LO HABIA SOLICITADO Y LO DESBLOQUEO
        pthread_mutex_lock(&(un_mutex->mutex_bloqueados));
        t_hilo_id* id_hilo_a_liberar = list_remove(un_mutex->lista_bloqueados, 0);
        pthread_mutex_unlock(&(un_mutex->mutex_bloqueados));

        log_trace(kernel_log_debug, "SIGUIENTE PID %d TID %d A LIBERAR BLOQUEADO POR EL MUTEX %s", id_hilo_a_liberar->pid_asociado, id_hilo_a_liberar->tid, un_mutex->nombre_recurso);

        desbloquear_hilo(id_hilo_a_liberar);

        t_tcb* tcb_a_liberar = buscar_tcb_por_tid(id_hilo_a_liberar->tid, id_hilo_a_liberar->pid_asociado);

        pthread_mutex_lock(&(tcb_a_liberar->mutex_tcb));
        list_add(tcb_a_liberar->lista_mutex_asignados, un_mutex);
        pthread_mutex_unlock(&(tcb_a_liberar->mutex_tcb));

        asignar_mutex(un_mutex,id_hilo_a_liberar->tid,id_hilo_a_liberar->pid_asociado);

        safe_free(id_hilo_a_liberar);
    }
}

void remover_recurso_asignado(t_mutex_pcb* un_mutex, t_tcb* tcb_que_libera_recurso){
    bool __buscar_mutex(t_mutex_pcb* void_mutex){
        pthread_mutex_lock(&(void_mutex->mutex_nombre_recurso));
		if(strcmp(void_mutex->nombre_recurso, un_mutex->nombre_recurso) == 0){
            pthread_mutex_unlock(&(void_mutex->mutex_nombre_recurso));
			return true;
		} else {
            pthread_mutex_unlock(&(void_mutex->mutex_nombre_recurso));
			return false;
		}
	};

    // REMUEVO EL RECURSO QUE ESTABA ASIGNADO AL HILO
    pthread_mutex_lock(&(tcb_que_libera_recurso->mutex_tcb));
    list_remove_by_condition(tcb_que_libera_recurso->lista_mutex_asignados, (void*) __buscar_mutex);
    pthread_mutex_unlock(&(tcb_que_libera_recurso->mutex_tcb));
}

void atender_liberacion_de_recurso(t_mutex_pcb* un_mutex, t_tcb* tcb_que_libera_recurso){

    remover_recurso_asignado(un_mutex, tcb_que_libera_recurso);
    protocolo_liberacion_recurso(un_mutex);
}

void destruir_lista_bloqueados(t_mutex_pcb* un_mutex){
	pthread_mutex_lock(&(un_mutex->mutex_bloqueados));
	list_destroy_and_destroy_elements(un_mutex->lista_bloqueados, (void*) safe_free);
	pthread_mutex_unlock(&(un_mutex->mutex_bloqueados));
}

void destruir_mutex(t_mutex_pcb* un_mutex){
    safe_free(un_mutex->nombre_recurso);
    safe_free(un_mutex->id_hilo_asignado);

    destruir_lista_bloqueados(un_mutex);

    pthread_mutex_destroy(&(un_mutex->mutex_bloqueados));
    pthread_mutex_destroy(&(un_mutex->mutex_id_hilo_asignado));

    safe_free((un_mutex));
}