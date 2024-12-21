#include "../src/kernel_cpu_dispatch.h"
#include "../src/pcb.h"
#include "../src/tcb.h"
#include "../src/servicios_kernel.h"
#include "../src/planificador_largo_plazo.h"
#include "../src/planificador_corto_plazo.h"
#include "../src/mutex.h"
#include "../src/kernel_memoria.h"
#include "../src/kernel_cpu_interrupt.h"

void atender_kernel_cpu_dispatch(){
    bool control_key=1;
    while (control_key){
        // log_error(kernel_log_debug, "ESPERANDO MENSAJE DE CPU");
        int cod_op = recibir_operacion(fd_cpu_dispatch);
        if(kernel_log_debug != NULL)
            log_info(kernel_log_debug, "Se recibio el siguiente codigo de operacion: %s", convertirCodOpAString(cod_op));
        t_tcb* tcb = NULL;
        t_pcb* pcb = NULL;
        t_buffer* unBuffer = NULL;
        t_mochila* mochila = NULL;
        int pid;
        int tid;
        int verificador;

        switch (cod_op) {
            case ATENDER_INSTRUCCION_CPU:
                unBuffer = recibir_paquete(fd_cpu_dispatch);

                pid = recibir_int_del_buffer(unBuffer);
                tid = recibir_int_del_buffer(unBuffer);
                verificador = recibir_int_del_buffer(unBuffer);

                if(mochila != NULL){
                    destruir_mochila(mochila); 
                    mochila = NULL;
                }

                mochila = crear_mochila();
                pthread_mutex_lock(&(mochila->mutex_mochila));
                recibir_mochila(unBuffer, mochila, kernel_log_debug);
                pthread_mutex_unlock(&(mochila->mutex_mochila));
                destruir_buffer(unBuffer);

                tcb = obtener_hilo_desalojado(pid, tid, verificador);
                pcb = buscar_pcb_por_pid(pid);

                pthread_mutex_lock(&(mochila->mutex_mochila));
                pthread_mutex_lock(&(pcb->mutex_pcb));
                if(mochila->instruccionAsociada == PROCESS_EXIT && pcb->flag_proceso_finalizado == true){
                    pthread_mutex_unlock(&(pcb->mutex_pcb));
                    pthread_mutex_unlock(&(mochila->mutex_mochila));
                    break;
                }
                pthread_mutex_unlock(&(pcb->mutex_pcb));

                if(mochila->instruccionAsociada == IO)
                    log_info(kernel_log_listas_io, "## (%d:%d) - Solicitó syscall: %s", pid, tid, convertirEnumAInstruccionString(mochila->instruccionAsociada));

                if(mochila->instruccionAsociada == IO || mochila->instruccionAsociada == DUMP_MEMORY){
                    pthread_mutex_lock(&(tcb->mutex_tcb));
                    tcb->flag_cancelar_quantum = true;
                    pthread_mutex_unlock(&(tcb->mutex_tcb));
                }
                log_info(kernel_log_obligatorio, "## (%d:%d) - Solicitó syscall: %s", pid, tid, convertirEnumAInstruccionString(mochila->instruccionAsociada));

                pthread_mutex_unlock(&(mochila->mutex_mochila));

                if(tcb != NULL){
                    pthread_mutex_lock(&(tcb->mutex_tcb));
                    tcb->flag_cancelar_quantum = true;
                    pthread_mutex_unlock(&(tcb->mutex_tcb));
                }

                if(tcb==NULL){
                    log_error(kernel_log_debug, "TCB_INVALIDO");
                }
                else{
                    char* archivo_instrucciones;
                    int* puntero_tamanio;
                    int tamanio;
                    int* puntero_prioridad_tid0;
                    int prioridad_tid0;
                    int* puntero_prioridad_tid;
                    int prioridad_tid;
                    t_pcb* un_pcb;
                    int* puntero_tid_a_esperar;
                    int tid_a_esperar;
                    t_tcb* tcb_a_esperar;
                    t_hilo_id* id_hilo_en_espera;
                    int* puntero_tid_a_finalizar;
                    int tid_a_finalizar;
                    t_hilo_id* id_hilo_a_eliminar;
                    char* nombre_recurso;
                    int* puntero_tiempo_sleep;
                    int tiempo_sleep;
                    t_mutex_pcb* un_mutex;
                    t_hilo_id* id_hilo_bloqueado;

                    pthread_mutex_lock(&(mochila->mutex_mochila));

                    switch (mochila->instruccionAsociada){
                        case PROCESS_CREATE:
                            archivo_instrucciones = (char*) queue_pop(mochila->parametros);

                            puntero_tamanio = (int*) queue_pop(mochila->parametros);
                            tamanio =  *puntero_tamanio;
                            safe_free(puntero_tamanio);

                            puntero_prioridad_tid0 = (int*) queue_pop(mochila->parametros);
                            pthread_mutex_unlock(&(mochila->mutex_mochila));
                            prioridad_tid0 =  *puntero_prioridad_tid0;
                            safe_free(puntero_prioridad_tid0);

                            crear_proceso(archivo_instrucciones, tamanio, prioridad_tid0);
                            enviar_mensaje_a_CPU(ATENDER_RTA_PROCESS_CREATE, "OK");
                            safe_free(archivo_instrucciones);
                            break;
                        case PROCESS_EXIT:
                            pthread_mutex_unlock(&(mochila->mutex_mochila));
                            if(tid == 0){
                                int* puntero_a_pid = malloc(sizeof(int));
                                *puntero_a_pid = pid;
                                ejecutar_en_un_hilo_nuevo_detach((void*) finalizar_proceso, (void*) puntero_a_pid);
                            }else
                                log_error(kernel_log_debug, "PROCESS CREATE NO FUE LLAMADA POR EL HILO 0");   
                            break;
                        case THREAD_CREATE:
                            archivo_instrucciones = (char*) queue_pop(mochila->parametros);

                            puntero_prioridad_tid = (int*) queue_pop(mochila->parametros);
                            pthread_mutex_unlock(&(mochila->mutex_mochila));
                            prioridad_tid =  *puntero_prioridad_tid;
                            safe_free(puntero_prioridad_tid);

                            un_pcb = buscar_pcb_por_pid_en(pid, lista_pcb_enMemoria);

                            if(un_pcb == NULL){
                                log_error(kernel_log_debug, "EL PCB %d ASOCIADO AL HILO %d NO FUE ENCONTRADO", pid, tid);
                                break;
                            }
                            crear_tcb(un_pcb, archivo_instrucciones, prioridad_tid);
                            enviar_mensaje_a_CPU(ATENDER_RTA_THREAD_CREATE, "OK");
                            safe_free(archivo_instrucciones);
                            break;
                        case THREAD_JOIN:
                            log_trace(kernel_log_debug, "ENTRE A THREAD_JOIN");
                            puntero_tid_a_esperar = (int*) queue_pop(mochila->parametros);
                            pthread_mutex_unlock(&(mochila->mutex_mochila));
                            tid_a_esperar = *puntero_tid_a_esperar;
                            safe_free(puntero_tid_a_esperar);

                            log_trace(kernel_log_debug, "TID A ESPERAR: %d", tid_a_esperar);

                            tcb_a_esperar = buscar_tcb_por_tid(tid_a_esperar, pid);

                            id_hilo_en_espera = malloc(sizeof(t_hilo_id));
                            id_hilo_en_espera->pid_asociado = tcb->id_hilo->pid_asociado;
                            id_hilo_en_espera->tid = tcb->id_hilo->tid;

                            if(existe_hilo(tid_a_esperar, pid)){
                                log_trace(kernel_log_debug, "EXISTE HILO DE PID %d TID %d", pid, tid_a_esperar);
                                enviar_mensaje_a_CPU(ATENDER_RTA_THREAD_JOIN, "DESALOJO");   
                                // sem_wait(&hilo_desalojado);
                                bloquear_hilo_en_ejecucion(tid, pid, PTHREAD_JOIN);
                                pthread_mutex_lock(&(tcb_a_esperar->mutex_tcb));
                                list_add(tcb_a_esperar->tids_en_espera_por_join, id_hilo_en_espera);
                                pthread_mutex_unlock(&(tcb_a_esperar->mutex_tcb));
                            }else{
                                log_trace(kernel_log_debug, "NO EXISTE HILO DE PID %d TID %d", pid, tid_a_esperar);
                                enviar_mensaje_a_CPU(ATENDER_RTA_THREAD_JOIN, "OK"); 
                                safe_free(id_hilo_en_espera);
                            }   
                            
                            break; 
                        case THREAD_CANCEL:
                            puntero_tid_a_finalizar = (int*) queue_pop(mochila->parametros);
                            pthread_mutex_unlock(&(mochila->mutex_mochila));
                            tid_a_finalizar = *puntero_tid_a_finalizar;

                            safe_free(puntero_tid_a_finalizar);

                            id_hilo_a_eliminar = malloc(sizeof(t_hilo_id));
                            id_hilo_a_eliminar->pid_asociado = pid;
                            id_hilo_a_eliminar->tid = tid_a_finalizar;

                            if(existe_hilo(tid_a_finalizar, pid))
                                finalizar_hilo(id_hilo_a_eliminar);

                            enviar_mensaje_a_CPU(ATENDER_RTA_THREAD_CANCEL, "OK");

                            break;
                        
                        case THREAD_EXIT:
                            pthread_mutex_unlock(&(mochila->mutex_mochila));
                            pthread_mutex_lock(&(tcb->mutex_tcb));
                            finalizar_hilo(tcb->id_hilo);
                            pthread_mutex_unlock(&(tcb->mutex_tcb));
                            break;   
                        case MUTEX_CREATE:
                            nombre_recurso = (char*) queue_pop(mochila->parametros);
                            pthread_mutex_unlock(&(mochila->mutex_mochila));
                    
                            crear_mutex(nombre_recurso, pid);
                            enviar_mensaje_a_CPU(ATENDER_RTA_MUTEX_CREATE, "OK");
                            safe_free(nombre_recurso);
                            break;
                        case MUTEX_LOCK:
                            nombre_recurso = (char*) queue_pop(mochila->parametros);
                            pthread_mutex_unlock(&(mochila->mutex_mochila));

                            un_mutex = obtener_mutex(nombre_recurso, pid);

                            if(un_mutex != NULL){
                                pthread_mutex_lock(&(un_mutex->mutex_id_hilo_asignado));

                                if(esta_tomado(un_mutex)){
                                    id_hilo_bloqueado = malloc(sizeof(t_hilo_id));
                                    id_hilo_bloqueado->pid_asociado = tcb->id_hilo->pid_asociado;
                                    id_hilo_bloqueado->tid = tcb->id_hilo->tid;

                                    enviar_mensaje_a_CPU(ATENDER_RTA_MUTEX_LOCK, "DESALOJO");
                                    // sem_wait(&hilo_desalojado);
                                    bloquear_hilo_en_ejecucion(tid, pid, MUTEX);
                                    pthread_mutex_lock(&(un_mutex->mutex_bloqueados));
                                    list_add(un_mutex->lista_bloqueados, id_hilo_bloqueado);
                                    pthread_mutex_unlock(&(un_mutex->mutex_bloqueados));
                                }else{
                                    enviar_mensaje_a_CPU(ATENDER_RTA_MUTEX_LOCK, "OK");
                                    pthread_mutex_lock(&(tcb->mutex_tcb));
                                    list_add(tcb->lista_mutex_asignados, un_mutex);
                                    pthread_mutex_unlock(&(tcb->mutex_tcb));
                                    
                                    asignar_mutex(un_mutex, tid, pid);
                                }

                                pthread_mutex_unlock(&(un_mutex->mutex_id_hilo_asignado));
                            }else{
                                log_warning(kernel_log_debug,"EL MUTEX DEL RECURSO %s NO EXISTE", nombre_recurso);
                                pthread_mutex_lock(&(tcb->mutex_tcb));
                                finalizar_hilo(tcb->id_hilo);
                                pthread_mutex_unlock(&(tcb->mutex_tcb));
                            }
                            
                            safe_free(nombre_recurso);
                            break;
                        case MUTEX_UNLOCK:
                            nombre_recurso = (char*) queue_pop(mochila->parametros);
                            pthread_mutex_unlock(&(mochila->mutex_mochila));

                            un_mutex = obtener_mutex(nombre_recurso, pid);

                            if(un_mutex != NULL){
                                pthread_mutex_lock(&(un_mutex->mutex_id_hilo_asignado));

                                if(esta_tomado_por(un_mutex, tid, pid)){

                                    atender_liberacion_de_recurso(un_mutex, tcb);
                                }
                                else
                                    log_warning(kernel_log_debug,"EL TID %d DEL PID %d NO TIENE TOMADO EL RECURSO %s", tid, pid, nombre_recurso);

                                pthread_mutex_unlock(&(un_mutex->mutex_id_hilo_asignado));
                                enviar_mensaje_a_CPU(ATENDER_RTA_MUTEX_UNLOCK, "OK");
                            }else{
                                log_warning(kernel_log_debug,"EL MUTEX DEL RECURSO %s NO EXISTE", nombre_recurso);
                                pthread_mutex_lock(&(tcb->mutex_tcb));
                                finalizar_hilo(tcb->id_hilo);
                                pthread_mutex_unlock(&(tcb->mutex_tcb));
                            }

                            safe_free(nombre_recurso);
                            break;
                        case DUMP_MEMORY:
                            pthread_mutex_unlock(&(mochila->mutex_mochila));
                            pthread_mutex_lock(&(tcb->mutex_tcb));
                            tcb->flag_cancelar_quantum = true;
                            pthread_mutex_unlock(&(tcb->mutex_tcb));
                            ejecutar_en_un_hilo_nuevo_detach((void*) atender_dump_memory, (void*) tcb);
                            pcp_planificador_corto_plazo();
                            break;
                        case IO:
                            pthread_mutex_lock(&(tcb->mutex_tcb));
                            tcb->flag_cancelar_quantum = true;
                            pthread_mutex_unlock(&(tcb->mutex_tcb));
                            puntero_tiempo_sleep = (int*) queue_pop(mochila->parametros);
                            pthread_mutex_unlock(&(mochila->mutex_mochila));
                            tiempo_sleep = *puntero_tiempo_sleep;

                            safe_free(puntero_tiempo_sleep);

                            t_sleep * tcb_sleep = malloc(sizeof(t_sleep));
                            tcb_sleep->tcb = tcb;
                            tcb_sleep->tiempo_sleep = tiempo_sleep;
                            
                            ejecutar_en_un_hilo_nuevo_detach((void*) atender_io, (void*) tcb_sleep);
                            break;
                        default:
                            pthread_mutex_unlock(&(mochila->mutex_mochila));
                            log_error(kernel_log_debug, "INSTRUCCION NO RECONOCIDA");
                            break;
                    }
                    
                }
                if(mochila != NULL){
                    destruir_mochila(mochila); 
                    mochila = NULL;
                }
                break;
                
            case ATENDER_DESALOJO_HILO_CPU:
                // sem_post(&hilo_desalojado);

                // log_error(kernel_log_debug, "RECIBI UNA INTERRUPCION");
                unBuffer = recibir_paquete(fd_cpu_dispatch);

                pid = recibir_int_del_buffer(unBuffer);
                tid = recibir_int_del_buffer(unBuffer);
                verificador = recibir_int_del_buffer(unBuffer);
                bool hubo_quantum = recibir_int_del_buffer(unBuffer);

                destruir_buffer(unBuffer);

                tcb = obtener_hilo_desalojado(pid, tid, verificador);

                if(tcb == NULL){
                    log_error(kernel_log_debug,"TCB ES NULL ATENDER PROCESO, SE CANCELA LA INTERRUPCION");
                    break;
                }

                // log_error(kernel_log_debug, "RECIBI UNA INTERRUPCION DEL TID %d PID %d", tcb->id_hilo->tid, tcb->id_hilo->pid_asociado);

                pthread_mutex_lock(&(tcb->mutex_tcb));
                tcb->flag_cancelar_quantum = true;
                pthread_mutex_unlock(&(tcb->mutex_tcb));
                
                atender_desalojo(tcb, hubo_quantum);
                break;
            case -1:
                if(kernel_log_debug != NULL)
                    printf("Desconexión de cpu dispatch\n");
                control_key = 0;
                break;
            default:
                log_warning(kernel_log_debug, "Operacion desconocida");
                break;
            }
    }
}

void atender_io(t_sleep * tcb_sleep){


    pthread_mutex_lock(&mutex_lista_en_espera_de_io);
    list_add(lista_en_espera_de_io, tcb_sleep);
    pthread_mutex_unlock(&mutex_lista_en_espera_de_io);

    log_warning(kernel_log_debug, "AGREGO HILO %d DEL PROCESO %d A LA LISTA ESPERA IO", tcb_sleep->tcb->id_hilo->tid, tcb_sleep->tcb->id_hilo->pid_asociado);
    log_info(kernel_log_listas_io, "AGREGO HILO %d DEL PROCESO %d A LA LISTA ESPERA IO", tcb_sleep->tcb->id_hilo->tid, tcb_sleep->tcb->id_hilo->pid_asociado);

    bloquear_hilo_en_ejecucion(tcb_sleep->tcb->id_hilo->tid, tcb_sleep->tcb->id_hilo->pid_asociado, _IO);

    pthread_mutex_lock(&mutex_lista_en_espera_de_io);
    if(list_size(lista_en_espera_de_io) == 1){
        pthread_mutex_unlock(&mutex_lista_en_espera_de_io);
        manejar_sleep(tcb_sleep);
    }else
        pthread_mutex_unlock(&mutex_lista_en_espera_de_io);
    
}

void manejar_sleep(void* arg){
    t_sleep * tcb_sleep = (t_sleep*) arg;
    int tiempo_sleep = tcb_sleep->tiempo_sleep;
    t_tcb* un_tcb = tcb_sleep->tcb;
    log_trace(kernel_log_debug, "PROCESO %d CUYO HILO ES %d ESPERANDO %d", un_tcb->id_hilo->pid_asociado, un_tcb->id_hilo->tid, tcb_sleep->tiempo_sleep);
    log_info(kernel_log_listas_io, "PROCESO %d CUYO HILO ES %d ESPERANDO %d", un_tcb->id_hilo->pid_asociado, un_tcb->id_hilo->tid, tcb_sleep->tiempo_sleep);

    pthread_mutex_lock(&mutex_lista_en_espera_de_io);
    log_warning(kernel_log_debug, "HILOS EN LISTA ESPERA IO: %d", list_size(lista_en_espera_de_io));
    pthread_mutex_unlock(&mutex_lista_en_espera_de_io);
    
    usleep(tiempo_sleep*1000);

    pthread_mutex_lock(&mutex_lista_en_espera_de_io);
    list_remove(lista_en_espera_de_io, 0);
    pthread_mutex_unlock(&mutex_lista_en_espera_de_io);

    log_info(kernel_log_listas_io, "REMUEVO HILO %d DEL PROCESO %d DE LA LISTA ESPERA IO", tcb_sleep->tcb->id_hilo->tid, tcb_sleep->tcb->id_hilo->pid_asociado);

    mostrarListaEsperaIO();

    desbloquear_hilo(un_tcb->id_hilo);

    log_info(kernel_log_obligatorio, "## (%d:%d) finalizó IO y pasa a READY", un_tcb->id_hilo->pid_asociado, un_tcb->id_hilo->tid);

    safe_free(tcb_sleep);

    pthread_mutex_lock(&mutex_lista_en_espera_de_io);
    if(list_size(lista_en_espera_de_io) > 0){

        t_sleep * nuevo_tcb_sleep = list_get(lista_en_espera_de_io, 0);
        pthread_mutex_unlock(&mutex_lista_en_espera_de_io);

        log_info(kernel_log_listas_io, "REMUEVO HILO %d DEL PROCESO %d DE LA LISTA ESPERA IO", nuevo_tcb_sleep->tcb->id_hilo->tid, nuevo_tcb_sleep->tcb->id_hilo->pid_asociado);
        
        pcp_planificador_corto_plazo();
        manejar_sleep(nuevo_tcb_sleep);
    }else
        pthread_mutex_unlock(&mutex_lista_en_espera_de_io);
}

void atender_dump_memory(void* tcb){
    t_tcb* un_tcb = (t_tcb*) tcb;
    bloquear_hilo_en_ejecucion(un_tcb->id_hilo->tid, un_tcb->id_hilo->pid_asociado, _DUMP_MEMORY);
    pthread_mutex_lock(&(un_tcb->mutex_tcb));
    int tid = un_tcb->id_hilo->tid;
    int pid = un_tcb->id_hilo->pid_asociado;
    pthread_mutex_unlock(&(un_tcb->mutex_tcb));
    avisar_a_memoria_para_dumpeo(tid, pid);
    
    sem_wait(&llego_respuesta_dumpeo);
    log_trace(kernel_log_debug, "LLEGO %s", rta_dumpeo);

    pthread_mutex_lock(&mutex_rta_dumpeo);

    if(strcmp(rta_dumpeo, "OK") == 0){
        desbloquear_hilo(un_tcb->id_hilo);
    }else{
        int* puntero_a_pid = malloc(sizeof(int));
        pthread_mutex_lock(&(un_tcb->mutex_tcb));
        *puntero_a_pid = un_tcb->id_hilo->pid_asociado;
        pthread_mutex_unlock(&(un_tcb->mutex_tcb));
        log_info(kernel_log_largo_plazo, "No hay espacio suficiente para DUMP_MEMORY. Mando a finalizar al proceso %d", *puntero_a_pid);
        ejecutar_en_un_hilo_nuevo_detach((void*) finalizar_proceso, (void*) puntero_a_pid);
    }

    pthread_mutex_unlock(&mutex_rta_dumpeo);
}