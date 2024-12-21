#include "../src/memoria_kernel.h"
#include "../src/proceso.h"
#include "../src/servicios_memoria.h"
#include "../src/particion.h"
#include "../src/hilo.h"
#include "../src/espacio_usuario.h"
#include "../src/memoria_filesystem.h"

void atender_memoria_kernel(void* arg){

	t_args* args = (t_args*) arg;
	int socket = args->socket;

    safe_free(args);

	t_buffer* unBuffer = NULL;
	int cod_op = recibir_operacion(socket);
	switch (cod_op) {
		case HANDSHAKE_KERNEL_MEMORIA:
			unBuffer = recibir_paquete(socket);
			atender_handshake_kernel_memoria(unBuffer, socket);
			break;
		case CREACION_PROCESO_KERNEL_MEMORIA:
			unBuffer = recibir_paquete(socket);
			atender_creacion_proceso(unBuffer, socket);
			break;
		case CREACION_HILO_KERNEL_MEMORIA:
			unBuffer = recibir_paquete(socket);
			atender_creacion_hilo(unBuffer, socket);
			break;
		case FINALIZACION_HILO_KERNEL_MEMORIA:
			unBuffer = recibir_paquete(socket);
			atender_finalizacion_hilo(unBuffer, socket);
			break;
		case FINALIZACION_PROCESO_KERNEL_MEMORIA:
			unBuffer = recibir_paquete(socket);
			atender_finalizacion_proceso(unBuffer, socket);
			break;
		case DUMP_MEMORY_KERNEL_MEMORIA:
			unBuffer = recibir_paquete(socket);
			atender_dump_memory(unBuffer, socket);
			break;
		case -1:
			printf("Desconexión de Kernel\n");
			break;
		default:
			log_warning(memoria_log_debug,"OPERACION DESCONOCIDA DE KERNEL");
			break;
	}

	liberar_conexion(socket);
}

// ATENCIONES

void atender_handshake_kernel_memoria(t_buffer* unBuffer, int socket){
	char* mensaje = recibir_string_del_buffer(unBuffer);
	destruir_buffer(unBuffer);
	log_info(memoria_log_debug,"RECIBI LO SIGUIENTE: %s", mensaje);
	safe_free(mensaje);
}

void atender_creacion_proceso(t_buffer* unBuffer, int socket){
	int pid = recibir_int_del_buffer(unBuffer);
	int tamanio = recibir_int_del_buffer(unBuffer);
	destruir_buffer(unBuffer);

	t_particion* particion_asignada = obtener_particion_a_asignar(tamanio);

	if(particion_asignada != NULL){

		if(ESQUEMA == FIJAS){
			crear_proceso_esquema_fijo(pid, particion_asignada);
			log_info(memoria_log_obligatorio, "## Proceso Creado - PID: %d - Tamaño: %d", pid, tamanio);
			pthread_mutex_lock(&(particion_asignada->mutex_particion));
			log_info(memoria_log_particion, "Proceso %d creado de tamaño %d. Particion %d asignada de tamaño %d", pid, tamanio,  particion_asignada->particion_id, particion_asignada->tamanio);
			pthread_mutex_unlock(&(particion_asignada->mutex_particion));
		}

		if(ESQUEMA == DINAMICAS){
			crear_proceso_esquema_dinamico(pid, particion_asignada, tamanio);
			log_info(memoria_log_obligatorio, "## Proceso Creado - PID: %d - Tamaño: %d", pid, tamanio);
			pthread_mutex_lock(&(particion_asignada->mutex_particion));
			log_info(memoria_log_particion, "Proceso %d creado. Particion asignada con tamaño %d que va de %d a %d", pid, particion_asignada->tamanio, particion_asignada->base, particion_asignada->limite);
			pthread_mutex_unlock(&(particion_asignada->mutex_particion));
		}

		responder_OK(socket, CREACION_PROCESO_KERNEL_MEMORIA);
	}else {
		log_error(memoria_log_debug, "## Proceso no pudo ser creado - PID: %d - Tamaño: %d", pid, tamanio);
		responder_ERROR(socket, CREACION_PROCESO_KERNEL_MEMORIA);
	}
}

void atender_creacion_hilo(t_buffer* unBuffer, int socket){
	int pid = recibir_int_del_buffer(unBuffer);
	int tid = recibir_int_del_buffer(unBuffer);
	char* archivo_instrucciones = recibir_string_del_buffer(unBuffer);
	destruir_buffer(unBuffer);

	if(existe_proceso(pid)){
		crear_hilo(pid, tid, archivo_instrucciones);
		log_info(memoria_log_obligatorio, "## Hilo Creado - (PID:TID) - (%d:%d)", pid, tid);
		responder_OK(socket, CREACION_HILO_KERNEL_MEMORIA);
	}else {
		log_error(memoria_log_debug, "EL PROCESO DEL HILO NUNCA FUE CREADO");
		responder_ERROR(socket, CREACION_HILO_KERNEL_MEMORIA);
	}

	safe_free(archivo_instrucciones);
}

void atender_finalizacion_proceso(t_buffer* unBuffer, int socket){
	int pid = recibir_int_del_buffer(unBuffer);
	destruir_buffer(unBuffer);

	t_proceso* un_proceso = buscar_proceso(pid);

	pthread_mutex_lock(&(un_proceso->mutex_proceso));
	int process_id = un_proceso->pid;
	int particion_id = un_proceso->particion_asignada_id;
	pthread_mutex_unlock(&(un_proceso->mutex_proceso));

	t_particion* una_particion = buscar_particion_por_id(particion_id);

	pthread_mutex_lock(&(una_particion->mutex_particion));
	int tamanio = una_particion->tamanio;
	pthread_mutex_unlock(&(una_particion->mutex_particion));

	if(un_proceso != NULL && una_particion != NULL){
		finalizar_proceso(una_particion, un_proceso);

		if(ESQUEMA == DINAMICAS)
			unificar_particiones_libres_contiguas();

		log_info(memoria_log_obligatorio, "## Proceso Destruido - PID: %d - Tamaño: %d", process_id, tamanio);
		responder_OK(socket, FINALIZACION_PROCESO_KERNEL_MEMORIA);
	}else
		responder_ERROR(socket, FINALIZACION_PROCESO_KERNEL_MEMORIA);
}

void atender_finalizacion_hilo(t_buffer* unBuffer, int socket){
	int pid = recibir_int_del_buffer(unBuffer);
	int tid = recibir_int_del_buffer(unBuffer);
	destruir_buffer(unBuffer);

	t_hilo* un_hilo = buscar_hilo(pid, tid);

	pthread_mutex_lock(&mutex_lst_hilos_recibidos);
	bool encontrado = list_remove_element(list_hilos_recibidos, un_hilo);
	pthread_mutex_unlock(&mutex_lst_hilos_recibidos);

	if(encontrado){
		destruir_hilo(un_hilo);
		log_info(memoria_log_obligatorio, "## Hilo Destruido - (PID:TID) - (%d:%d)", pid, tid);
		responder_OK(socket, FINALIZACION_HILO_KERNEL_MEMORIA);
	}else
		responder_ERROR(socket, FINALIZACION_HILO_KERNEL_MEMORIA);
}

void atender_dump_memory(t_buffer* unBuffer, int socket){
	int pid = recibir_int_del_buffer(unBuffer);
	int tid = recibir_int_del_buffer(unBuffer);
	destruir_buffer(unBuffer);

	log_info(memoria_log_obligatorio, "## Memory Dump solicitado - (PID:TID) - (%d:%d)", pid, tid);

	t_proceso* un_proceso = buscar_proceso(pid);
	pthread_mutex_lock(&(un_proceso->mutex_proceso));
	int base = un_proceso->base;
	int limite = un_proceso->limite;
	pthread_mutex_unlock(&(un_proceso->mutex_proceso));

	t_particion* una_particion = buscar_particion_por_base_y_limite(base, limite);

	pthread_mutex_lock(&(una_particion->mutex_particion));
	pthread_mutex_lock(&(un_proceso->mutex_proceso));
	size_t tamanio = (size_t) (un_proceso->mayor_direccion_fisica_escrita - una_particion->base)+1;
	pthread_mutex_unlock(&(un_proceso->mutex_proceso));
	pthread_mutex_unlock(&(una_particion->mutex_particion));

	log_trace(memoria_log_debug, "TAM ESCRITO: %lu", tamanio);

	pthread_mutex_lock(&(una_particion->mutex_particion));
	int dir_fisica = una_particion->base;
	pthread_mutex_unlock(&(una_particion->mutex_particion));

	void* mensaje = leer_data_de_dir_fisica(pid, tid, dir_fisica, tamanio);

	pthread_mutex_lock(&(una_particion->mutex_particion));
	enviar_memory_dump_filesystem(pid, tid, mensaje, (size_t) una_particion->tamanio, tamanio);
	pthread_mutex_unlock(&(una_particion->mutex_particion));

	log_trace(memoria_log_debug, "ESPERANDO");
	sem_wait(&llego_respuesta_memory_dump_fs);
	log_trace(memoria_log_debug, "LLEGO: %s", respuesta_memory_dump_fs);

	if(strcmp(respuesta_memory_dump_fs, "OK") == 0)
		responder_OK(socket, DUMP_MEMORY_KERNEL_MEMORIA);
	else
		responder_ERROR(socket, DUMP_MEMORY_KERNEL_MEMORIA);
}