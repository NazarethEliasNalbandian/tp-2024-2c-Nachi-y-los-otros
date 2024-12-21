#include "../src/fcb.h"
#include "../src/bloques.h"

void crear_fcb(char* nombre_archivo, size_t tamanio) {

    // Creo el FCB y lo agrego a la lista de structs
    t_fcb* nuevo_fcb = malloc(sizeof(t_fcb));
    if (nuevo_fcb == NULL) {
        log_error(fs_log_debug, "No se pudo asignar memoria para nuevo_fcb");
        return;
    }

    nuevo_fcb->nombre_archivo = malloc(strlen(nombre_archivo) + 1);

    if (nuevo_fcb->nombre_archivo == NULL) {
        log_error(fs_log_debug, "No se pudo asignar memoria para nuevo_fcb->nombre_archivo");

        safe_free(nuevo_fcb);
        return;
    }

    strcpy(nuevo_fcb->nombre_archivo, nombre_archivo);

    nuevo_fcb->tamanio = tamanio;
    nuevo_fcb->tamanio_en_bloques = tamanio_en_bloques((int) nuevo_fcb->tamanio);
    nuevo_fcb->indice_bloques_de_datos = list_create();

    pthread_mutex_init(&(nuevo_fcb->mutex_fcb), NULL);

    list_add(lista_fcb, nuevo_fcb);
}

void crear_archivo_metadata(t_fcb* un_fcb){
    size_t path_len = strlen(MOUNT_DIR) + strlen("/files/") + strlen(un_fcb->nombre_archivo) + 1;
    char* path_archivo = malloc(path_len);
    if (path_archivo == NULL) {
        log_error(fs_log_debug, "No se pudo asignar memoria para path_archivo");
        return;
    }

    strcpy(path_archivo, MOUNT_DIR);
    strcat(path_archivo, "/files/");
    strcat(path_archivo, un_fcb->nombre_archivo);

    log_info(fs_log_debug, "PATH: %s", path_archivo);

    char text_tamanio_archivo[10];
    sprintf(text_tamanio_archivo, "%ld", un_fcb->tamanio);

    char text_tamanio_archivo2[10];
    sprintf(text_tamanio_archivo2, "%d", un_fcb->indice_bloque_indice);

    FILE* file_fcb = fopen(path_archivo, "a+");
    if (file_fcb == NULL) {
        log_error(fs_log_debug, "No se pudo crear el archivo de metadata");
        safe_free(path_archivo);
        
        return;
    }
    fclose(file_fcb);

    un_fcb->archivo_metadata = config_create(path_archivo);
    if (un_fcb->archivo_metadata == NULL) {
        log_error(fs_log_debug, "No se pudo crear el archivo de configuración");
        safe_free(path_archivo);

        return;
    }

    config_set_value(un_fcb->archivo_metadata, "SIZE", text_tamanio_archivo);
    config_set_value(un_fcb->archivo_metadata, "INDEX_BLOCK", text_tamanio_archivo2);

    config_save(un_fcb->archivo_metadata);

    safe_free(path_archivo);
}

bool existe_fcb(char* nombre_archivo){
    t_fcb* un_fcb;
    un_fcb = obtener_fcb(nombre_archivo);
    return un_fcb != NULL;
}

t_fcb* obtener_fcb(char* nombre_archivo) {
    t_fcb* fcb_buscado = NULL;

    if (!list_is_empty(lista_fcb)) {
        for (int i = 0; i < list_size(lista_fcb); i++) {
            fcb_buscado = list_get(lista_fcb, i);
            if (fcb_buscado != NULL && strcmp(fcb_buscado->nombre_archivo, nombre_archivo) == 0) {
                pthread_mutex_unlock(&mutex_lista_fcb);
                return fcb_buscado;
            }
        }
    }
    return NULL;
}

// USAR CON MUTEX
void setear_valor_entero_en_fcb(t_fcb* una_fcb, char* clave, int valor) {
    if (una_fcb == NULL || clave == NULL) {
        log_error(fs_log_debug, "Error: FCB o clave nulo en setear_valor_entero_en_fcb");
        return;
    }

    char* text_valor = malloc(10);
    if (text_valor == NULL) {
        log_error(fs_log_debug, "Error: No se pudo asignar memoria para text_valor");
        return;
    }

    sprintf(text_valor, "%d", valor);

    for (int i = 0; i < list_size(lista_fcb); i++) {
        t_fcb* archivo_buscado = list_get(lista_fcb, i);
        if (archivo_buscado != NULL && strcmp(archivo_buscado->nombre_archivo, una_fcb->nombre_archivo) == 0) {
            
            config_set_value(archivo_buscado->archivo_metadata, clave, text_valor);
            config_save(archivo_buscado->archivo_metadata);
            break;
        }
    }

    safe_free(text_valor);
}

void finalizar_fcb(t_fcb* fcb){
    if (fcb == NULL) {
        log_error(fs_log_debug,"ARCHIVO A ELIMINAR ES NULO");
        return;
    }

    if (fcb->archivo_metadata != NULL) {
        char* path_archivo = strdup(fcb->archivo_metadata->path);

        config_destroy(fcb->archivo_metadata);
        safe_free(path_archivo);
    }

    list_destroy_and_destroy_elements(fcb->indice_bloques_de_datos, (void*) safe_free);

    pthread_mutex_destroy(&(fcb->mutex_fcb));

    safe_free(fcb->nombre_archivo);
    safe_free(fcb);
}

void destruir_listas_fcbs(){
	list_destroy_and_destroy_elements(lista_fcb, (void*) finalizar_fcb);
}