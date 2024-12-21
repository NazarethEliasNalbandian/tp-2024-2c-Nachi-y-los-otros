#include "../src/finalizar_cpu.h"

void finalizar_cpu(){
	destruir_semaforos();
	destruir_pthreads();
	destruir_configs();
	liberar_punteros();
	liberar_sockets();
	// safe_free(rta_peticion_contexto);
	// safe_free(rta_actualizacion_contexto);
	// safe_free(rta_escritura);
	// safe_free(rta_mutex_create);
	// safe_free(rta_mutex_lock);
	// safe_free(rta_mutex_unlock);
	// safe_free(rta_thread_create);
	// safe_free(rta_process_create);
	// safe_free(rta_thread_cancel);
	// safe_free(rta_thread_join);
	destruir_logs();
}

void destruir_semaforos(){
	sem_destroy(&sem_fetch);
	sem_destroy(&sem_decode);
	sem_destroy(&sem_execute);
	sem_destroy(&sem_val_leido);
	sem_destroy(&sem_val_escrito);
	sem_destroy(&sem_rta_kernel);
	sem_destroy(&termino_ciclo_instruccion);
	sem_destroy(&llego_contexto);
	sem_destroy(&llego_contexto_una_vez);
	sem_destroy(&se_desconecto_kernel);
	sem_destroy(&contexto_actualizado);
	sem_destroy(&sem_rta_mutex_create);
    sem_destroy(&sem_rta_mutex_lock);
    sem_destroy(&sem_rta_mutex_unlock);
    sem_destroy(&sem_rta_thread_create);
    sem_destroy(&sem_rta_process_create);
    sem_destroy(&sem_rta_thread_cancel);
    sem_destroy(&sem_rta_thread_join);
	sem_destroy(&sem_hilo_en_ejecucion_desalojado);
}
void destruir_pthreads(){
	pthread_mutex_destroy(&mutex_interruptFlag);
	pthread_mutex_destroy(&mutex_manejo_contexto);
	pthread_mutex_destroy(&mutex_interrupt_motivo);
	pthread_mutex_destroy(&mutex_instruccion_split);
	pthread_mutex_destroy(&mutex_tipo_desalojo);
	pthread_mutex_destroy(&mutex_VERIFICADOR);
	pthread_mutex_destroy(&mutex_existe_hilo_a_joinear);
	pthread_mutex_destroy(&mutex_hubo_quantum);
}

void destruir_logs(){
	log_destroy(cpu_log_debug);
	cpu_log_debug = NULL;
	log_destroy(cpu_log_obligatorio);
	cpu_log_obligatorio = NULL;
}

void destruir_configs(){
	config_destroy(cpu_config);
}

void liberar_punteros(){
	while(string_array_is_empty(op_autorizada)){
		free(string_array_pop(op_autorizada));
	}

	safe_free(op_autorizada);
	// safe_free(un_contexto->r_cpu);
	// safe_free(un_contexto);
	safe_free(instruccion);
	safe_free(interrupt_motivo);

	if(instruccion_split != NULL){
		string_array_destroy(instruccion_split);
		instruccion_split = NULL;
	}
}

void liberar_sockets(){
	liberar_conexion(fd_kernel_dispatch);
	liberar_conexion(fd_kernel_interrupt);
	liberar_conexion(fd_memoria);
}