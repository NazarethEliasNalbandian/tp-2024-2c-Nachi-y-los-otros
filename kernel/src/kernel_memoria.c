#include "../src/kernel_memoria.h"

void atender_kernel_memoria(int fd_memoria){

	// log_error(kernel_log_debug, "ESPERANDO MENSAJE DE MEMORIA");
	int cod_op = recibir_operacion(fd_memoria);
	log_info(kernel_log_debug, "Se recibio el siguiente codigo de operacion: %s", convertirCodOpAString(cod_op));
	t_buffer* unBuffer = NULL;
	switch (cod_op) {
		case CREACION_PROCESO_KERNEL_MEMORIA:
			unBuffer = recibir_paquete(fd_memoria);

			pthread_mutex_lock(&mutex_rta_creacion_proceso);
			safe_free(rta_creacion_proceso);
			rta_creacion_proceso = recibir_string_del_buffer(unBuffer);
			pthread_mutex_unlock(&mutex_rta_creacion_proceso);
			destruir_buffer(unBuffer);
			sem_post(&llego_respuesta_creacion_proceso);
			break;
		case CREACION_HILO_KERNEL_MEMORIA:
			unBuffer = recibir_paquete(fd_memoria);

			pthread_mutex_lock(&mutex_rta_creacion_hilo);
			safe_free(rta_creacion_hilo);
			rta_creacion_hilo = recibir_string_del_buffer(unBuffer);

			if(strcmp(rta_creacion_hilo,"OK") != 0){
					log_error(kernel_log_debug, "NO SE PUDO CREAR EL HILO");
					exit(EXIT_FAILURE);
				}
			pthread_mutex_unlock(&mutex_rta_creacion_hilo);

			destruir_buffer(unBuffer);
			sem_post(&llego_respuesta_creacion_hilo);
			break;
		case FINALIZACION_HILO_KERNEL_MEMORIA:
			unBuffer = recibir_paquete(fd_memoria);

			pthread_mutex_lock(&mutex_rta_finalizacion_hilo);
			safe_free(rta_finalizacion_hilo);
			rta_finalizacion_hilo = recibir_string_del_buffer(unBuffer);
			pthread_mutex_unlock(&mutex_rta_finalizacion_hilo);

			destruir_buffer(unBuffer);
			sem_post(&llego_respuesta_finalizacion_hilo);
			break;
		case FINALIZACION_PROCESO_KERNEL_MEMORIA:
			unBuffer = recibir_paquete(fd_memoria);

			pthread_mutex_lock(&mutex_rta_finalizacion_proceso);
			safe_free(rta_finalizacion_proceso);
			rta_finalizacion_proceso = recibir_string_del_buffer(unBuffer);
			log_trace(kernel_log_debug, "LLEGO RTA FINALIZACION PROCESO: %s", rta_finalizacion_proceso);
			pthread_mutex_unlock(&mutex_rta_finalizacion_proceso);

			destruir_buffer(unBuffer);
			sem_post(&llego_respuesta_finalizacion_proceso);
			break;
		case DUMP_MEMORY_KERNEL_MEMORIA:
			unBuffer = recibir_paquete(fd_memoria);

			pthread_mutex_lock(&mutex_rta_dumpeo);
			safe_free(rta_dumpeo);
			rta_dumpeo = recibir_string_del_buffer(unBuffer);
			pthread_mutex_unlock(&mutex_rta_dumpeo);

			destruir_buffer(unBuffer);
			sem_post(&llego_respuesta_dumpeo);
			break;
		case -1:
			printf("Desconexión de Memoria\n");
			break;
		default:
			log_warning(kernel_log_debug,"OPERACION DESCONOCIDA DE MEMORIA");
			break;
	}

	liberar_conexion(fd_memoria);
}

void enviar_y_recibir_peticion_creacion_proceso(int pid, int tamanio){

	int fd_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);

	t_paquete* un_paquete = crear_paquete(CREACION_PROCESO_KERNEL_MEMORIA);
	cargar_int_al_paquete(un_paquete, pid);
	cargar_int_al_paquete(un_paquete, tamanio);

	enviar_paquete(un_paquete, fd_memoria);
	eliminar_paquete(un_paquete);

	atender_kernel_memoria(fd_memoria);
}

void enviar_y_recibir_peticion_creacion_hilo(int pid, int tid, char* archivo_instrucciones){

	int fd_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);

	t_paquete* un_paquete = crear_paquete(CREACION_HILO_KERNEL_MEMORIA);
	cargar_int_al_paquete(un_paquete, pid);
	cargar_int_al_paquete(un_paquete, tid);
	cargar_string_al_paquete(un_paquete, archivo_instrucciones);
	enviar_paquete(un_paquete, fd_memoria);
	eliminar_paquete(un_paquete);

	atender_kernel_memoria(fd_memoria);
}

void avisar_a_memoria_para_liberar_estructuras_hilo(int tid, int pid){
	int fd_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);

	t_paquete* un_paquete = crear_paquete(FINALIZACION_HILO_KERNEL_MEMORIA);
	cargar_int_al_paquete(un_paquete, pid);
	cargar_int_al_paquete(un_paquete, tid);
	
	enviar_paquete(un_paquete, fd_memoria);
	eliminar_paquete(un_paquete);

	atender_kernel_memoria(fd_memoria);

	log_trace(kernel_log_debug, "SOLICITO LIBERACION PID %d TID %d", pid, tid);
}

void avisar_a_memoria_para_liberar_estructuras_proceso(int pid){
	int fd_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);

	t_paquete* un_paquete = crear_paquete(FINALIZACION_PROCESO_KERNEL_MEMORIA);
	cargar_int_al_paquete(un_paquete, pid);

	enviar_paquete(un_paquete, fd_memoria);
	eliminar_paquete(un_paquete);

	atender_kernel_memoria(fd_memoria);

	log_trace(kernel_log_debug, "SOLICITO LIBERACION PID %d", pid);
}

void avisar_a_memoria_para_dumpeo(int tid, int pid){
	int fd_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);

	t_paquete* un_paquete = crear_paquete(DUMP_MEMORY_KERNEL_MEMORIA);
	cargar_int_al_paquete(un_paquete, pid);
	cargar_int_al_paquete(un_paquete, tid);

	enviar_paquete(un_paquete, fd_memoria);
	eliminar_paquete(un_paquete);

	atender_kernel_memoria(fd_memoria);
}