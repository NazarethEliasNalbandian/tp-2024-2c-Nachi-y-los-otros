#include "../src/servicios_kernel.h"
#include "../src/cola_multinivel.h"
#include "../src/finalizar_kernel.h"
#include "../src/pcb.h"

int generar_verificador(){
	int valor_verificador;
	pthread_mutex_lock(&mutex_verificador);
	var_verificador++;
	valor_verificador = var_verificador;
	pthread_mutex_unlock(&mutex_verificador);
	return valor_verificador;
}

char* algoritmo_to_string(t_algoritmo algoritmo){

	switch(algoritmo){
	case FIFO:
		return "FIFO";
		break;
	case PRIORIDADES:
		return "PRIORIDADES";
		break;
	case CMN:
		return "CMN";
		break;
	default:
		return "ERROR";
	}
}

t_mochila* crear_mochila() {
    t_mochila* mochila = malloc(sizeof(t_mochila));

    if (mochila == NULL) 
		return NULL;

    mochila->parametros = queue_create();
	mochila->cantidad_parametros_inicial = 0;

	pthread_mutex_init(&(mochila->mutex_mochila), NULL);

    return mochila;
}

void destruir_mochila(t_mochila* mochila){
    if (mochila == NULL) 
		return;

    if (mochila->parametros != NULL) {
        queue_destroy_and_destroy_elements(mochila->parametros, (void*) safe_free);
        mochila->parametros = NULL;
    }

	pthread_mutex_destroy(&(mochila->mutex_mochila));

	safe_free(mochila);
}

char* motivo_blocked_to_string(t_motivo_blocked motivo_blocked){

	switch(motivo_blocked){
		case PTHREAD_JOIN:
			return "PTHREAD_JOIN";
		case MUTEX:
			return "MUTEX";
		case _IO:
			return "IO";
		case _DUMP_MEMORY:
			return "DUMP_MEMORY";
		default:
			return "ERROR";
	}
}

void agregar_hilo_a_lista_ready(t_tcb* un_tcb){

	if(ALGORITMO_PLANIFICACION != CMN){ // SI EL ALGORITMO NO ES DE COLA MULTINIVEL, AGREGO EL TCB A LA LISTA READY QUE ES ÚNICA
		pthread_mutex_lock(&mutex_lista_tcb_ready);
		list_add(lista_tcb_ready, un_tcb);
		pthread_mutex_unlock(&mutex_lista_tcb_ready);
		log_info(kernel_log_listas_tcb, "PID %d TID %d AGREGADO A LISTA READY", un_tcb->id_hilo->pid_asociado, un_tcb->id_hilo->tid);
		
	}else{ // EN CASO DE QUE EL ALGORITMO SEA DE COLA MULTINIVEL, OBTENGO LA COLA CON LA PRIORIDAD DEL TCB Y LO AGREGO A ESA COLA 
		t_cola_multinivel* cola_seleccionada = obtener_cola_por_prioridad(un_tcb->prioridad);

		pthread_mutex_lock(&(cola_seleccionada->lista_ready_y_mutex->mutex_lista_ready));
		list_add(cola_seleccionada->lista_ready_y_mutex->lista_ready, un_tcb);
		pthread_mutex_unlock(&(cola_seleccionada->lista_ready_y_mutex->mutex_lista_ready));

		pthread_mutex_lock(&(cola_seleccionada->mutex_prioridad));
		log_info(kernel_log_listas_tcb, "PID %d TID %d AGREGADO A LA COLA MULTINIVEL DE PRIORIDAD: %d",  un_tcb->id_hilo->pid_asociado, un_tcb->id_hilo->tid, cola_seleccionada->prioridad_lista);

		log_error(kernel_log_debug, "TAMAÑO DE COLA MULTINIVEL DE PRIORIDAD %d: %d", cola_seleccionada->prioridad_lista, list_size(cola_seleccionada->lista_ready_y_mutex->lista_ready));
		pthread_mutex_unlock(&(cola_seleccionada->mutex_prioridad));
	}
	un_tcb->estado = READY;
}

void enviar_mensaje_a_CPU(op_code codigo_op, char* mensaje){
	t_paquete* un_paquete = crear_paquete(codigo_op);
	cargar_string_al_paquete(un_paquete, mensaje);
	enviar_paquete(un_paquete, fd_cpu_dispatch);
	eliminar_paquete(un_paquete);
}

void sighandler(int signal){
	// finalizar_kernel();
    printf("\nKERNEL SE FINALIZO CORRECTAMENTE...\n");
	exit(EXIT_SUCCESS);
}

void mostrarListaEsperaIO(){
	int i;

	for(i=0; i<list_size(lista_en_espera_de_io); i++){
		t_sleep* un_tcb_sleep = list_get(lista_en_espera_de_io, i);
		log_info(kernel_log_listas_io, "PROCESO %d EN LA POSICION %d DE LISTA ESPERA IO", un_tcb_sleep->tcb->id_hilo->pid_asociado, i);
	}
}

void mostrarListaPCBMemoria(){
	int i;

	for(i=0; i<list_size(lista_pcb_enMemoria); i++){
		t_pcb* un_pcb = list_get(lista_pcb_enMemoria, i);
		pthread_mutex_lock(&(un_pcb->mutex_pcb));
		log_error(kernel_log_debug, "PROCESO %d EN LA POSICION %d DE LISTA PCB EN MEMORIA", un_pcb->pid, i);
		pthread_mutex_unlock(&(un_pcb->mutex_pcb));
	}
}