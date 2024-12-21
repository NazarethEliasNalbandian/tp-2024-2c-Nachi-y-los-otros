#include "../src/finalizar_kernel.h"
#include "../src/pcb.h"
#include "../src/tcb.h"
#include "../src/cola_multinivel.h"

void finalizar_kernel(){
	_finalizar_config();
	// eliminar_tcbs();
	eliminar_pcbs();
	_destruir_conexiones();
	_finalizar_semaforos();
	_finalizar_pthread();
    safe_free(rta_creacion_proceso);
    safe_free(rta_creacion_hilo);
    safe_free(rta_finalizacion_proceso);
    safe_free(rta_finalizacion_hilo);
    safe_free(rta_dumpeo);
	_finalizar_logger();
}

void _finalizar_logger(){
	log_destroy(kernel_log_obligatorio);
    kernel_log_obligatorio = NULL;
	log_destroy(kernel_log_debug);
	kernel_log_debug = NULL;
    log_destroy(kernel_log_largo_plazo);
    kernel_log_largo_plazo = NULL;
    log_destroy(kernel_log_listas_pcb);
    kernel_log_listas_pcb = NULL;
    log_destroy(kernel_log_listas_tcb);
    kernel_log_listas_tcb = NULL;
    log_destroy(kernel_log_listas_io);
    kernel_log_listas_io = NULL;
}

void _finalizar_config(){
	config_destroy(kernel_config);
}

void _finalizar_semaforos(){
	sem_destroy(&llego_respuesta_creacion_proceso);
    sem_destroy(&llego_respuesta_creacion_hilo);
	sem_destroy(&llego_respuesta_finalizacion_hilo);
    sem_destroy(&llego_respuesta_finalizacion_proceso);
    sem_destroy(&llego_respuesta_dumpeo);
    sem_destroy(&enviar_interrupcion_quantum);
    sem_destroy(&finalizo_proceso_inicial);
    sem_destroy(&hilo_desalojado);
}

void _finalizar_pthread(){
	pthread_mutex_destroy(&mutex_lista_pcb_new);
    pthread_mutex_destroy(&mutex_lista_pcb_enMemoria);
    pthread_mutex_destroy(&mutex_lista_pcb_exit);

	pthread_mutex_destroy(&mutex_lista_tcb_ready);
	pthread_mutex_destroy(&mutex_lista_tcb_exec);
	pthread_mutex_destroy(&mutex_lista_tcb_blocked);
	pthread_mutex_destroy(&mutex_lista_tcb_exit);

    pthread_mutex_destroy(&mutex_lista_cola_multinivel);

	pthread_mutex_destroy(&mutex_process_id);
	pthread_mutex_destroy(&mutex_verificador);

    pthread_mutex_destroy(&mutex_lista_en_espera_de_io);

    pthread_mutex_destroy(&mutex_rta_creacion_proceso);
	pthread_mutex_destroy(&mutex_rta_creacion_hilo);
	pthread_mutex_destroy(&mutex_rta_finalizacion_hilo);
	pthread_mutex_destroy(&mutex_rta_finalizacion_proceso);
	pthread_mutex_destroy(&mutex_rta_dumpeo);
}

void _destruir_conexiones(){
	liberar_conexion(fd_cpu_dispatch);
	liberar_conexion(fd_cpu_interrupt);
}

void eliminar_tcbs(){
    // Liberar todos los TCBs en cada lista
    if (lista_tcb_ready != NULL) 
        list_destroy_and_destroy_elements(lista_tcb_ready, (void*)destruir_tcb);
    
    if (lista_tcb_exec != NULL) 
        list_destroy_and_destroy_elements(lista_tcb_exec, (void*)destruir_tcb);
    
    if (lista_tcb_blocked != NULL) 
        list_destroy_and_destroy_elements(lista_tcb_blocked, (void*)destruir_tcb);
    
    if (lista_tcb_exit != NULL)
        list_destroy_and_destroy_elements(lista_tcb_exit, (void*)destruir_tcb);
    
    if(lista_cola_multinivel != NULL)
        list_destroy_and_destroy_elements(lista_cola_multinivel, (void*) destruir_cola_multinivel);

    list_destroy_and_destroy_elements(lista_en_espera_de_io, (void*) safe_free);
}

void eliminar_pcbs(){
    // Liberar todos los TCBs en cada lista
    if (lista_pcb_new != NULL) {
        list_destroy_and_destroy_elements(lista_pcb_new, (void*)destruir_proceso);
    }
    if (lista_pcb_enMemoria != NULL) {
        list_destroy_and_destroy_elements(lista_pcb_enMemoria, (void*)destruir_proceso);
    }
    if (lista_pcb_exit != NULL) {
        list_destroy_and_destroy_elements(lista_pcb_exit, (void*)destruir_proceso);
    }
}
