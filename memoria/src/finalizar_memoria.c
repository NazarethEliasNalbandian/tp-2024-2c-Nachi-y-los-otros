#include "../src/finalizar_memoria.h"
#include "../src/hilo.h"
#include "../src/particion.h"

void finalizar_memoria(){

    finalizar_estructuras();
    finalizar_configs();
    finalizar_conexiones();
    finalizar_semaforos();
    finalizar_pthreads();
    finalizar_logs();
}

void finalizar_estructuras(){
    safe_free(respuesta_memory_dump_fs);
    safe_free(espacio_usuario);

    list_destroy_and_destroy_elements(lista_procesos_recibidos, (void*) safe_free);
    list_destroy_and_destroy_elements(lista_particiones, (void*) destruir_particion);
    list_destroy_and_destroy_elements(list_hilos_recibidos, (void*) destruir_hilo);
}

void finalizar_logs(){
    log_destroy(memoria_log_debug);
    memoria_log_debug = NULL;
	log_destroy(memoria_log_obligatorio);
    memoria_log_obligatorio = NULL;
    log_destroy(memoria_log_espacio_usuario);
    memoria_log_espacio_usuario = NULL;
    log_destroy(memoria_log_particion);
    memoria_log_particion = NULL;
}

void finalizar_configs(){
    config_destroy(memoria_config);
}
void finalizar_conexiones(){
    liberar_conexion(fd_cpu);
}

void finalizar_semaforos(){
    sem_destroy(&se_desconecto_cpu);
    sem_destroy(&llego_respuesta_memory_dump_fs);
}

void finalizar_pthreads(){
	pthread_mutex_destroy(&mutex_lst_procss_recibidos);
	pthread_mutex_destroy(&mutex_particionID);
	pthread_mutex_destroy(&mutex_lista_particiones);
    pthread_mutex_destroy(&mutex_particionID);
	pthread_mutex_destroy(&mutex_respuesta_memory_dump_fs);
	pthread_mutex_destroy(&mutex_lst_procss_recibidos);
    pthread_mutex_destroy(&mutex_fd_memoria);
}