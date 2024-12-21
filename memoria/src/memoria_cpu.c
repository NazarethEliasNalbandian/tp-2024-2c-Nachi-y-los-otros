#include "../src/memoria_cpu.h"
#include "../src/hilo.h"
#include "../src/proceso.h"
#include "../src/servicios_memoria.h"
#include "../src/espacio_usuario.h"

void atender_memoria_cpu(){

	bool control_key = 1;

	while (control_key) {
		t_buffer* unBuffer = NULL;
		int cod_op = recibir_operacion(fd_cpu);
		switch (cod_op) {
			case HANDSHAKE_CPU_MEMORIA:
				unBuffer = recibir_paquete(fd_cpu);
				atender_handshake_cpu_memoria(unBuffer);
				break;
			case PETICION_CONTEXTO_CPU_MEMORIA:
				unBuffer = recibir_paquete(fd_cpu);
				atender_peticion_contexto(unBuffer);
				break;
			case ACTUALIZACION_CONTEXTO_CPU_MEMORIA:
				unBuffer = recibir_paquete(fd_cpu);
				atender_actualizacion_contexto(unBuffer);
				break;
			case PETICION_DE_INSTRUCCION_CPU_MEMORIA:
				unBuffer = recibir_paquete(fd_cpu);
				atender_peticion_instruccion(unBuffer);
				break;
			case LECTURA_CPU_MEMORIA:
				unBuffer = recibir_paquete(fd_cpu);
				atender_lectura(unBuffer);
				break;
			case ESCRITURA_CPU_MEMORIA:
				unBuffer = recibir_paquete(fd_cpu);
				atender_escritura(unBuffer);
				break;
			case -1:
				enviar_finalizacion_filesystem();
				printf("Desconexión de CPU\n");
				control_key = 0;
				sem_post(&se_desconecto_cpu);
				break;
			default:
				log_warning(memoria_log_debug,"OPERACION DESCONOCIDA DE CPU");
				break;
			}
	}
}

void atender_handshake_cpu_memoria(t_buffer* unBuffer){
	char* mensaje = recibir_string_del_buffer(unBuffer);
	destruir_buffer(unBuffer);
	log_info(memoria_log_debug, "RECIBI LO SIGUIENTE: %s",mensaje);
	safe_free(mensaje);
}

void enviar_finalizacion_filesystem(){
	int fd_filesystem = crear_conexion(IP_FILESYSTEM, PUERTO_FILESYSTEM);
	t_paquete* un_paquete = crear_paquete(FINALIZACION_MEMORIA_FILESYSTEM);
	cargar_int_al_paquete(un_paquete,1);
	enviar_paquete(un_paquete, fd_filesystem);
	eliminar_paquete(un_paquete);
}

void atender_peticion_contexto(t_buffer* unBuffer){
	int pid = recibir_int_del_buffer(unBuffer);
	int tid = recibir_int_del_buffer(unBuffer);
	destruir_buffer(unBuffer);

	t_hilo* un_hilo = buscar_hilo(pid, tid);

	t_proceso* un_proceso = buscar_proceso(pid);

	if(un_proceso == NULL || un_hilo == NULL){
		retardo_respuesta();
		responder_ERROR(fd_cpu, PETICION_CONTEXTO_CPU_MEMORIA);
		return;
	}

	pthread_mutex_lock(&(un_proceso->mutex_proceso));
	pthread_mutex_lock(&(un_hilo->mutex_hilo));
	log_info(memoria_log_obligatorio, "## Contexto Solicitado - (PID:TID) - (%d:%d)", un_proceso->pid, un_hilo->tid);
	pthread_mutex_unlock(&(un_hilo->mutex_hilo));
	pthread_mutex_unlock(&(un_proceso->mutex_proceso));

	t_contexto* un_contexto = malloc(sizeof(t_contexto));
	un_contexto->r_cpu = malloc(sizeof(t_registrosCPU));
	pthread_mutex_lock(&(un_hilo->mutex_hilo));
	un_contexto->pid = un_hilo->pid_asociado;
	un_contexto->tid = un_hilo->tid;
	un_contexto->maximo_PC = un_hilo->maximo_PC;
	un_contexto->r_cpu->base = un_proceso->base;
	un_contexto->r_cpu->limite = un_proceso->limite;
	cargar_registros_al_contexto(un_contexto, un_hilo->registros);
	pthread_mutex_unlock(&(un_hilo->mutex_hilo));
	enviar_contexto(un_contexto);

	safe_free(un_contexto->r_cpu);
	safe_free(un_contexto);
}

void atender_actualizacion_contexto(t_buffer* unBuffer){
	log_trace(memoria_log_debug, "ENTRE A ACTUALIZACION CONTEXTO");
	t_contexto* un_contexto = malloc(sizeof(t_contexto));
	un_contexto->r_cpu = malloc(sizeof(t_registrosCPU));
	recibir_contexto(unBuffer, un_contexto);
	destruir_buffer(unBuffer);

	t_hilo* un_hilo = buscar_hilo(un_contexto->pid, un_contexto->tid);

	if(un_hilo != NULL){
		pthread_mutex_lock(&(un_hilo->mutex_hilo));
		actualizar_contexto_hilo(un_hilo, un_contexto);
		pthread_mutex_unlock(&(un_hilo->mutex_hilo));
		log_info(memoria_log_obligatorio, "## Contexto Actualizado - (PID:TID) - (%d:%d)", un_contexto->pid, un_contexto->tid);
		retardo_respuesta();
		responder_OK(fd_cpu, ACTUALIZACION_CONTEXTO_CPU_MEMORIA);
	}
	else{
		retardo_respuesta();
		responder_ERROR(fd_cpu, ACTUALIZACION_CONTEXTO_CPU_MEMORIA);
	}

	safe_free(un_contexto->r_cpu);
	safe_free(un_contexto);
}

void atender_peticion_instruccion(t_buffer* unBuffer){
	int pid = recibir_int_del_buffer(unBuffer);
	int tid = recibir_int_del_buffer(unBuffer);
	u_int32_t programCounter = recibir_uint32_del_buffer(unBuffer);
	destruir_buffer(unBuffer);

	log_trace(memoria_log_debug, "PROGRAM COUNTER RECIBIDO: %d", programCounter);

	t_hilo* un_hilo = buscar_hilo(pid, tid);
	char* instruccion;
	char** instruccion_split;

	if(un_hilo != NULL){
		instruccion = obtener_instruccion_por_indice(un_hilo, programCounter);
		enviar_instruccion(instruccion);
		instruccion_split = string_split(instruccion, " ");
		loggear_instruccion_enviada(instruccion_split, pid, tid);
		string_array_destroy(instruccion_split);
	}
	else{
		retardo_respuesta();
		responder_ERROR(fd_cpu, PETICION_DE_INSTRUCCION_CPU_MEMORIA);
	}
}

void atender_lectura(t_buffer* unBuffer){
	int pid = recibir_int_del_buffer(unBuffer);
	int tid = recibir_int_del_buffer(unBuffer);
	int dir_fisica = recibir_int_del_buffer(unBuffer);
	destruir_buffer(unBuffer);

	uint32_t valor = leer_uint32_de_dir_fisica(pid, tid, dir_fisica);
	enviar_valor_por_lectura(valor);
}

void atender_escritura(t_buffer* unBuffer){
	int pid = recibir_int_del_buffer(unBuffer);
	int tid = recibir_int_del_buffer(unBuffer);
	int dir_fisica = recibir_int_del_buffer(unBuffer);
	uint32_t valor_a_escribir = recibir_uint32_del_buffer(unBuffer);
	destruir_buffer(unBuffer);

	t_proceso* un_proceso = buscar_proceso(pid);
	
	pthread_mutex_lock(&(un_proceso->mutex_proceso));
	if(dir_fisica > un_proceso->mayor_direccion_fisica_escrita)
		un_proceso->mayor_direccion_fisica_escrita = dir_fisica;
	
	pthread_mutex_unlock(&(un_proceso->mutex_proceso));

	escribir_uint32_en_dir_fisica(pid, tid, dir_fisica, &valor_a_escribir);
	
	retardo_respuesta();
	responder_OK(fd_cpu, ESCRITURA_CPU_MEMORIA);
}

void enviar_contexto(t_contexto* un_contexto){
	retardo_respuesta();
	t_paquete* un_paquete = crear_paquete(PETICION_CONTEXTO_CPU_MEMORIA);
	cargar_string_al_paquete(un_paquete, "OK");
	cargar_contexto_al_paquete(un_paquete, un_contexto);
	enviar_paquete(un_paquete, fd_cpu);
	eliminar_paquete(un_paquete);
}

void enviar_instruccion(char* instruccion){
	retardo_respuesta();
	t_paquete* un_paquete = crear_paquete(PETICION_DE_INSTRUCCION_CPU_MEMORIA);
	cargar_string_al_paquete(un_paquete, instruccion);
	enviar_paquete(un_paquete, fd_cpu);
	eliminar_paquete(un_paquete);
}

void enviar_valor_por_lectura(uint32_t valor){
	retardo_respuesta();
	t_paquete* un_paquete = crear_paquete(LECTURA_CPU_MEMORIA);
	cargar_uint32_al_paquete(un_paquete, valor);
	enviar_paquete(un_paquete, fd_cpu);
	eliminar_paquete(un_paquete);
}