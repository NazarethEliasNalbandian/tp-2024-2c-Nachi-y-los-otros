#include "../src/memoria_filesystem.h"
#include "../src/espacio_usuario.h"

void atender_memoria_filesystem(int fd_filesystem){

	t_buffer* unBuffer = NULL;
	log_trace(memoria_log_debug, "ESPERO FS");
	int cod_op = recibir_operacion(fd_filesystem);
	log_trace(memoria_log_debug, "LLEGO CODIGO: %d", cod_op);
	switch (cod_op) {
		case MEMORY_DUMP_MEMORIA_FILESYSTEM:
			unBuffer = recibir_paquete(fd_filesystem);

			pthread_mutex_lock(&mutex_respuesta_memory_dump_fs);
			safe_free(respuesta_memory_dump_fs);
			respuesta_memory_dump_fs = recibir_string_del_buffer(unBuffer);
			pthread_mutex_unlock(&mutex_respuesta_memory_dump_fs);

			destruir_buffer(unBuffer);
			sem_post(&llego_respuesta_memory_dump_fs);
			break;
		case -1:
			printf("Desconexión de Filesystem\n");
			break;
		default:
			log_warning(memoria_log_debug,"OPERACION DESCONOCIDA");
			break;
	}

	liberar_conexion(fd_filesystem);
}

void enviar_memory_dump_filesystem(int pid, int tid, void* mensaje, size_t tamanio_particion, size_t tamanio_mensaje){

	int fd_filesystem = crear_conexion(IP_FILESYSTEM, PUERTO_FILESYSTEM);

	t_paquete* un_paquete = crear_paquete(MEMORY_DUMP_MEMORIA_FILESYSTEM);
	char* tiempo = temporal_get_string_time("%H:%M:%S:%MS");
	char* nombre_archivo = string_from_format("%d-%d-%s.dmp", pid, tid, tiempo);
	cargar_string_al_paquete(un_paquete, nombre_archivo);
	cargar_size_t_al_paquete(un_paquete, tamanio_mensaje);
	cargar_size_t_al_paquete(un_paquete, tamanio_particion);
	cargar_generico_al_paquete(un_paquete, mensaje, tamanio_mensaje);
	log_trace(memoria_log_debug, "TAM: %lu", tamanio_mensaje);
	enviar_paquete(un_paquete, fd_filesystem);
	eliminar_paquete(un_paquete);

	safe_free(tiempo);
	safe_free(nombre_archivo);
	safe_free(mensaje);

	atender_memoria_filesystem(fd_filesystem);
}
