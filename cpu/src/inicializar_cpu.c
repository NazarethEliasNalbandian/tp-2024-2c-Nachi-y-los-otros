#include "../src/incializar_cpu.h"
#include "../src/dic_operaciones.h"

void inicializar_cpu(char* archivo_config){

    inicializar_configs(archivo_config);
    inicializar_logs();
    inicializar_booleans();

    un_contexto = malloc(sizeof(t_contexto));
    un_contexto->r_cpu = malloc(sizeof(t_registrosCPU));

    diccionario_operaciones();
    interrupt_motivo = NULL;
    iniciar_semaforos();
    iniciar_pthreads();
}

void inicializar_configs(char* archivo_config){
    cpu_config = config_create(archivo_config);

    if(cpu_config == NULL){
		log_error(cpu_log_debug, "No se encontro el path del config\n");
		config_destroy(cpu_config);
		log_destroy(cpu_log_debug);
		exit(2);
	}

    IP_MEMORIA               = config_get_string_value(cpu_config,"IP_MEMORIA");
    PUERTO_MEMORIA           = config_get_string_value(cpu_config,"PUERTO_MEMORIA");
    PUERTO_ESCUCHA_DISPATCH  = config_get_string_value(cpu_config,"PUERTO_ESCUCHA_DISPATCH");
    PUERTO_ESCUCHA_INTERRUPT = config_get_string_value(cpu_config,"PUERTO_ESCUCHA_INTERRUPT");
    LOG_LEVEL                = config_get_string_value(cpu_config,"LOG_LEVEL");
}

void inicializar_logs(){
    cpu_log_debug = log_create("cpu_debug.log","[CPU - Debug]",1,LOG_LEVEL_TRACE);
    cpu_log_obligatorio = log_create("cpu_log_obligatorio.log","[CPU - Log Obligatorio]",1,log_level_from_string(LOG_LEVEL));
}

void inicializar_booleans(){
    interrupt_flag = false;
    hay_exit      = false;
    instancia_ocupada = false;
    existe_hilo_a_joinear = false;
    hubo_quantum = false;
}

void iniciar_semaforos(){
    sem_init(&sem_fetch, 0, 0);
	sem_init(&sem_decode, 0, 0);
	sem_init(&sem_execute, 0, 0);
	sem_init(&sem_val_leido, 0, 0);
    sem_init(&sem_val_escrito, 0, 0);
    sem_init(&sem_rta_kernel, 0, 0);
    sem_init(&termino_ciclo_instruccion,0,1);
    sem_init(&llego_contexto,0,0);
    sem_init(&llego_contexto_una_vez,0,0);
    sem_init(&se_desconecto_kernel,0,0);
    sem_init(&contexto_actualizado,0,0);
    sem_init(&sem_rta_mutex_create,0,0);
    sem_init(&sem_rta_mutex_lock,0,0);
    sem_init(&sem_rta_mutex_unlock,0,0);
    sem_init(&sem_rta_thread_create,0,0);
    sem_init(&sem_rta_process_create,0,0);
    sem_init(&sem_rta_thread_cancel,0,0);
    sem_init(&sem_rta_thread_join,0,0);
    sem_init(&sem_hilo_en_ejecucion_desalojado, 0, 1);
}

void iniciar_pthreads(){
    pthread_mutex_init(&mutex_interruptFlag, NULL);
	pthread_mutex_init(&mutex_manejo_contexto, NULL);
    pthread_mutex_init(&mutex_tipo_desalojo, NULL);
    pthread_mutex_init(&mutex_interrupt_motivo, NULL);
    pthread_mutex_init(&mutex_instruccion_split, NULL);
    pthread_mutex_init(&mutex_VERIFICADOR, NULL);
    pthread_mutex_init(&mutex_existe_hilo_a_joinear, NULL);
    pthread_mutex_init(&mutex_hubo_quantum, NULL);
}
