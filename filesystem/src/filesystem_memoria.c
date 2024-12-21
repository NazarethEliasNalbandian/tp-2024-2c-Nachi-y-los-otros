#include "../src/filesystem_memoria.h"
#include "../src/fcb.h"
#include "../src/bloques.h"
#include "../src/bitmap.h"
#include "../src/servicios_filesystem.h"

void atender_filesystem_memoria(void* arg) {

    t_args* args = (t_args*) arg;
	int socket = args->socket;

    safe_free(args);

    int cod_op = recibir_operacion(socket);
    t_buffer* unBuffer = NULL;
    switch (cod_op) {
        case HANDSHAKE_MEMORIA_FILESYSTEM:
            unBuffer = recibir_paquete(socket);
            atender_handshake_memoria_filesystem(unBuffer, socket);
            break;
        case MEMORY_DUMP_MEMORIA_FILESYSTEM:
            unBuffer = recibir_paquete(socket);
            atender_memory_dump(unBuffer, socket);
            break;
        case -1:
            printf("Desconexión de Memoria\n");
            break;
        case FINALIZACION_MEMORIA_FILESYSTEM:
            unBuffer = recibir_paquete(socket);
            int finalizacion = recibir_int_del_buffer(unBuffer);
            destruir_buffer(unBuffer);
            if(finalizacion)
                sem_post(&se_desconecto_memoria);
            break;
        default:
            log_warning(fs_log_debug, "Operación desconocida de la memoria");
            break;
    }

    liberar_conexion(socket);
    
}

void atender_handshake_memoria_filesystem(t_buffer* unBuffer, int socket){
	char* mensaje = recibir_string_del_buffer(unBuffer);
	destruir_buffer(unBuffer);
	log_info(fs_log_debug,"RECIBI LO SIGUIENTE: %s", mensaje);
    safe_free(mensaje);
}

void atender_memory_dump(t_buffer* unBuffer, int socket){
    char* nombre_archivo = recibir_string_del_buffer(unBuffer);
    size_t tamanio_mensaje = recibir_size_t_del_buffer(unBuffer);
    size_t tamanio_particion = recibir_size_t_del_buffer(unBuffer);
    void* mensaje_memoria = recibir_generico_del_buffer(unBuffer);

    destruir_buffer(unBuffer);

    int tamanio_bloques = tamanio_en_bloques((int) tamanio_particion);

    if((tamanio_bloques + 1) <= cantidad_bloques_libres(bitmap)){

        pthread_mutex_lock(&mutex_lista_fcb);
        if(!existe_fcb(nombre_archivo))
            crear_fcb(nombre_archivo, tamanio_particion);
        pthread_mutex_unlock(&mutex_lista_fcb);

        pthread_mutex_lock(&mutex_lista_fcb);
        t_fcb* un_fcb = obtener_fcb(nombre_archivo);
        pthread_mutex_unlock(&mutex_lista_fcb);

        asignar_bloques_de_datos(un_fcb);
        asignar_bloque_de_indice(un_fcb);

        pthread_mutex_lock(&(un_fcb->mutex_fcb));
        crear_archivo_metadata(un_fcb);
        
        pthread_mutex_unlock(&(un_fcb->mutex_fcb));

        escribir_bloque_de_indice(un_fcb);

        escribir_bloques_de_datos(un_fcb, (void*) mensaje_memoria, tamanio_mensaje);

        log_info(fs_log_obligatorio, "## Archivo Creado: %s - Tamaño: %zu", nombre_archivo, tamanio_particion); 

        responder_OK(socket);
    }else {
        log_error(fs_log_debug, "ESPACIO INSUFICIENTE PARA %s", nombre_archivo);
        responder_ERROR(socket);
    }

    log_info(fs_log_obligatorio, "## Fin de solicitud - Archivo: %s", nombre_archivo);

    safe_free(mensaje_memoria);
    safe_free(nombre_archivo);
}