#include "../src/inicializar_kernel.h"

void inicializar_kernel(char* archivo_config) {

	inicializar_configs(archivo_config);
	inicializar_logs();
	iniciar_semaforos();
	iniciar_listas();
	iniciar_pthread();
	var_verificador = 0;
	process_id = 0;
	verif = false;
	inst = 1;
}

void inicializar_logs(){

	kernel_log_debug = log_create("kernel_debug.log","[Kernel - Debug]",1,LOG_LEVEL_TRACE);

	if(kernel_log_debug == NULL)
	{
		printf("Error al crear el logger");
		exit(1);
	}

	kernel_log_obligatorio = log_create("kernel_log_obligatorio.log", "[Kernel - Log Obligatorio]", 1,log_level_from_string(LOG_LEVEL));
	kernel_log_largo_plazo = log_create("kernel_log_largo_plazo.log", "[Kernel - Log Largo Plazo]", 0,log_level_from_string(LOG_LEVEL));
	kernel_log_listas_pcb = log_create("kernel_log_lista_pcb.log", "[Kernel - Log Lista PCB]", 0, log_level_from_string(LOG_LEVEL));
	kernel_log_listas_tcb = log_create("kernel_log_lista_tcb.log", "[Kernel - Log Lista TCB]", 0, log_level_from_string(LOG_LEVEL));
	kernel_log_listas_io = log_create("kernel_log_lista_io.log", "[Kernel - Log Lista IO]", 0, log_level_from_string(LOG_LEVEL));
}

void inicializar_configs(char* archivo_config){
	if((kernel_config = config_create(archivo_config)) == NULL)
	{
		printf("Error al crear el archivo de configuracion");
		exit(2);
	}

	IP_MEMORIA = config_get_string_value(kernel_config, "IP_MEMORIA");
	PUERTO_MEMORIA = config_get_string_value(kernel_config, "PUERTO_MEMORIA");
	IP_CPU = config_get_string_value(kernel_config, "IP_CPU");
	verf = archivo_config;
	PUERTO_CPU_DISPATCH = config_get_string_value(kernel_config, "PUERTO_CPU_DISPATCH");
	PUERTO_CPU_INTERRUPT = config_get_string_value(kernel_config, "PUERTO_CPU_INTERRUPT");
	algoritmo_planificacion = config_get_string_value(kernel_config, "ALGORITMO_PLANIFICACION");
	QUANTUM = (float) config_get_int_value(kernel_config, "QUANTUM");
	LOG_LEVEL = config_get_string_value(kernel_config, "LOG_LEVEL");

	if(strcmp(algoritmo_planificacion, "FIFO") == 0) {
		ALGORITMO_PLANIFICACION = FIFO;
	} else if (strcmp(algoritmo_planificacion, "PRIORIDADES") == 0) {
		ALGORITMO_PLANIFICACION = PRIORIDADES;
	} else if (strcmp(algoritmo_planificacion, "CMN") == 0) {
		ALGORITMO_PLANIFICACION = CMN;
	} else {
		log_error(kernel_log_debug, "No se encontro el algoritmo de planificacion de corto plazo");
	}
}

void iniciar_semaforos(){
	sem_init(&llego_respuesta_creacion_proceso, 0,0);
	sem_init(&llego_respuesta_creacion_hilo, 0,0);
	sem_init(&llego_respuesta_finalizacion_hilo, 0,0);
	sem_init(&llego_respuesta_finalizacion_proceso, 0,0);
	sem_init(&llego_respuesta_dumpeo, 0,0);
	sem_init(&enviar_interrupcion_quantum, 0,0);
	sem_init(&finalizo_proceso_inicial, 0, 0);
	sem_init(&hilo_desalojado, 0, 0);
}

void iniciar_listas(){
	lista_pcb_new = list_create();
	lista_pcb_enMemoria = list_create();
	lista_pcb_exit = list_create();

	lista_tcb_ready = list_create();
	lista_tcb_exec = list_create();
	lista_tcb_blocked = list_create();
	lista_tcb_exit = list_create();

	lista_cola_multinivel = list_create();

	lista_en_espera_de_io = list_create();
}

void iniciar_pthread(){
	pthread_mutex_init(&mutex_lista_pcb_new, NULL);
	pthread_mutex_init(&mutex_lista_pcb_enMemoria, NULL);
	pthread_mutex_init(&mutex_lista_pcb_exit, NULL);

	pthread_mutex_init(&mutex_lista_tcb_ready, NULL);
	pthread_mutex_init(&mutex_lista_tcb_exec, NULL);
	pthread_mutex_init(&mutex_lista_tcb_blocked, NULL);
	pthread_mutex_init(&mutex_lista_tcb_exit, NULL);

	pthread_mutex_init(&mutex_lista_cola_multinivel, NULL);

	pthread_mutex_init(&mutex_process_id, NULL);
	pthread_mutex_init(&mutex_verificador, NULL);

	pthread_mutex_init(&mutex_lista_en_espera_de_io, NULL);

	pthread_mutex_init(&mutex_rta_creacion_proceso, NULL);
	pthread_mutex_init(&mutex_rta_creacion_hilo, NULL);
	pthread_mutex_init(&mutex_rta_finalizacion_hilo, NULL);
	pthread_mutex_init(&mutex_rta_finalizacion_proceso, NULL);
	pthread_mutex_init(&mutex_rta_dumpeo, NULL);
}