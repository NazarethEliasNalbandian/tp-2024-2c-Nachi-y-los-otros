#include "../src/cola_multinivel.h"
#include "../src/tcb.h"

void crear_colamultinivel(int prioridad_cola){


    if(existe_cola_con_esa_prioridad(prioridad_cola)){
        log_warning(kernel_log_debug,"YA EXISTE UNA COLA CON LA SIGUIENTE PRIORIDAD: %d", prioridad_cola);
        return;
        
    }else{

        t_cola_multinivel* nueva_cola = malloc(sizeof(t_cola_multinivel));
        nueva_cola->prioridad_lista = prioridad_cola;

        pthread_mutex_init(&(nueva_cola->mutex_prioridad), NULL);
        
        t_lista_ready_y_mutex* nueva_lista_ready_y_mutex = malloc(sizeof(t_lista_ready_y_mutex));
        nueva_cola->lista_ready_y_mutex = nueva_lista_ready_y_mutex;

        nueva_cola->lista_ready_y_mutex->lista_ready = list_create();
        pthread_mutex_init(&(nueva_cola->lista_ready_y_mutex->mutex_lista_ready),NULL); 

        pthread_mutex_lock(&(mutex_lista_cola_multinivel));
        list_add(lista_cola_multinivel, nueva_cola);
        pthread_mutex_unlock(&(mutex_lista_cola_multinivel));

        pthread_mutex_lock(&(nueva_cola->mutex_prioridad));
        log_trace(kernel_log_debug, "SE AGREGO LA COLA MULTINIVEL DE PRIORIDAD %d", prioridad_cola);
        pthread_mutex_unlock(&(nueva_cola->mutex_prioridad));
    }

}

bool existe_cola_con_esa_prioridad(int prioridad){
    t_cola_multinivel* cola_multinivel_a_buscar = obtener_cola_por_prioridad(prioridad); 
    return cola_multinivel_a_buscar != NULL;

}

t_cola_multinivel* obtener_cola_por_prioridad(int prioridad) {
    t_cola_multinivel* cola_multinivel_a_buscar;

    bool es_prioridad_buscada(t_cola_multinivel* cola) {
        pthread_mutex_lock(&(cola->mutex_prioridad));
        bool esIgual = cola->prioridad_lista == prioridad;
        pthread_mutex_unlock(&(cola->mutex_prioridad));
        return esIgual;
    };

    cola_multinivel_a_buscar = list_find(lista_cola_multinivel, (void*) es_prioridad_buscada);

    return cola_multinivel_a_buscar;
}

t_tcb* obtener_siguiente_tcb_a_ejecutar_en_CMN(){
    bool tiene_hilos_ready(void* elem) {
        t_cola_multinivel* cola = (t_cola_multinivel*) elem;
        pthread_mutex_lock(&(cola->lista_ready_y_mutex->mutex_lista_ready));
        bool tiene_elementos = !list_is_empty(cola->lista_ready_y_mutex->lista_ready);
        pthread_mutex_unlock(&(cola->lista_ready_y_mutex->mutex_lista_ready));
        return tiene_elementos;
    };
    
    t_list* colas_con_elementos = list_filter(lista_cola_multinivel, tiene_hilos_ready);

    if (list_is_empty(colas_con_elementos)) {
        list_destroy(colas_con_elementos);  
        return NULL;
    }

    t_cola_multinivel* __maxima_prioridad(t_cola_multinivel* void_1, t_cola_multinivel* void_2){
        pthread_mutex_lock(&(void_1->mutex_prioridad));
        pthread_mutex_lock(&(void_2->mutex_prioridad));
		if(void_1->prioridad_lista <= void_2->prioridad_lista) 
            {
                pthread_mutex_unlock(&(void_1->mutex_prioridad));
                pthread_mutex_unlock(&(void_2->mutex_prioridad));
                return void_1;
            }
		else 
            {
                pthread_mutex_unlock(&(void_1->mutex_prioridad));
                pthread_mutex_unlock(&(void_2->mutex_prioridad));
                return void_2;
            }
    };



    t_cola_multinivel* cola_con_mayor_prioridad = list_get_maximum(colas_con_elementos, (void*) __maxima_prioridad);

    pthread_mutex_lock(&(cola_con_mayor_prioridad->lista_ready_y_mutex->mutex_lista_ready));
    t_tcb* siguiente_tcb = (t_tcb*) list_remove(cola_con_mayor_prioridad->lista_ready_y_mutex->lista_ready, 0);
    pthread_mutex_unlock(&(cola_con_mayor_prioridad->lista_ready_y_mutex->mutex_lista_ready));

    list_destroy(colas_con_elementos);

    return siguiente_tcb;
}

bool todas_las_colas_estan_vacias() {
    for (int i = 0; i < list_size(lista_cola_multinivel); i++) {
        t_cola_multinivel* cola = list_get(lista_cola_multinivel, i);
        pthread_mutex_lock(&(cola->lista_ready_y_mutex->mutex_lista_ready));
        bool esta_vacia = list_is_empty(cola->lista_ready_y_mutex->lista_ready);
        pthread_mutex_unlock(&(cola->lista_ready_y_mutex->mutex_lista_ready));

        if (!esta_vacia) {
            return false;
        }
    }

    
    return true;
}

t_tcb* buscar_y_remover_tcb_de_cola_multinivel(int un_tid, int un_pid) {
    t_tcb* tcb_encontrado = NULL;

    for (int i = 0; i < list_size(lista_cola_multinivel); i++) {
        t_cola_multinivel* cola = list_get(lista_cola_multinivel, i);

        tcb_encontrado = buscar_y_remover_tcb_por_tid_de(un_tid, un_pid, cola->lista_ready_y_mutex->lista_ready, cola->lista_ready_y_mutex->mutex_lista_ready, READY);

        if (tcb_encontrado != NULL) {
            pthread_mutex_lock(&(cola->mutex_prioridad));
            log_info(kernel_log_listas_tcb,"PID %d TID %d REMOVIDO DE COLA MULTINIVEL DE PRIORIDAD %d", un_pid, un_tid, cola->prioridad_lista);
            pthread_mutex_unlock(&(cola->mutex_prioridad));
            
            return tcb_encontrado;
        }
    }

    return NULL;
}

t_tcb* buscar_tcb_en_cola_multinivel(int un_tid, int un_pid) {
    t_tcb* tcb_encontrado = NULL;

    log_warning(kernel_log_debug, "TAMAÑO LISTA DE COLAS MULTINIVEL: %d", list_size(lista_cola_multinivel));
    int i;

    for (i = 0; i < list_size(lista_cola_multinivel); i++) {
        t_cola_multinivel* cola = list_get(lista_cola_multinivel, i);

        log_warning(kernel_log_debug, "TAMAÑO DE COLA MULTINIVEL DE PRIORIDAD %d: %d",cola->prioridad_lista, list_size(cola->lista_ready_y_mutex->lista_ready));

        tcb_encontrado = buscar_tcb_por_tid_de(un_tid, un_pid, cola->lista_ready_y_mutex->lista_ready, cola->lista_ready_y_mutex->mutex_lista_ready);

        if (tcb_encontrado != NULL) {
            log_trace(kernel_log_debug, "PID %d TID %d ENCONTRADO", un_pid, un_tid);
            return tcb_encontrado;
        }

        log_trace(kernel_log_debug, "VUELTA %d COLA_PRIORIDAD %d", i, cola->prioridad_lista);
    }

    log_trace(kernel_log_debug, "PID %d TID %d NO ENCONTRADO EN NINGUNA COLA MULTINIVEL", un_pid, un_tid);

    return NULL;
}

void destruir_cola_multinivel(t_cola_multinivel* cola_multinivel){
    pthread_mutex_destroy(&(cola_multinivel->lista_ready_y_mutex->mutex_lista_ready));
    list_destroy_and_destroy_elements(cola_multinivel->lista_ready_y_mutex->lista_ready, (void*)destruir_tcb);
    safe_free(cola_multinivel->lista_ready_y_mutex);
    safe_free(cola_multinivel);
}