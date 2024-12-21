#include "../src/ciclo_instruccion.h"
#include "../src/cpu_kernel_dispatch.h"
#include "../src/mmu.h"
#include "../src/servicios_cpu.h"

void fetch()
{
    // PODRIAMOS HACER Q MEMORIA NOS ENVIE EL PROGRAM COUNTER MAXIMO DE UN CONTEXTO Y VALIDAR QUE EL PROGRAM COUNTER NO SEA MAYOR A ESO PARA NO HACER ENVIOS ERRONEOS

    // log_warning(cpu_log_debug, "PC: %d \t MAXIMO PC: %d",un_contexto->r_cpu->PC, un_contexto->maximo_PC);
    if(un_contexto != NULL){
    
        if(un_contexto->r_cpu->PC <= un_contexto->maximo_PC){
            log_info(cpu_log_obligatorio, "## TID: %d - FETCH - Program Counter: %d", un_contexto->tid, un_contexto->r_cpu->PC);
            log_warning(cpu_log_debug, "## PID: %d TID: %d - FETCH - Program Counter: %d",un_contexto->pid, un_contexto->tid, un_contexto->r_cpu->PC);
            t_paquete *paquete = crear_paquete(PETICION_DE_INSTRUCCION_CPU_MEMORIA);
            cargar_int_al_paquete(paquete, un_contexto->pid);
            cargar_int_al_paquete(paquete, un_contexto->tid);
            cargar_uint32_al_paquete(paquete, un_contexto->r_cpu->PC);
            enviar_paquete(paquete, fd_memoria);
            eliminar_paquete(paquete);
        }else{

            log_error(cpu_log_debug, "EVITE ERROR PC");
            pthread_mutex_unlock(&mutex_tipo_desalojo);
        }
    }
        

}

void decode()
{
    pthread_mutex_lock(&mutex_instruccion_split);
	if(validador_de_header(instruccion_split[0])){
		// log_info(cpu_logger, "Instruccion Validada: [%s] -> OK", instruccion_split[0]);
		sem_post(&sem_decode);
	}else{
        if(un_contexto != NULL)
		    log_error(cpu_log_debug, "Instruccion no encontrada: [PC: %d] [Instruc_Header: %s]", un_contexto->r_cpu->PC, instruccion_split[0]);
        tipo_desalojo = ATENDER_THREAD_EXIT;
        hay_exit = true;
	}
    pthread_mutex_unlock(&mutex_instruccion_split);
}


uint32_t solicitar_memoria(int dir_logica)
{
    int dir_fisica = MMU(dir_logica);

    if (dir_fisica == -1)
    {
        return -1;
    }
    else
    {
        t_paquete *paqueteLecturaMemoria = crear_paquete(LECTURA_CPU_MEMORIA);
        if(un_contexto != NULL){
            cargar_int_al_paquete(paqueteLecturaMemoria, un_contexto->pid);
            cargar_int_al_paquete(paqueteLecturaMemoria, un_contexto->tid);
            cargar_int_al_paquete(paqueteLecturaMemoria, dir_fisica);
        }

        enviar_paquete(paqueteLecturaMemoria, fd_memoria);
        eliminar_paquete(paqueteLecturaMemoria);

        sem_wait(&sem_val_leido);

        if(un_contexto != NULL)
            log_info(cpu_log_obligatorio, "## TID: %d - Acción: LEER - Dirección Física: %d", un_contexto->tid, dir_fisica);

        return valor_leido;
    }
}

int escribir_valor_memoria(uint32_t dir_logica, uint32_t valorAEscribir)
{
    int dir_fisica = MMU(dir_logica);

    if (dir_fisica != -1){
        t_paquete *paqueteEscrituraMemoria = crear_paquete(ESCRITURA_CPU_MEMORIA);
        if(un_contexto != NULL){
            cargar_int_al_paquete(paqueteEscrituraMemoria, un_contexto->pid);
            cargar_int_al_paquete(paqueteEscrituraMemoria, un_contexto->tid);
        }
        cargar_int_al_paquete(paqueteEscrituraMemoria, dir_fisica);
        cargar_uint32_al_paquete(paqueteEscrituraMemoria, valorAEscribir);

        enviar_paquete(paqueteEscrituraMemoria, fd_memoria);
        eliminar_paquete(paqueteEscrituraMemoria);

        sem_wait(&sem_val_escrito);
        if(un_contexto != NULL)
            log_info(cpu_log_obligatorio, "## TID: %d - Acción: ESCRIBIR - Dirección Física: %d", un_contexto->tid, dir_fisica);
        return 0;
    }
    else{
        return -1;
    }
}

void execute()
{
    char* nombre_instruccion = instruccion_split[0]; 

    nombre_instruccion_enum = convertirInstruccionAEnum(nombre_instruccion);

    t_paquete * paquete = NULL;
    int dir_logica;
    int resultado;
    uint32_t *r = NULL;
    uint32_t *ra = NULL;
    uint32_t *rb = NULL;
    u_int32_t* registroAEscribir = NULL;
    uint32_t valorAEscribir;
    int cantParametros;
    t_registros nombre_registro_enum;

    switch (nombre_instruccion_enum)
    {
        case SET:
            if(un_contexto != NULL)
                un_contexto->r_cpu->PC += 1;
            // SET (Registro, Valor)
            if(un_contexto != NULL)
                log_info(cpu_log_obligatorio, "## TID: %d - Ejecutando: %s - %s - %s", un_contexto->tid, instruccion_split[0], instruccion_split[1], instruccion_split[2]);

            nombre_registro_enum = convertirRegistroAEnum(instruccion_split[1]);
            r = detectar_registro(nombre_registro_enum); // REGISTRO

            *r = (uint32_t) atoi(instruccion_split[2]); // VALOR

            break;
        case READ_MEM:
            if(un_contexto != NULL)
                un_contexto->r_cpu->PC += 1;
            // READ_MEM (Registro Datos, Registro Dirección)
            if(un_contexto != NULL)
                log_info(cpu_log_obligatorio, "## TID: %d - Ejecutando: %s - %s - %s", un_contexto->tid, instruccion_split[0], instruccion_split[1], instruccion_split[2]);

            nombre_registro_enum = convertirRegistroAEnum(instruccion_split[2]);
            dir_logica = obtenerIntDeRegistro(nombre_registro_enum,&r); // REGISTRO DIRECCION

            valorAEscribir = solicitar_memoria(dir_logica);

            nombre_registro_enum = convertirRegistroAEnum(instruccion_split[1]);

            if(valorAEscribir == -1){
                atender_SEGMENTATION_FAULT();
            }
            else
                escribir_registro(nombre_registro_enum, valorAEscribir, registroAEscribir); // REGISTRO DATOS
            
            break;
        case WRITE_MEM:
            if(un_contexto != NULL)
                un_contexto->r_cpu->PC += 1;
            // WRITE_MEM (Registro Dirección, Registro Datos)
            if(un_contexto != NULL)
                log_info(cpu_log_obligatorio, "## TID: %d - Ejecutando: %s - %s - %s", un_contexto->tid, instruccion_split[0], instruccion_split[1], instruccion_split[2]);
            
            nombre_registro_enum = convertirRegistroAEnum(instruccion_split[1]);
            dir_logica = obtenerIntDeRegistro(nombre_registro_enum, &r);

            nombre_registro_enum = convertirRegistroAEnum(instruccion_split[2]);

            asignarRegistro(nombre_registro_enum, &registroAEscribir);
            valorAEscribir = *registroAEscribir;
            // log_trace(cpu_log_debug,"VALOR A ESCRIBIR: %d", (int) valorAEscribir);
            resultado = escribir_valor_memoria((uint32_t) dir_logica, valorAEscribir);
            
            if(resultado == -1){
                atender_SEGMENTATION_FAULT();
            }
            
            break;
        case SUM:
            // SUM (Registro Destino, Registro Origen)
            if(un_contexto != NULL)
                un_contexto->r_cpu->PC += 1;
            if(un_contexto != NULL)
                log_info(cpu_log_obligatorio, "## TID: %d - Ejecutando: %s - %s - %s", un_contexto->tid, instruccion_split[0], instruccion_split[1], instruccion_split[2]);


            nombre_registro_enum = convertirRegistroAEnum(instruccion_split[1]);
            ra = detectar_registro(nombre_registro_enum); // REGISTRO DESTINO

            nombre_registro_enum = convertirRegistroAEnum(instruccion_split[2]);
            rb = detectar_registro(nombre_registro_enum); // REGISTRO ORIGEN
            
            *ra = (*ra) + (*rb);
            
            break;
        case SUB:
            // SUB (Registro Destino, Registro Origen)
            if(un_contexto != NULL)
                un_contexto->r_cpu->PC += 1;
            if(un_contexto != NULL)
                log_info(cpu_log_obligatorio, "## TID: %d - Ejecutando: %s - %s - %s", un_contexto->tid, instruccion_split[0], instruccion_split[1], instruccion_split[2]);

            nombre_registro_enum = convertirRegistroAEnum(instruccion_split[1]);
            ra = detectar_registro(nombre_registro_enum); // REGISTRO DESTINO

            nombre_registro_enum = convertirRegistroAEnum(instruccion_split[2]);
            rb = detectar_registro(nombre_registro_enum); // REGISTRO ORIGEN
            *ra = (*ra) - (*rb);

            break;
        case JNZ:
            // JNZ (Registro Destino, Instruccion)
            if(un_contexto != NULL)
                un_contexto->r_cpu->PC += 1;
            if(un_contexto != NULL)
                log_info(cpu_log_obligatorio, "## TID: %d - Ejecutando: %s - %s - %s", un_contexto->tid, instruccion_split[0], instruccion_split[1], instruccion_split[2]);

            nombre_registro_enum = convertirRegistroAEnum(instruccion_split[1]);
            r = detectar_registro(nombre_registro_enum); // REGISTRO DESTINO
            
            if (*r != 0)
            {
                if(un_contexto != NULL)
                    un_contexto->r_cpu->PC = (u_int32_t) atoi(instruccion_split[2]); // INSTRUCCION
            }
            
            break;
        case LOG:
            // LOG (Registro)
            if(un_contexto != NULL)
                un_contexto->r_cpu->PC += 1;
            if(un_contexto != NULL)
                log_info(cpu_log_obligatorio, "## TID: %d - Ejecutando: %s - %s", un_contexto->tid, instruccion_split[0], instruccion_split[1]);

            if(instruccion_split[1] != NULL)
                nombre_registro_enum = convertirRegistroAEnum(instruccion_split[1]);
                
            r = detectar_registro(nombre_registro_enum); // REGISTRO

            log_info(cpu_log_obligatorio, "Valor: %u", *r);
            break;
        case IO:
            // IO (Tiempo)
            if(un_contexto != NULL){
                un_contexto->r_cpu->PC += 1;
                log_info(cpu_log_obligatorio, "## TID: %d - Ejecutando: %s - %s", un_contexto->tid, instruccion_split[0], instruccion_split[1]);
            }

            cantParametros = 1;
            paquete = crear_paquete(ATENDER_INSTRUCCION_CPU);
            if(un_contexto != NULL)
                cargar_hilo_a_paquete(paquete, un_contexto->pid, un_contexto->tid, un_contexto->verificador);
            cargar_int_al_paquete(paquete, IO);
            cargar_int_al_paquete(paquete, cantParametros);

            // PARAMETROS
            cargar_int_al_paquete(paquete, T_INT);
            cargar_int_al_paquete(paquete, atoi(instruccion_split[1])); // TIEMPO

            enviar_paquete(paquete, fd_kernel_dispatch);
            eliminar_paquete(paquete);

            break;
        case DUMP_MEMORY:
            // DUMP_MEMORY
            if(un_contexto != NULL){
                un_contexto->r_cpu->PC += 1;
                log_info(cpu_log_obligatorio, "## TID: %d - Ejecutando: %s", un_contexto->tid, instruccion_split[0]);
            }

            cantParametros = 0;
            paquete = crear_paquete(ATENDER_INSTRUCCION_CPU);
            if(un_contexto != NULL)
                cargar_hilo_a_paquete(paquete, un_contexto->pid, un_contexto->tid, un_contexto->verificador);
            cargar_int_al_paquete(paquete, DUMP_MEMORY);
            cargar_int_al_paquete(paquete, cantParametros);

            enviar_paquete(paquete, fd_kernel_dispatch);
            eliminar_paquete(paquete);

            break;
        case MUTEX_CREATE:
            // MUTEX_CREATE (Recurso)
            if(un_contexto != NULL){
                un_contexto->r_cpu->PC += 1;
                log_info(cpu_log_obligatorio, "## TID: %d - Ejecutando: %s - %s", un_contexto->tid, instruccion_split[0], instruccion_split[1]);
            }

            paquete = crear_paquete(ATENDER_INSTRUCCION_CPU);
            if(un_contexto != NULL)
                cargar_hilo_a_paquete(paquete, un_contexto->pid, un_contexto->tid, un_contexto->verificador);
            cargar_int_al_paquete(paquete, MUTEX_CREATE);
            cantParametros = 1;

            cargar_int_al_paquete(paquete, cantParametros);
            cargar_int_al_paquete(paquete, T_STRING);
            cargar_string_al_paquete(paquete, instruccion_split[1]); // NOMBRE MUTEX
            enviar_paquete(paquete, fd_kernel_dispatch);
            eliminar_paquete(paquete);

            sem_wait(&sem_rta_mutex_create);
            pthread_mutex_lock(&mutex_interruptFlag);
		    interrupt_flag = 1;
		    pthread_mutex_unlock(&mutex_interruptFlag);
            break;
        case MUTEX_LOCK:
            // MUTEX_LOCK (Recurso)
            if(un_contexto != NULL){
                un_contexto->r_cpu->PC += 1;
                log_info(cpu_log_obligatorio, "## TID: %d - Ejecutando: %s - %s", un_contexto->tid, instruccion_split[0], instruccion_split[1]);
            }

            paquete = crear_paquete(ATENDER_INSTRUCCION_CPU);
            if(un_contexto != NULL)
                cargar_hilo_a_paquete(paquete, un_contexto->pid, un_contexto->tid, un_contexto->verificador);
            cargar_int_al_paquete(paquete, MUTEX_LOCK);
            cantParametros = 1;

            cargar_int_al_paquete(paquete, cantParametros);
            cargar_int_al_paquete(paquete, T_STRING);
            cargar_string_al_paquete(paquete, instruccion_split[1]); // NOMBRE MUTEX
            enviar_paquete(paquete, fd_kernel_dispatch);
            eliminar_paquete(paquete);

            sem_wait(&sem_rta_mutex_lock);
            break;
        case MUTEX_UNLOCK:
            // MUTEX_UNLOCK (Recurso)
            if(un_contexto != NULL){
                un_contexto->r_cpu->PC += 1;
                log_info(cpu_log_obligatorio, "## TID: %d - Ejecutando: %s - %s", un_contexto->tid, instruccion_split[0], instruccion_split[1]);
            }

            paquete = crear_paquete(ATENDER_INSTRUCCION_CPU);
            if(un_contexto != NULL)
                cargar_hilo_a_paquete(paquete, un_contexto->pid, un_contexto->tid, un_contexto->verificador);
            cargar_int_al_paquete(paquete, MUTEX_UNLOCK);
            cantParametros = 1;

            cargar_int_al_paquete(paquete, cantParametros);

            cargar_int_al_paquete(paquete, T_STRING);
            cargar_string_al_paquete(paquete, instruccion_split[1]); // NOMBRE MUTEX

            enviar_paquete(paquete, fd_kernel_dispatch);
            eliminar_paquete(paquete);

            sem_wait(&sem_rta_mutex_unlock);
            // pthread_mutex_lock(&mutex_interruptFlag);
		    // interrupt_flag = 1;
		    // pthread_mutex_unlock(&mutex_interruptFlag);
            break;
        case PROCESS_CREATE:
            // PROCESS_CREATE (Archivo) (Tamaño) (Prioridad_TID_0)
            if(un_contexto != NULL){
                un_contexto->r_cpu->PC += 1;
                log_info(cpu_log_obligatorio, "## TID: %d - Ejecutando: %s - %s - %s - %s", un_contexto->tid, instruccion_split[0], instruccion_split[1], instruccion_split[2], instruccion_split[3]);
            }

            paquete = crear_paquete(ATENDER_INSTRUCCION_CPU);
            if(un_contexto != NULL)
                cargar_hilo_a_paquete(paquete, un_contexto->pid, un_contexto->tid, un_contexto->verificador);
            cargar_int_al_paquete(paquete, PROCESS_CREATE);
            cantParametros = 3;

            cargar_int_al_paquete(paquete, cantParametros);

            cargar_int_al_paquete(paquete, T_STRING);
            cargar_string_al_paquete(paquete, instruccion_split[1]); // NOMBRE ARCHIVO INSTRUCCIONES

            cargar_int_al_paquete(paquete, T_SIZE_T);
            cargar_size_t_al_paquete(paquete, (size_t) atoi(instruccion_split[2])); // TAMAÑO PROCESO

            cargar_int_al_paquete(paquete, T_INT);
            cargar_int_al_paquete(paquete, atoi(instruccion_split[3])); // PRIORIDAD TID 0

            enviar_paquete(paquete, fd_kernel_dispatch);
            eliminar_paquete(paquete);

            sem_wait(&sem_rta_process_create);
            pthread_mutex_lock(&mutex_interruptFlag);
		    interrupt_flag = 1;
		    pthread_mutex_unlock(&mutex_interruptFlag);
            break;
        case PROCESS_EXIT:
            // PROCESS_EXIT
            if(un_contexto != NULL){
                un_contexto->r_cpu->PC += 1;
                log_info(cpu_log_obligatorio, "## TID: %d - Ejecutando: %s", un_contexto->tid, instruccion_split[0]);
            }

            hay_exit = true;
            TIPO_EXIT = PROCESS;
            break;
        case THREAD_CREATE:
            // THREAD_CREATE (Archivo) (Prioridad)
            if(un_contexto != NULL){
                un_contexto->r_cpu->PC += 1;
                log_info(cpu_log_obligatorio, "## TID: %d - Ejecutando: %s - %s - %s", un_contexto->tid, instruccion_split[0], instruccion_split[1], instruccion_split[2]);
            }

            paquete = crear_paquete(ATENDER_INSTRUCCION_CPU);
            if(un_contexto != NULL)
                cargar_hilo_a_paquete(paquete, un_contexto->pid, un_contexto->tid, un_contexto->verificador);
            cargar_int_al_paquete(paquete, THREAD_CREATE);
            cantParametros = 2;

            cargar_int_al_paquete(paquete, cantParametros);

            cargar_int_al_paquete(paquete, T_STRING);
            cargar_string_al_paquete(paquete, instruccion_split[1]); // NOMBRE ARCHIVO INSTRUCCIONES

            cargar_int_al_paquete(paquete, T_INT);
            cargar_int_al_paquete(paquete, atoi(instruccion_split[2])); // PRIORIDAD TID 0

            enviar_paquete(paquete, fd_kernel_dispatch);
            eliminar_paquete(paquete);

            sem_wait(&sem_rta_thread_create);
            pthread_mutex_lock(&mutex_interruptFlag);
		    interrupt_flag = 1;
		    pthread_mutex_unlock(&mutex_interruptFlag);
            break;
        case THREAD_CANCEL:
            // THREAD_CANCEL (TID)
            if(un_contexto != NULL){
                un_contexto->r_cpu->PC += 1;
                log_info(cpu_log_obligatorio, "## TID: %d - Ejecutando: %s - %s", un_contexto->tid, instruccion_split[0], instruccion_split[1]);
            }

            paquete = crear_paquete(ATENDER_INSTRUCCION_CPU);
            if(un_contexto != NULL)
                cargar_hilo_a_paquete(paquete, un_contexto->pid, un_contexto->tid, un_contexto->verificador);
            cargar_int_al_paquete(paquete, (int) THREAD_CANCEL);
            cantParametros = 1;

            cargar_int_al_paquete(paquete, cantParametros);

            cargar_int_al_paquete(paquete, T_INT);
            cargar_int_al_paquete(paquete, atoi(instruccion_split[1])); // PRIORIDAD TID 0

            enviar_paquete(paquete, fd_kernel_dispatch);
            eliminar_paquete(paquete);

            sem_wait(&sem_rta_thread_cancel);
            // pthread_mutex_lock(&mutex_interruptFlag);
		    // interrupt_flag = 1;
		    // pthread_mutex_unlock(&mutex_interruptFlag);
            break;
        case THREAD_JOIN:
            // THREAD_JOIN (TID)
            if(un_contexto != NULL){
                un_contexto->r_cpu->PC += 1;
                log_info(cpu_log_obligatorio, "## TID: %d - Ejecutando: %s - %s", un_contexto->tid, instruccion_split[0], instruccion_split[1]);
            }

            paquete = crear_paquete(ATENDER_INSTRUCCION_CPU);
            if(un_contexto != NULL)
                cargar_hilo_a_paquete(paquete, un_contexto->pid, un_contexto->tid, un_contexto->verificador);
            cargar_int_al_paquete(paquete, THREAD_JOIN);
            cantParametros = 1;

            cargar_int_al_paquete(paquete, cantParametros);

            cargar_int_al_paquete(paquete, T_INT);
            cargar_int_al_paquete(paquete, atoi(instruccion_split[1])); // PRIORIDAD TID 0

            enviar_paquete(paquete, fd_kernel_dispatch);
            eliminar_paquete(paquete);

            sem_wait(&sem_rta_thread_join);
            break;
        case THREAD_EXIT:
            // THREAD_EXIT
            if(un_contexto != NULL){
                un_contexto->r_cpu->PC += 1;
                log_info(cpu_log_obligatorio, "## TID: %d - Ejecutando: %s", un_contexto->tid, instruccion_split[0]);
            }

            hay_exit = true;
            TIPO_EXIT = THREAD;
            break;
    }

    pthread_mutex_lock(&mutex_instruccion_split);
    if(instruccion_split != NULL){
        string_array_destroy(instruccion_split);
        instruccion_split = NULL;
    }
    pthread_mutex_unlock(&mutex_instruccion_split);
}