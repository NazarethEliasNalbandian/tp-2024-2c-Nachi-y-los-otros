#include "../src/servicios_memoria.h"

void retardo_respuesta(){
	usleep(RETARDO_RESPUESTA*1000);
}

void logg_acceso_a_espacio_de_usuario(int pid, int tid, char* accion, int dir_fisica, size_t tamanio){
	if(strcmp(accion, "leer") == 0){
		log_info(memoria_log_obligatorio, "## Lectura - (PID:TID) - (%d:%d) - Dir. Física: %d - Tamaño: %zu", pid, tid, dir_fisica, tamanio);
	}else if(strcmp(accion, "escribir") == 0){
		log_info(memoria_log_obligatorio, "## Escritura - (PID:TID) - (%d:%d) - Dir. Física: %d - Tamaño: %zu", pid, tid, dir_fisica, tamanio);
	}else{
		log_error(memoria_log_debug, "logg_acceso_a_espacio_de_usuario, con parametro incorrecto, tiene que ser 0-lectura 1-escritura");
		exit(EXIT_FAILURE);
	}
}

void responder_OK(int file_descriptor, op_code codigo_operacion){
	t_paquete* un_paquete = crear_paquete(codigo_operacion);
	cargar_string_al_paquete(un_paquete, "OK");
	enviar_paquete(un_paquete, file_descriptor);
	eliminar_paquete(un_paquete);
}

void responder_ERROR(int file_descriptor, op_code codigo_operacion){
	t_paquete* un_paquete = crear_paquete(codigo_operacion);
	cargar_string_al_paquete(un_paquete, "ERROR");
	enviar_paquete(un_paquete, file_descriptor);
	eliminar_paquete(un_paquete);
}

void loggear_instruccion_enviada(char** instruccion_split, int pid, int tid){
	int cant_argumentos = string_array_size(instruccion_split)-1;

	switch(cant_argumentos){
		case 0:
			log_info(memoria_log_obligatorio, "## Obtener instrucción - (PID:TID) - (%d:%d) - Instrucción: %s", pid, tid, instruccion_split[0]);
			break;
		case 1:
			log_info(memoria_log_obligatorio, "## Obtener instrucción - (PID:TID) - (%d:%d) - Instrucción: %s - %s", pid, tid, instruccion_split[0], instruccion_split[1]);
			break;
		case 2:
			log_info(memoria_log_obligatorio, "## Obtener instrucción - (PID:TID) - (%d:%d) - Instrucción: %s - %s - %s", pid, tid, instruccion_split[0], instruccion_split[1], instruccion_split[2]);
			break;
		case 3:
			log_info(memoria_log_obligatorio, "## Obtener instrucción - (PID:TID) - (%d:%d) - Instrucción: %s - %s - %s - %s", pid, tid, instruccion_split[0], instruccion_split[1], instruccion_split[2], instruccion_split[3]);
			break;
		default:
			log_error(memoria_log_debug,"CANTIDAD DE ARGUMENTOS DE INSTRUCCION INVALIDA");
			break;
	}
	
}