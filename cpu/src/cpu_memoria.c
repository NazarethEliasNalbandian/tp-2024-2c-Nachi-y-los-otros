#include "../src/cpu_memoria.h"
#include "../src/ciclo_instruccion.h"
#include "../src/servicios_cpu.h"

void atender_cpu_memoria(){
    bool control_key=1;
    while (control_key) {
		int cod_op = recibir_operacion(fd_memoria);
		t_buffer* unBuffer = NULL;
		switch (cod_op) {
			case PETICION_CONTEXTO_CPU_MEMORIA:
				unBuffer = recibir_paquete(fd_memoria);
				safe_free(rta_peticion_contexto);
				rta_peticion_contexto = recibir_string_del_buffer(unBuffer);

				if((strcmp(rta_peticion_contexto, "ERROR") == 0)){
					hay_exit = true;
					TIPO_EXIT = THREAD;
					destruir_buffer(unBuffer);
					log_error(cpu_log_debug, "NO SE PUDO OBTENER EL CONTEXTO");
					sem_post(&llego_contexto);
				} else{
					pthread_mutex_lock(&mutex_manejo_contexto);
					recibir_contexto(unBuffer, un_contexto); // LLEGO EL CONTEXTO
					pthread_mutex_lock(&mutex_VERIFICADOR);
					if(un_contexto != NULL)
						un_contexto->verificador = VERIFICADOR;
					pthread_mutex_unlock(&mutex_VERIFICADOR);
					pthread_mutex_unlock(&mutex_manejo_contexto);
					destruir_buffer(unBuffer);

					sem_post(&llego_contexto);
					sem_post(&llego_contexto_una_vez);
				}
				break;
			case ACTUALIZACION_CONTEXTO_CPU_MEMORIA:
				unBuffer = recibir_paquete(fd_memoria);
				safe_free(rta_actualizacion_contexto);
				rta_actualizacion_contexto = recibir_string_del_buffer(unBuffer);
				destruir_buffer(unBuffer);
				// log_trace(cpu_log_debug, "LLEGO LO SIGUIENTE EN ACTUALIZAR CONTEXTO: %s", rta_actualizacion_contexto);
				if(strcmp(rta_actualizacion_contexto,"OK") == 0)
					sem_post(&contexto_actualizado);
				else	
					log_error(cpu_log_debug,"ERROR AL ACTUALIZAR CONTEXTO");
				break;
			case PETICION_DE_INSTRUCCION_CPU_MEMORIA: // FALTARIA CASO EN EL QUE LA INSTRUCCION NO EXISTA
				sem_wait(&termino_ciclo_instruccion);
				unBuffer = recibir_paquete(fd_memoria);
				char* instruccion_actual_string = recibir_string_del_buffer(unBuffer); 
				destruir_buffer(unBuffer);
				pthread_mutex_lock(&mutex_instruccion_split);

				if(instruccion_split != NULL){
					string_array_destroy(instruccion_split);
					instruccion_split = NULL;
				}

				instruccion_split = string_split(instruccion_actual_string, " ");
				safe_free(instruccion_actual_string);
				sem_post(&sem_fetch);
				pthread_mutex_unlock(&mutex_instruccion_split);
				break;
			case LECTURA_CPU_MEMORIA: // FALTARIA CASO DE ERROR DE LECTURA
				unBuffer = recibir_paquete(fd_memoria);
				valor_leido = recibir_uint32_del_buffer(unBuffer);
				destruir_buffer(unBuffer);
				sem_post(&sem_val_leido);
				break;
			case ESCRITURA_CPU_MEMORIA:
				unBuffer = recibir_paquete(fd_memoria);
				safe_free(rta_escritura);
				rta_escritura = recibir_string_del_buffer(unBuffer);
				destruir_buffer(unBuffer);
				if(strcmp(rta_escritura, "OK") != 0)
					log_error(cpu_log_debug, "ERROR DE ESCRITURA");
				sem_post(&sem_val_escrito);
				break;
			case -1:
				printf("Desconexión de Memoria\n");
				control_key=0;
				break;
			default:
				log_warning(cpu_log_debug,"Operacion desconocida de Memoria.");
				break;
		}
	}
}

void solicitar_contexto_a_memoria(int pid_en_ejecucion,int tid_en_ejecucion){
	t_paquete* un_paquete = crear_paquete(PETICION_CONTEXTO_CPU_MEMORIA);
	cargar_int_al_paquete(un_paquete, pid_en_ejecucion);
	cargar_int_al_paquete(un_paquete, tid_en_ejecucion);
	enviar_paquete(un_paquete, fd_memoria);
	eliminar_paquete(un_paquete);

	log_info(cpu_log_obligatorio, "## TID: %d - Solicito Contexto Ejecución", tid_en_ejecucion);
}

void solicitar_base_actual_a_memoria(int pid_en_ejecucion,int tid_en_ejecucion){
	t_paquete* un_paquete = crear_paquete(SOLICITAR_BASE_CPU_MEMORIA);
	cargar_int_al_paquete(un_paquete, pid_en_ejecucion);
	cargar_int_al_paquete(un_paquete, tid_en_ejecucion);
	enviar_paquete(un_paquete, fd_memoria);
	eliminar_paquete(un_paquete);
}