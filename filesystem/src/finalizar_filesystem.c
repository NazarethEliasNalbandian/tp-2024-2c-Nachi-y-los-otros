#include "../src/finalizar_filesystem.h"
#include "../src/fcb.h"

void finalizar_filesystem()
{
    finalizar_configs();
    finalizar_semaforos();
    finalizar_pthreads();
	destruir_listas_fcbs();

	close(fd_bitmap);
	close(fd_archivoBloques);

	bitarray_destroy(bitmap);

    safe_free(PATH_BITMAP);
    safe_free(PATH_ARCHIVO_BLOQUES);
    finalizar_logs();
}

void finalizar_logs(){
    log_destroy(fs_log_obligatorio);
	fs_log_obligatorio = NULL;
	log_destroy(fs_log_debug);
	fs_log_debug = NULL;
}

void finalizar_configs(){
	config_destroy(fs_config);
}

void finalizar_semaforos(){
	sem_destroy(&se_desconecto_memoria);
}

void finalizar_pthreads(){
    pthread_mutex_destroy(&mutex_bitmap);
	pthread_mutex_destroy(&mutex_lista_fcb);
	pthread_mutex_destroy(&mutex_bloquesEnMemoria);
}


