#include "../src/servicios_cpu.h"
#include "../src/ciclo_instruccion.h"
#include "../src/cpu_kernel_dispatch.h"

void destruir_contexto(){
    safe_free(un_contexto->r_cpu);
    safe_free(un_contexto);
}

void enviar_hilo_a_kernel(int pid, int tid, int verificador, op_code tipo_desalojo, tipo_exit tipo_de_exit){
    t_paquete* un_paquete = crear_paquete(tipo_desalojo);

    cargar_hilo_a_paquete(un_paquete, pid, tid, verificador);
    
    if(tipo_de_exit == THREAD){
        cargar_int_al_paquete(un_paquete, THREAD_EXIT);
        cargar_int_al_paquete(un_paquete,0);
    }

    if(tipo_de_exit == PROCESS){
        cargar_int_al_paquete(un_paquete, PROCESS_EXIT);
        cargar_int_al_paquete(un_paquete,0);
    }

    pthread_mutex_lock(&mutex_hubo_quantum);
    cargar_int_al_paquete(un_paquete, (int) hubo_quantum);
    hubo_quantum = false;
    pthread_mutex_unlock(&mutex_hubo_quantum);

    enviar_paquete(un_paquete, fd_kernel_dispatch);
    eliminar_paquete(un_paquete);
}

void cargar_hilo_a_paquete(t_paquete* un_paquete, int pid, int tid, int verificador){
    cargar_int_al_paquete(un_paquete, pid);
    cargar_int_al_paquete(un_paquete, tid);
    cargar_int_al_paquete(un_paquete, verificador);
}

bool validador_de_header(char* header_string){
	bool respuesta = false;
	int i = 0;
	while(op_autorizada[i] != NULL){
		if(strcmp(op_autorizada[i], header_string) == 0 || esDumpMemory(header_string)) 
            respuesta = true;
        i++;
	}
	return respuesta;
}

void asignarRegistro(t_registros registro, uint32_t** valor_a_guardar)
{
    *valor_a_guardar = detectar_registro(registro);   
}

int obtenerIntDeRegistro(t_registros registro, uint32_t** r){
                
    asignarRegistro(registro, r);
    return ((int)(**r));

}

void escribir_registro(t_registros registro_a_escribir, int valor, uint32_t* registroAEscribir){

    registroAEscribir = detectar_registro(registro_a_escribir);
    *registroAEscribir = (uint32_t) valor;
}

void actualizar_contexto(){
    t_paquete* un_paquete = crear_paquete(ACTUALIZACION_CONTEXTO_CPU_MEMORIA);
    if(un_contexto != NULL)
        cargar_contexto_al_paquete(un_paquete, un_contexto);
    enviar_paquete(un_paquete, fd_memoria);
    eliminar_paquete(un_paquete);
    // log_warning(cpu_log_debug, "ESPERANDO CONTEXTO PARA ACTUALIZAR");
    sem_wait(&contexto_actualizado);
    if(un_contexto != NULL)
        log_info(cpu_log_obligatorio, "## TID: %d - Actualizo Contexto Ejecución", un_contexto->tid);
}

bool hay_bloqueo(nombre_instruccion_comando nombre_inst){
    switch (nombre_inst){
        case IO:
        case DUMP_MEMORY:
            return true;
        default:
            return false;
    }
}

void atender_SEGMENTATION_FAULT(){
    log_error(cpu_log_debug, "SEGMENTATION FAULT");
    actualizar_contexto();
    hay_exit = true;
    TIPO_EXIT = PROCESS;
}

bool hay_syscall(nombre_instruccion_comando nombre_inst){
    switch (nombre_inst){
        case IO:
        case DUMP_MEMORY:
        case MUTEX_CREATE:
        case MUTEX_LOCK:
        case MUTEX_UNLOCK:
        case PROCESS_CREATE:
        case PROCESS_EXIT:
        case THREAD_CREATE:
        case THREAD_JOIN:
        case THREAD_CANCEL:
        case THREAD_EXIT:
            return true;
        default:
            return false;
    }
}

