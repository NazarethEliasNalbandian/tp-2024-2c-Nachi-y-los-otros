#include "../src/espacio_usuario.h"

void escribir_uint32_en_dir_fisica(int pid, int tid, int dir_fisica, uint32_t* valor) {
    
    pthread_mutex_lock(&mutex_espacio_usuario);
    memcpy(espacio_usuario + dir_fisica, valor, sizeof(uint32_t));
    pthread_mutex_unlock(&mutex_espacio_usuario);
    logg_acceso_a_espacio_de_usuario(pid, tid, "escribir", dir_fisica, sizeof(uint32_t));

    log_warning(memoria_log_debug, "Se escribio lo siguiente %c", (char) *valor);
    log_info(memoria_log_espacio_usuario, "(PID:TID) (%d, %d) SE ESCRIBIO %u EN LA DIRECCION FISICA %d", pid, tid, *valor, dir_fisica);
}

uint32_t leer_uint32_de_dir_fisica(int pid, int tid, int dir_fisica){
    uint32_t* valor_leido = malloc(sizeof(uint32_t));
    uint32_t dato_retorno;

    if (valor_leido == NULL) {
        log_error(memoria_log_debug,"Error al asignar memoria");
    }

    pthread_mutex_lock(&mutex_espacio_usuario);
    memcpy(valor_leido, espacio_usuario + dir_fisica, sizeof(uint32_t));
    pthread_mutex_unlock(&mutex_espacio_usuario);

	dato_retorno = *valor_leido;

    safe_free(valor_leido);
    
    log_trace(memoria_log_debug, "VALOR LEIDO: %u", dato_retorno);
    log_info(memoria_log_espacio_usuario, "(PID:TID) (%d, %d) VALOR LEIDO: %u", pid, tid, dato_retorno);
    // log_warning(memoria_log_debug, "Se leyo lo siguiente %c", (char) dato_retorno);

    logg_acceso_a_espacio_de_usuario(pid, tid, "leer", dir_fisica, sizeof(uint32_t));
    return  dato_retorno;
}

void* leer_data_de_dir_fisica(int pid, int tid, int dir_fisica, size_t tamanio){
    void* valor_leido = malloc(tamanio);

    if (valor_leido == NULL) {
        log_error(memoria_log_debug,"Error al asignar memoria");
    }

    pthread_mutex_lock(&mutex_espacio_usuario);
    memcpy(valor_leido, espacio_usuario + dir_fisica, tamanio);
    pthread_mutex_unlock(&mutex_espacio_usuario);

    logg_acceso_a_espacio_de_usuario(pid, tid, "leer", dir_fisica, tamanio);
    return  valor_leido;
}