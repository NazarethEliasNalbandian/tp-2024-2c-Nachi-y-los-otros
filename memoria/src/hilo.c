#include "../src/hilo.h"

void crear_hilo(int pid, int tid, char* archivo_instrucciones){
    t_hilo* nuevo_hilo = malloc(sizeof(t_hilo));

    nuevo_hilo->pid_asociado = pid;
    nuevo_hilo->tid = tid;

    nuevo_hilo->registros = malloc(sizeof(t_registrosCPU));
    nuevo_hilo->registros->AX = 0;
    nuevo_hilo->registros->BX = 0;
    nuevo_hilo->registros->CX = 0;
    nuevo_hilo->registros->DX = 0;
    nuevo_hilo->registros->EX = 0;
    nuevo_hilo->registros->FX = 0;
    nuevo_hilo->registros->GX = 0;
    nuevo_hilo->registros->HX = 0;
    nuevo_hilo->registros->PC = 0;

    size_t path_len = strlen(PATH_INSTRUCCIONES) + strlen(archivo_instrucciones) + 1;
    char* path_archivo = malloc(path_len);

    if (path_archivo == NULL) {
        log_error(memoria_log_debug, "No se pudo asignar memoria para path_archivo");
        return;
    }

    strcpy(path_archivo, PATH_INSTRUCCIONES);
    strcat(path_archivo, archivo_instrucciones);

    nuevo_hilo->archivo_instrucciones = path_archivo;

    nuevo_hilo->lista_instrucciones = leer_archivo_y_cargar_instrucciones(archivo_instrucciones, memoria_log_debug);

    nuevo_hilo->maximo_PC = list_size(nuevo_hilo->lista_instrucciones) - 1;

    pthread_mutex_init(&(nuevo_hilo->mutex_hilo), NULL);

    pthread_mutex_lock(&mutex_lst_hilos_recibidos);
    list_add(list_hilos_recibidos, nuevo_hilo);
    pthread_mutex_unlock(&mutex_lst_hilos_recibidos);
}

t_hilo* buscar_hilo(int pid, int tid){
    bool __es_hilo_buscado(void* elem) {
        t_hilo* hilo = (t_hilo*) elem;
        return (hilo->pid_asociado == pid && hilo->tid == tid);
    };

    pthread_mutex_lock(&mutex_lst_hilos_recibidos);
    t_hilo* hilo_encontrado = list_find(list_hilos_recibidos, __es_hilo_buscado);
    pthread_mutex_unlock(&mutex_lst_hilos_recibidos);

    return hilo_encontrado;
}

void actualizar_contexto_hilo(t_hilo* un_hilo, t_contexto* un_contexto){
    un_hilo->registros->AX = un_contexto->r_cpu->AX;
    un_hilo->registros->BX = un_contexto->r_cpu->BX;
    un_hilo->registros->CX = un_contexto->r_cpu->CX;
    un_hilo->registros->DX = un_contexto->r_cpu->DX;
    un_hilo->registros->EX = un_contexto->r_cpu->EX;
    un_hilo->registros->FX = un_contexto->r_cpu->FX;
    un_hilo->registros->GX = un_contexto->r_cpu->GX;
    un_hilo->registros->HX = un_contexto->r_cpu->HX;
    un_hilo->registros->PC = un_contexto->r_cpu->PC;
}

char* obtener_instruccion_por_indice(t_hilo* un_hilo, uint32_t indice_instruccion){
	char* instruccion_actual;
    pthread_mutex_lock(&(un_hilo->mutex_hilo));
	if(indice_instruccion >= 0 && indice_instruccion < list_size(un_hilo->lista_instrucciones)){
		instruccion_actual = list_get(un_hilo->lista_instrucciones, (int) indice_instruccion);
        pthread_mutex_unlock(&(un_hilo->mutex_hilo));
		return instruccion_actual;
	}
	else{
		log_error(memoria_log_debug, "PID %d TID %d - Nro de Instruccion %u NO VALIDA", un_hilo->pid_asociado, un_hilo->tid, indice_instruccion);
        pthread_mutex_unlock(&(un_hilo->mutex_hilo));
		return NULL;
	}
}

void destruir_hilo(t_hilo* un_hilo){
    safe_free(un_hilo->registros);
    safe_free(un_hilo->archivo_instrucciones);

    list_destroy_and_destroy_elements(un_hilo->lista_instrucciones, (void*)safe_free);

    pthread_mutex_destroy(&(un_hilo->mutex_hilo));

    safe_free(un_hilo);
}