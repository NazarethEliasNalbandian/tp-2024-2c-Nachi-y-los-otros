#include "../src/inicializar_memoria.h"

void inicializar_memoria(char* archivo_config){

	inicializar_configs(archivo_config);
	inicializar_logs();
	iniciar_semaforos();
	iniciar_estructuras();
	iniciar_pthreads();
	baseSiguienteParticion = 0;
	particionID = 1; 
}

void inicializar_logs(){
	memoria_log_debug = log_create("memoria_debug.log", "[Memoria - Debug]",1,LOG_LEVEL_TRACE);

	if(memoria_log_debug == NULL)
	{
		printf("Error al crear el logger");
		exit(1);
	}

	memoria_log_obligatorio = log_create("memoria_log_obligatorio.log", "[Memoria - Log Obligatorio]",1,log_level_from_string(LOG_LEVEL));

	if(memoria_log_obligatorio == NULL)
	{
		printf("Error al crear el logger");
		exit(1);
	}

	memoria_log_espacio_usuario = log_create("memoria_log_espacio_usuario.log", "[Memoria - Log Espacio Usuario]",0,log_level_from_string(LOG_LEVEL));

	if(memoria_log_espacio_usuario == NULL)
	{
		printf("Error al crear el logger");
		exit(1);
	}

	memoria_log_particion = log_create("memoria_log_particion.log", "[Memoria - Log Particion]",0,log_level_from_string(LOG_LEVEL));
}

void inicializar_configs(char* archivo_config){

	if((memoria_config = config_create(archivo_config)) == NULL)
	{
		printf("Error al crear el archivo de configuracion");
		exit(2);
	}

	PUERTO_ESCUCHA  = config_get_string_value(memoria_config, "PUERTO_ESCUCHA");
	IP_FILESYSTEM = config_get_string_value(memoria_config, "IP_FILESYSTEM");
	PUERTO_FILESYSTEM  = config_get_string_value(memoria_config, "PUERTO_FILESYSTEM");
	TAM_MEMORIA = config_get_int_value(memoria_config, "TAM_MEMORIA");
	PATH_INSTRUCCIONES = config_get_string_value(memoria_config, "PATH_INSTRUCCIONES");
	RETARDO_RESPUESTA = config_get_int_value(memoria_config, "RETARDO_RESPUESTA");
	esquema = config_get_string_value(memoria_config, "ESQUEMA");
	PATH_INSTRUCCIONES = config_get_string_value(memoria_config, "ESQUEMA");
	algoritmo_busqueda = config_get_string_value(memoria_config, "ALGORITMO_BUSQUEDA");
	LOG_LEVEL = config_get_string_value(memoria_config, "LOG_LEVEL");

	if(strcmp(algoritmo_busqueda, "FIRST") == 0) {
		ALGORITMO_BUSQUEDA = FIRST;
		}
	else if (strcmp(algoritmo_busqueda, "BEST") == 0) {
		ALGORITMO_BUSQUEDA = BEST;
		}
	else if (strcmp(algoritmo_busqueda, "WORST") == 0) {
		ALGORITMO_BUSQUEDA = WORST;
		}
	else {
		log_error(memoria_log_debug, "No se encontro el algoritmo de busqueda");
	}

	if(strcmp(esquema, "FIJAS") == 0){
		ESQUEMA = FIJAS;
	}else if(strcmp(esquema, "DINAMICAS") == 0)
	{
		ESQUEMA = DINAMICAS;
	}else	
		log_error(memoria_log_debug, "No se encontro el esquema");

	if(ESQUEMA == FIJAS){
		PARTICIONES = config_get_array_value(memoria_config, "PARTICIONES");
	}
}

void iniciar_estructuras(){
	espacio_usuario = malloc(TAM_MEMORIA);
	
	if(espacio_usuario == NULL){
		log_error(memoria_log_debug, "Fallo Malloc");
		exit(1);
	}

	lista_procesos_recibidos = list_create();
	list_hilos_recibidos = list_create();
	lista_particiones = list_create();
	iniciar_particiones();
}

void iniciar_particiones(){
	if(ESQUEMA == FIJAS){
		int i = 0;
		int suma_limite = 0;

		for(i=0; i < string_array_size(PARTICIONES); i++){
			t_particion* particion = malloc(sizeof(t_particion));
			particion->tamanio = atoi(PARTICIONES[i]);
			particion->particion_id = i;
			particion->esta_ocupada = false;
			particion->base = suma_limite;
			particion->limite = particion->base + particion->tamanio - 1;
			pthread_mutex_init(&(particion->mutex_particion), NULL);
			suma_limite += particion->tamanio;
			particion->proceso_en_particion = NULL;

			list_add(lista_particiones, particion);

			log_info(memoria_log_particion, "Partición: %d Tamaño: %d Base: %d Límite: %d", particion->particion_id, particion->tamanio, particion->base, particion->limite);
		}
		string_array_destroy(PARTICIONES);
	}else if(ESQUEMA == DINAMICAS){
		t_particion* particion = malloc(sizeof(t_particion));
		particion->tamanio = TAM_MEMORIA;
		particion->particion_id = 0;
		particion->esta_ocupada = false;
		particion->base = 0;
		particion->limite = particion->base + particion->tamanio - 1;
		pthread_mutex_init(&(particion->mutex_particion), NULL);
		particion->proceso_en_particion = NULL;

		list_add(lista_particiones, particion);
	}
}

void iniciar_semaforos(){
	sem_init(&se_desconecto_cpu, 0, 0);
	sem_init(&llego_respuesta_memory_dump_fs, 0, 0);
}

void iniciar_pthreads(){
	pthread_mutex_init(&mutex_lst_procss_recibidos, NULL);
	pthread_mutex_init(&mutex_lst_hilos_recibidos, NULL);
	pthread_mutex_init(&mutex_particionID, NULL);
	pthread_mutex_init(&mutex_lista_particiones, NULL);
	pthread_mutex_init(&mutex_respuesta_memory_dump_fs, NULL);
	pthread_mutex_init(&mutex_fd_memoria, NULL);
}