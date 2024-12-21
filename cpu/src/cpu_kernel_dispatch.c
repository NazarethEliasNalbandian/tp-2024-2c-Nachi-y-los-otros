#include "../src/cpu_kernel_dispatch.h"
#include "../src/cpu_memoria.h"
#include "../src/ciclo_instruccion.h"
#include "../src/servicios_cpu.h"

void atender_cpu_kernel_dispatch(){
    bool control_key = 1;
    while (control_key){
        int cod_op = recibir_operacion(fd_kernel_dispatch);
        // log_info(cpu_log_debug, "Se recibio el siguiente codigo de operacion: %s", convertirCodOpAString(cod_op));
        t_buffer* unBuffer = NULL;
        switch (cod_op){
            case HANDSHAKE_KERNEL_CPU_DISPATCH:
                unBuffer = recibir_paquete(fd_kernel_dispatch);
                atender_handshake_kernel(unBuffer);
                break;
            case EJECUTAR_HILO_KERNEL_CPU:
                unBuffer = recibir_paquete(fd_kernel_dispatch);
                sem_wait(&sem_hilo_en_ejecucion_desalojado);
                ejecutar_en_un_hilo_nuevo_detach((void*)atender_hilo_kernel, (void*)  unBuffer);
                break;
            case ATENDER_RTA_MUTEX_CREATE:
                unBuffer = recibir_paquete(fd_kernel_dispatch);
                safe_free(rta_mutex_create);
                rta_mutex_create = recibir_string_del_buffer(unBuffer);
                destruir_buffer(unBuffer);
                
                if(strcmp(rta_mutex_create, "OK") == 0)
                    sem_post(&sem_rta_mutex_create);
                else
                    log_error(cpu_log_debug, "RESPUESTA NO RECONOCIDA EN MUTEX CREATE");
                
                break;
            case ATENDER_RTA_MUTEX_LOCK:
                unBuffer = recibir_paquete(fd_kernel_dispatch);
                safe_free(rta_mutex_lock);
                rta_mutex_lock = recibir_string_del_buffer(unBuffer);
                destruir_buffer(unBuffer);
                
                if(strcmp(rta_mutex_lock, "OK") == 0){
                    // // ACA
                    // pthread_mutex_lock(&mutex_interruptFlag);
                    // interrupt_flag = 1;
                    // pthread_mutex_unlock(&mutex_interruptFlag);
                    sem_post(&sem_rta_mutex_lock);
                }else if (strcmp(rta_mutex_lock, "DESALOJO") == 0){
                    instancia_ocupada = true;
                    sem_post(&sem_rta_mutex_lock);
                }else
                    log_error(cpu_log_debug, "RESPUESTA NO RECONOCIDA EN MUTEX LOCK");
                
                break;
            case ATENDER_RTA_MUTEX_UNLOCK:
                unBuffer = recibir_paquete(fd_kernel_dispatch);
                safe_free(rta_mutex_unlock);
                rta_mutex_unlock = recibir_string_del_buffer(unBuffer);
                destruir_buffer(unBuffer);
                
                if(strcmp(rta_mutex_unlock, "OK") == 0)
                    sem_post(&sem_rta_mutex_unlock);
                else
                    log_error(cpu_log_debug, "RESPUESTA NO RECONOCIDA EN MUTEX UNLOCK");
                
                break;
            case ATENDER_RTA_THREAD_CREATE:
                unBuffer = recibir_paquete(fd_kernel_dispatch);
                safe_free(rta_thread_create);
                rta_thread_create = recibir_string_del_buffer(unBuffer);
                destruir_buffer(unBuffer);
                
                if(strcmp(rta_thread_create, "OK") == 0)
                    sem_post(&sem_rta_thread_create);
                else
                    log_error(cpu_log_debug, "RESPUESTA NO RECONOCIDA EN THREAD CREATE");
                
                break;
            case ATENDER_RTA_PROCESS_CREATE:
                unBuffer = recibir_paquete(fd_kernel_dispatch);
                safe_free(rta_process_create);
                rta_process_create = recibir_string_del_buffer(unBuffer);
                destruir_buffer(unBuffer);
                
                if(strcmp(rta_process_create, "OK") == 0)
                    sem_post(&sem_rta_process_create);
                else
                    log_error(cpu_log_debug, "RESPUESTA NO RECONOCIDA EN PROCESS CREATE");
                
                break;
            case ATENDER_RTA_THREAD_CANCEL:
                unBuffer = recibir_paquete(fd_kernel_dispatch);
                safe_free(rta_thread_cancel);
                rta_thread_cancel = recibir_string_del_buffer(unBuffer);
                destruir_buffer(unBuffer);
                
                if(strcmp(rta_thread_cancel, "OK") == 0)
                    sem_post(&sem_rta_thread_cancel);
                else
                    log_error(cpu_log_debug, "RESPUESTA NO RECONOCIDA EN THREAD CANCEL");
                
                break;
            case ATENDER_RTA_THREAD_JOIN:
                unBuffer = recibir_paquete(fd_kernel_dispatch);
                safe_free(rta_thread_join);
                rta_thread_join = recibir_string_del_buffer(unBuffer);
                destruir_buffer(unBuffer);
                
                if(strcmp(rta_thread_join, "OK") == 0){
                    // // ACA
                    // pthread_mutex_lock(&mutex_interruptFlag);
                    // interrupt_flag = 1;
                    // pthread_mutex_unlock(&mutex_interruptFlag);
                    sem_post(&sem_rta_thread_join);
                }
                else if (strcmp(rta_thread_join, "DESALOJO") == 0){
                    pthread_mutex_lock(&mutex_existe_hilo_a_joinear);
                    existe_hilo_a_joinear = true;
                    pthread_mutex_unlock(&mutex_existe_hilo_a_joinear);
                    sem_post(&sem_rta_thread_join);
                }else
                    log_error(cpu_log_debug, "RESPUESTA NO RECONOCIDA EN THREAD JOIN");
                
                break;
            case -1:
                printf("Desconexión de Kernel Dispatch\n");
                sem_post(&se_desconecto_kernel);
                control_key = 0;
                break;
            default:
                log_warning(cpu_log_debug, "Operacion desconocida de Kernel Dispatch.");
                break;
        }
    }
}

void atender_handshake_kernel(t_buffer* unBuffer){
    char* mensaje = recibir_string_del_buffer(unBuffer);
    destruir_buffer(unBuffer);
    log_info(cpu_log_debug, "RECIBI LO SIGUIENTE: %s", mensaje);
    safe_free(mensaje);
}

uint32_t *detectar_registro(t_registros RX)
{
    if(un_contexto != NULL){
        switch (RX)
        {
            case PC:
                return &(un_contexto->r_cpu->PC);
            case AX:
                return &(un_contexto->r_cpu->AX);
            case BX:
                return &(un_contexto->r_cpu->BX);
            case CX:
                return &(un_contexto->r_cpu->CX);
            case DX:
                return &(un_contexto->r_cpu->DX);
            case EX:
                return &(un_contexto->r_cpu->EX);
            case FX:
                return &(un_contexto->r_cpu->FX);
            case GX:
                return &(un_contexto->r_cpu->GX);
            case HX:
                return &(un_contexto->r_cpu->HX);
        
            default:
                log_error(cpu_log_debug, "REGISTRO %d NO ENCONTRADO", RX);
                return NULL;
                break;
        }
    }else
        return NULL;
}

bool check_interrupt()
{
    pthread_mutex_lock(&mutex_interruptFlag);
    bool interrupt = interrupt_flag;
    pthread_mutex_unlock(&mutex_interruptFlag);
    if(interrupt)
    {
        return 1;
    }
    return 0;
}

void ciclo_de_instruccion()
{
    fetch();
	sem_wait(&sem_fetch);
	decode();
	sem_wait(&sem_decode);
	execute();
}

void atender_hilo_kernel(t_buffer* unBuffer){

    int pid_en_ejecucion = recibir_int_del_buffer(unBuffer);
    int tid_en_ejecucion = recibir_int_del_buffer(unBuffer);
    pthread_mutex_lock(&mutex_VERIFICADOR);
    VERIFICADOR = recibir_int_del_buffer(unBuffer);
    if(un_contexto != NULL)
        un_contexto->verificador = VERIFICADOR;
    pthread_mutex_unlock(&mutex_VERIFICADOR);
    destruir_buffer(unBuffer);  


    solicitar_contexto_a_memoria(pid_en_ejecucion, tid_en_ejecucion);

    sem_wait(&llego_contexto);


    if(!hay_exit){
        log_trace(cpu_log_debug, "LLEGO EL CONTEXTO DEL PID %d TID %d", pid_en_ejecucion, tid_en_ejecucion);
        // log_trace(cpu_log_debug, "PC: %u", un_contexto->r_cpu->PC);

        pthread_mutex_lock(&mutex_manejo_contexto);
        pthread_mutex_lock(&mutex_tipo_desalojo);
        
        while(true){
            ciclo_de_instruccion();
            sem_post(&termino_ciclo_instruccion);
            
            if(hay_syscall(nombre_instruccion_enum))
                actualizar_contexto();

            if(hay_exit){
                break;
            }

            if(instancia_ocupada)
                break;

            pthread_mutex_lock(&mutex_existe_hilo_a_joinear);
            if(existe_hilo_a_joinear){

                pthread_mutex_unlock(&mutex_existe_hilo_a_joinear);
                break;
            }else
                pthread_mutex_unlock(&mutex_existe_hilo_a_joinear);

            if(hay_bloqueo(nombre_instruccion_enum)){
                break;
            }
            
            pthread_mutex_lock(&mutex_interruptFlag);
            bool bool_interrupt = interrupt_flag;
            pthread_mutex_unlock(&mutex_interruptFlag);

            // Controlar si hay interrupciones provenientes de kernel
            if(bool_interrupt){
                break;
            }
        }
        log_info(cpu_log_debug, "Finalice ciclo instruccion PID:%d TID:%d",un_contexto->pid, un_contexto->tid);
        pthread_mutex_lock(&mutex_interruptFlag);
        if(!hay_exit && interrupt_flag){
            pthread_mutex_unlock(&mutex_interruptFlag);
            log_info(cpu_log_debug, "ENTRE A INTERRUPT_FLAG");
            if(!hay_syscall(nombre_instruccion_enum))
                actualizar_contexto();
            if(un_contexto != NULL)
                enviar_hilo_a_kernel(un_contexto->pid, un_contexto->tid, un_contexto->verificador, ATENDER_DESALOJO_HILO_CPU, NADA);
        }
        else
            pthread_mutex_unlock(&mutex_interruptFlag);
    }
    if(hay_exit){
        // ENTRA ACA EN CASO DE THREAD EXIT O PROCESS EXIT
        if(un_contexto != NULL){
            log_info(cpu_log_debug, "ENTRE A DESALOJAR_FLAG");
            enviar_hilo_a_kernel(un_contexto->pid, un_contexto->tid, un_contexto->verificador, ATENDER_INSTRUCCION_CPU, TIPO_EXIT);
        }
	}
    if(un_contexto != NULL)
	    log_info(cpu_log_debug, "Hilo desalojado PID:%d TID:%d",un_contexto->pid, un_contexto->tid);

    hay_exit = false;
    pthread_mutex_lock(&mutex_interruptFlag);
    interrupt_flag = false;
    pthread_mutex_unlock(&mutex_interruptFlag);
    instancia_ocupada = false;
    existe_hilo_a_joinear = false;
    TIPO_EXIT = NADA;

    pthread_mutex_lock(&mutex_instruccion_split);
    if(instruccion_split != NULL){

        string_array_destroy(instruccion_split);
        instruccion_split = NULL;
    }
    pthread_mutex_unlock(&mutex_instruccion_split);

    pthread_mutex_unlock(&mutex_tipo_desalojo);
	pthread_mutex_unlock(&mutex_manejo_contexto);

    sem_post(&sem_hilo_en_ejecucion_desalojado);
}