#include "../src/particion.h"

void crear_particion_a_demanda(int tamanio){
    t_particion* nueva_particion = malloc(sizeof(t_particion));

    pthread_mutex_lock(&mutex_particionID);
    nueva_particion->particion_id = particionID++;
    pthread_mutex_unlock(&mutex_particionID);

    nueva_particion->esta_ocupada = true;

    nueva_particion->tamanio = tamanio;

    pthread_mutex_init(&(nueva_particion->mutex_particion), NULL);

    pthread_mutex_lock(&mutex_lista_particiones);
    list_add(lista_particiones, nueva_particion);
    pthread_mutex_unlock(&mutex_lista_particiones);

    log_trace(memoria_log_debug, "PARTICION CREADA");
}

t_particion* buscar_particion_por_id(int particion_id) {
    t_list_iterator* iterador = list_iterator_create(lista_particiones);
    t_particion* particion = NULL;

    while (list_iterator_has_next(iterador)) {
        particion = list_iterator_next(iterador);
        if (particion->particion_id == particion_id) {
            list_iterator_destroy(iterador);
            return particion;
        }
    }

    list_iterator_destroy(iterador);
    return NULL;
}

t_particion* buscar_particion_por_base_y_limite(int base, int limite) {
    t_list_iterator* iterador = list_iterator_create(lista_particiones);
    t_particion* particion = NULL;

    while (list_iterator_has_next(iterador)) {
        particion = list_iterator_next(iterador);
        if (particion->base == base && particion->limite == limite) {
            list_iterator_destroy(iterador);
            return particion;
        }
    }

    list_iterator_destroy(iterador);
    return NULL;
}

t_particion* obtener_primera_particion_libre(int tamanio_solicitado) {
    t_list_iterator* iterador = list_iterator_create(lista_particiones);
    t_particion* particion = NULL;

    while (list_iterator_has_next(iterador)) {
        particion = list_iterator_next(iterador);
        if (!particion->esta_ocupada && particion->tamanio >= tamanio_solicitado) {
            list_iterator_destroy(iterador);
            return particion;
        }
    }

    list_iterator_destroy(iterador);
    return NULL;
}

t_particion* obtener_peor_particion_libre(int tamanio_solicitado) {
    t_list_iterator* iterador = list_iterator_create(lista_particiones);
    t_particion* particion = NULL;
    t_particion* peor_particion = NULL;

    while (list_iterator_has_next(iterador)) {
        particion = list_iterator_next(iterador);
        if (!particion->esta_ocupada && particion->tamanio >= tamanio_solicitado &&
            (peor_particion == NULL || particion->tamanio > peor_particion->tamanio)) {
            peor_particion = particion;
        }
    }

    list_iterator_destroy(iterador);
    return peor_particion;
}

t_particion* obtener_mejor_particion_libre(int tamanio_solicitado) {
    t_list_iterator* iterador = list_iterator_create(lista_particiones);
    t_particion* particion = NULL;
    t_particion* mejor_particion = NULL;

    while (list_iterator_has_next(iterador)) {
        particion = list_iterator_next(iterador);
        if (!particion->esta_ocupada && particion->tamanio >= tamanio_solicitado &&
            (mejor_particion == NULL || particion->tamanio < mejor_particion->tamanio)) {
            mejor_particion = particion;
        }
    }

    list_iterator_destroy(iterador);
    return mejor_particion;
}

t_particion* obtener_particion_a_asignar(int tamanio){
    t_particion* particion_libre;

    switch(ALGORITMO_BUSQUEDA) {
        case FIRST:
            particion_libre = obtener_primera_particion_libre(tamanio);
            return particion_libre;
        case WORST:
            particion_libre = obtener_peor_particion_libre(tamanio);
            return particion_libre;
        case BEST:
            particion_libre = obtener_mejor_particion_libre(tamanio);
            return particion_libre;
        default:
            return NULL;
    }
}

void ordenar_particiones_por_base() {
    bool comparar_particiones_por_base(t_particion* p1, t_particion* p2) {
        return p1->base < p2->base;
    };

    list_sort(lista_particiones, (void*) comparar_particiones_por_base);
}

void mostrarParticiones(){
    ordenar_particiones_por_base();

    for (int i = 0; i < list_size(lista_particiones); i++){
        t_particion* particion = list_get(lista_particiones, i);
        
        pthread_mutex_lock(&(particion->mutex_particion));
        if(particion->esta_ocupada)
            log_info(memoria_log_particion, "PARTICION CON TAMAÑO %d QUE VA DE %d A %d Y ESTA OCUPADA POR %d", particion->tamanio, particion->base, particion->limite, particion->proceso_en_particion->pid);
        else
            log_info(memoria_log_particion, "HUECO CON TAMAÑO %d QUE VA DE %d A %d", particion->tamanio, particion->base, particion->limite);
        
        pthread_mutex_unlock(&(particion->mutex_particion));
    }
}

void unificar_particiones_libres_contiguas() {

    log_info(memoria_log_particion, "ANTES DE UNIFICAR");
    mostrarParticiones();

    bool hubo_cambios;
    do {
        hubo_cambios = false;
        for (int i = 0; i < list_size(lista_particiones) - 1; i++) {
            t_particion* actual = list_get(lista_particiones, i);
            t_particion* siguiente = list_get(lista_particiones, i + 1);

            if (!actual->esta_ocupada && !siguiente->esta_ocupada && actual->limite + 1 == siguiente->base) {
                actual->tamanio += siguiente->tamanio;
                actual->limite = siguiente->limite;

                list_remove_and_destroy_element(lista_particiones, i + 1, safe_free);
                hubo_cambios = true;
                break;
            }
        }
    } while (hubo_cambios);

    log_info(memoria_log_particion, "DESPUÉS DE UNIFICAR");
    mostrarParticiones();
}

void destruir_particion(t_particion* una_particion){
    pthread_mutex_destroy(&(una_particion->mutex_particion));

    safe_free(una_particion);
}