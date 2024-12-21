#include "../src/proceso.h"
#include "../src/particion.h"

void crear_proceso_esquema_fijo(int pid, t_particion* particion_asignada){
    t_proceso* nuevo_proceso = malloc(sizeof(t_proceso));

    nuevo_proceso->pid = pid;

    nuevo_proceso->base = (uint32_t) particion_asignada->base;
    nuevo_proceso->limite = (uint32_t) (particion_asignada->base + particion_asignada->tamanio - 1);
    nuevo_proceso->particion_asignada_id = particion_asignada->particion_id;
    nuevo_proceso->mayor_direccion_fisica_escrita = -1;
    pthread_mutex_init(&(nuevo_proceso->mutex_proceso), NULL);

    pthread_mutex_lock(&mutex_lst_procss_recibidos);
    list_add(lista_procesos_recibidos, nuevo_proceso);
    pthread_mutex_unlock(&mutex_lst_procss_recibidos);

    particion_asignada->proceso_en_particion = nuevo_proceso;
    particion_asignada->esta_ocupada = true;
}

// RETORNA TRUE SI LOGRA CREAR EL PROCESO
void crear_proceso_esquema_dinamico(int pid, t_particion* particion_asignada, int tamanio) {

    t_proceso* nuevo_proceso = malloc(sizeof(t_proceso));
    nuevo_proceso->pid = pid;
    nuevo_proceso->base = (uint32_t) particion_asignada->base;
    nuevo_proceso->limite = (uint32_t) (particion_asignada->base + tamanio - 1);
    nuevo_proceso->particion_asignada_id = particion_asignada->particion_id;
    nuevo_proceso->mayor_direccion_fisica_escrita = -1;
    pthread_mutex_init(&(nuevo_proceso->mutex_proceso), NULL);

    pthread_mutex_lock(&mutex_lst_procss_recibidos);
    list_add(lista_procesos_recibidos, nuevo_proceso);
    pthread_mutex_unlock(&mutex_lst_procss_recibidos);

    if (particion_asignada->tamanio > tamanio) {
        t_particion* nueva_particion = malloc(sizeof(t_particion));
        nueva_particion->tamanio = particion_asignada->tamanio - tamanio;

        pthread_mutex_lock(&mutex_particionID);
        nueva_particion->particion_id = particionID;
        particionID++;
        pthread_mutex_unlock(&mutex_particionID);

        nueva_particion->esta_ocupada = false;
        nueva_particion->base = nuevo_proceso->limite + 1;
        nueva_particion->limite = particion_asignada->limite;
        nueva_particion->proceso_en_particion = NULL;
        pthread_mutex_init(&(nueva_particion->mutex_particion), NULL);

        list_add(lista_particiones, nueva_particion);
        particion_asignada->limite = nuevo_proceso->limite;
        particion_asignada->tamanio = tamanio;
    }

    particion_asignada->proceso_en_particion = nuevo_proceso;
    particion_asignada->esta_ocupada = true;
}

void finalizar_proceso(t_particion *particion, t_proceso *proceso){
    if (particion != NULL && proceso != NULL) {
        pthread_mutex_lock(&(particion->mutex_particion));
        particion->esta_ocupada = false;
        particion->proceso_en_particion = NULL;

        int process_id = proceso->pid;
        int particion_id = particion->particion_id;
        pthread_mutex_unlock(&(particion->mutex_particion));

        bool remover_proceso(t_proceso* p) {
            return p->pid == proceso->pid;
        };

        list_remove_and_destroy_by_condition(lista_procesos_recibidos, (void*) remover_proceso, safe_free);

        log_trace(memoria_log_debug, "Proceso %d finalizado. Particion %d liberada.", process_id, particion_id);
        log_info(memoria_log_particion, "Proceso %d finalizado. Particion %d liberada.", process_id, particion_id);
    }
}

t_proceso* buscar_proceso(int pid) {
    bool __es_proceso_buscado(void* elem) {
        t_proceso* proceso = (t_proceso*) elem;
        return proceso->pid == pid;
    };

    pthread_mutex_lock(&mutex_lst_procss_recibidos);
    t_proceso* proceso_encontrado = list_find(lista_procesos_recibidos, __es_proceso_buscado);
    pthread_mutex_unlock(&mutex_lst_procss_recibidos);

    return proceso_encontrado;
}

bool existe_proceso(int pid){
    t_proceso* un_proceso = buscar_proceso(pid);
    return un_proceso != NULL;
}

