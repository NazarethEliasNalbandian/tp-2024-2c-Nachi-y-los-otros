#include "../src/bloques.h"
#include "../src/servicios_filesystem.h"
#include "../src/bitmap.h"
#include "../src/fcb.h"

int tamanio_en_bloques(int tamanio_en_bytes){
    if(tamanio_en_bytes == 0)
        return 1;
    else
	    return ceil(((double) tamanio_en_bytes)/((double)BLOCK_SIZE));
}

void escribir_data_en_bloque(int indice, void* valor, size_t tamanio) {

    pthread_mutex_lock(&mutex_bloquesEnMemoria);
    memcpy(bloquesEnMemoria + indice, valor, tamanio);
    pthread_mutex_unlock(&mutex_bloquesEnMemoria);

    retardo_acceso_bloque();
}

void* leer_data_de_bloque(int indice, size_t tamanio){
    void* valor_leido = malloc(tamanio);

    pthread_mutex_lock(&mutex_bloquesEnMemoria);
    memcpy(valor_leido, bloquesEnMemoria + (indice * BLOCK_SIZE), tamanio);
    pthread_mutex_unlock(&mutex_bloquesEnMemoria);

    retardo_acceso_bloque();

    return valor_leido;
}

void asignar_bloques_de_datos(t_fcb* un_fcb){
    int indice;
    int i;
    int tamanio_bloques;

    pthread_mutex_lock(&(un_fcb->mutex_fcb));
    tamanio_bloques = un_fcb->tamanio_en_bloques;
    pthread_mutex_unlock(&(un_fcb->mutex_fcb));

    for(i = 0; i < tamanio_bloques; i++){
        int* puntero_indice = malloc(sizeof(int));
        indice = encontrar_primer_bit_libre(bitmap);
        *puntero_indice = indice;
        setear_bloque_bits(bitmap, indice);

        pthread_mutex_lock(&(un_fcb->mutex_fcb));
        list_add(un_fcb->indice_bloques_de_datos, puntero_indice);
        pthread_mutex_unlock(&(un_fcb->mutex_fcb));

        pthread_mutex_lock(&(un_fcb->mutex_fcb));
        log_info(fs_log_obligatorio, "## Bloque asignado: %d - Archivo: %s - Bloques Libres: %d", indice, un_fcb->nombre_archivo, cantidad_bloques_libres(bitmap));
        pthread_mutex_unlock(&(un_fcb->mutex_fcb));

        imprimir_bitarray(bitmap);
    }
}

void asignar_bloque_de_indice(t_fcb* un_fcb){

    pthread_mutex_lock(&(un_fcb->mutex_fcb));
    un_fcb->indice_bloque_indice = encontrar_primer_bit_libre(bitmap);
    setear_bloque_bits(bitmap, un_fcb->indice_bloque_indice);
    log_info(fs_log_obligatorio, "## Bloque asignado: %d - Archivo: %s - Bloques Libres: %d", un_fcb->indice_bloque_indice, un_fcb->nombre_archivo, cantidad_bloques_libres(bitmap));
    pthread_mutex_unlock(&(un_fcb->mutex_fcb));

    imprimir_bitarray(bitmap);
}

void escribir_bloque_de_indice(t_fcb* un_fcb){
    int i;

    pthread_mutex_lock(&(un_fcb->mutex_fcb));

    log_trace(fs_log_debug, "TAM EN BLOQUES: %d", un_fcb->tamanio_en_bloques);

    size_t tamanio_bloques_datos = sizeof(uint32_t) * list_size(un_fcb->indice_bloques_de_datos);

    pthread_mutex_unlock(&(un_fcb->mutex_fcb));

    void* buffer = malloc(tamanio_bloques_datos);

    int tamanio_bloques;

    pthread_mutex_lock(&(un_fcb->mutex_fcb));
    tamanio_bloques = un_fcb->tamanio_en_bloques;
    pthread_mutex_unlock(&(un_fcb->mutex_fcb));

    for(i = 0; i < tamanio_bloques; i++){
        // OBTENGO DATA DEL NUMERO DE INDICE DE EL BLOQUE DE DATOS I (UINT32_T)
        pthread_mutex_lock(&(un_fcb->mutex_fcb));
        uint32_t* puntero_a_indice = list_get(un_fcb->indice_bloques_de_datos, i);
        pthread_mutex_unlock(&(un_fcb->mutex_fcb));

        memcpy(buffer + i * sizeof(uint32_t), puntero_a_indice, sizeof(uint32_t));

        log_trace(fs_log_debug, "DATO A ESCRIBIR: %u", *puntero_a_indice);

        pthread_mutex_lock(&(un_fcb->mutex_fcb));
        log_trace(fs_log_debug, "BLOQUE INDICE DONDE SE ESCRIBIRA %u", un_fcb->indice_bloque_indice);   
        pthread_mutex_unlock(&(un_fcb->mutex_fcb));     
    }

    // ESCRIBO DATA EN LA DIRECCION DEL BLOQUE DE INDICES (ACTUALIZANDO EL INDICE A MEDIDA QUE ESCRIBO)
    pthread_mutex_lock(&(un_fcb->mutex_fcb));
    int indice = un_fcb->indice_bloque_indice;
    pthread_mutex_unlock(&(un_fcb->mutex_fcb));

    escribir_data_en_bloque(indice * BLOCK_SIZE, buffer, tamanio_bloques_datos);

    safe_free(buffer);
}

void escribir_bloques_de_datos(t_fcb* un_fcb, void* mensaje_memoria, size_t tamanio_mensaje){

    // DATA DEL BLOQUE DE INDICES
    pthread_mutex_lock(&(un_fcb->mutex_fcb));
    int indice = un_fcb->indice_bloque_indice;
    pthread_mutex_unlock(&(un_fcb->mutex_fcb));

    pthread_mutex_lock(&(un_fcb->mutex_fcb));
    int tamaño = list_size(un_fcb->indice_bloques_de_datos);
    pthread_mutex_unlock(&(un_fcb->mutex_fcb));

    void* indices_de_bloque_de_indices = leer_data_de_bloque(indice, sizeof(uint32_t) * tamaño);

    pthread_mutex_lock(&(un_fcb->mutex_fcb));
    log_info(fs_log_obligatorio, "## Acceso Bloque - Archivo: %s - Tipo Bloque: INDICE - Bloque File System %d", un_fcb->nombre_archivo, un_fcb->indice_bloque_indice);
    pthread_mutex_unlock(&(un_fcb->mutex_fcb));


    int i;

    pthread_mutex_lock(&(un_fcb->mutex_fcb));
    log_trace(fs_log_debug, "TAM: %ld", un_fcb->tamanio);
    log_trace(fs_log_debug, "CANT TOTAL BLOQUES DATOS: %d", un_fcb->tamanio_en_bloques);
    pthread_mutex_unlock(&(un_fcb->mutex_fcb));

    float cant_datos = (((float) tamanio_mensaje) /((float) BLOCK_SIZE));
    double parte_entera;

    log_trace(fs_log_debug, "CANT DATOS: %f", cant_datos);

    double sobrante = modf(cant_datos, &parte_entera);

    int tamanio_ult_bloque = sobrante * BLOCK_SIZE;

    int cant_bloques_de_datos = parte_entera + 1;
    int cant_bloques_de_datos_llenos = (int) parte_entera;

    log_trace(fs_log_debug, "CANT BLOQUES DE DATOS A ESCRIBIR: %d", cant_bloques_de_datos);

    log_trace(fs_log_debug, "TAM ULT BLOQUE CON DATOS: %d", tamanio_ult_bloque);

    pthread_mutex_lock(&(un_fcb->mutex_fcb));
    int tamanio_bloques = un_fcb->tamanio_en_bloques;
    pthread_mutex_unlock(&(un_fcb->mutex_fcb));

    for(i = 0; i < tamanio_bloques; i++){
        // OBTENGO INDICE DEL SIGUIENTE BLOQUE DE DATOS DONDE VOY A ESCRIBIR
        uint32_t* indice_bloque_datos = malloc(sizeof(uint32_t));
        memcpy(indice_bloque_datos, indices_de_bloque_de_indices + (i * sizeof(uint32_t)),  sizeof(uint32_t));
        int indice = *indice_bloque_datos;
        safe_free(indice_bloque_datos);

        log_trace(fs_log_debug, "INDICE: %d", indice);

        // ESCRIBO PORCIONES EN UINT32_T DEL MENSAJE ENVIADO POR MEMORIA
        if(i <= (cant_bloques_de_datos_llenos-1) || tamanio_ult_bloque == 0){
            escribir_data_en_bloque(indice * BLOCK_SIZE, mensaje_memoria + (i * BLOCK_SIZE), BLOCK_SIZE);
            pthread_mutex_lock(&(un_fcb->mutex_fcb));
            log_info(fs_log_obligatorio, "## Acceso Bloque - Archivo: %s - Tipo Bloque: DATOS - Bloque File System %d", un_fcb->nombre_archivo, indice);
            pthread_mutex_unlock(&(un_fcb->mutex_fcb));
        }
        else if(i == cant_bloques_de_datos_llenos && tamanio_ult_bloque != 0){
            escribir_data_en_bloque(indice * BLOCK_SIZE, mensaje_memoria + (i * BLOCK_SIZE), tamanio_ult_bloque);
            pthread_mutex_lock(&(un_fcb->mutex_fcb));
            log_info(fs_log_obligatorio, "## Acceso Bloque - Archivo: %s - Tipo Bloque: DATOS - Bloque File System %d", un_fcb->nombre_archivo, indice);
            pthread_mutex_unlock(&(un_fcb->mutex_fcb));
        }
    }

    safe_free(indices_de_bloque_de_indices);
}