#ifndef M_GESTOR_H_
#define M_GESTOR_H_

#include "../../utils/src/shared.h"

typedef enum{
	FIRST,
	BEST,
	WORST
}t_algoritmo_busqueda;

typedef struct{
	int pid;
	uint32_t base;
	uint32_t limite;
	int particion_asignada_id;
	int mayor_direccion_fisica_escrita;
	pthread_mutex_t mutex_proceso;
}t_proceso;

typedef struct{
	int tamanio;
	int particion_id;
	bool esta_ocupada;
	int base;
	int limite;
	t_proceso* proceso_en_particion;
	pthread_mutex_t mutex_particion;
}t_particion;

typedef struct {
	int pid_asociado;
	int tid;
	t_registrosCPU* registros;
	char* archivo_instrucciones;
	t_list* lista_instrucciones;
	int maximo_PC;
	pthread_mutex_t mutex_hilo;
}t_hilo;

typedef enum{
	FIJAS,
	DINAMICAS
}t_esquema;

extern int particionID;
extern int baseSiguienteParticion;

// VARIABLES GLOBALES
extern t_log* memoria_log_debug;
extern t_log* memoria_log_obligatorio;
extern t_log* memoria_log_espacio_usuario;
extern t_log* memoria_log_particion;
extern t_config* memoria_config;

extern int fd_memoria;
extern int fd_cpu;

extern char* PUERTO_ESCUCHA;
extern char* IP_FILESYSTEM;
extern char* PUERTO_FILESYSTEM;
extern int TAM_MEMORIA;
extern char* PATH_INSTRUCCIONES;
extern int RETARDO_RESPUESTA;
extern char* esquema;
extern char* algoritmo_busqueda;
extern t_algoritmo_busqueda ALGORITMO_BUSQUEDA;
extern t_esquema ESQUEMA;
extern char** PARTICIONES;
extern char* LOG_LEVEL;
extern char* respuesta_memory_dump_fs;

extern void* espacio_usuario;

extern t_list* lista_procesos_recibidos;
extern t_list* list_hilos_recibidos;
extern t_list* lista_particiones;

extern pthread_mutex_t mutex_espacio_usuario;
extern pthread_mutex_t mutex_lst_procss_recibidos;
extern pthread_mutex_t mutex_lst_hilos_recibidos;
extern pthread_mutex_t mutex_particionID;
extern pthread_mutex_t mutex_lista_particiones;
extern pthread_mutex_t mutex_respuesta_memory_dump_fs;
extern pthread_mutex_t mutex_fd_memoria;


extern sem_t se_desconecto_cpu;
extern sem_t llego_respuesta_memory_dump_fs;

#endif /* M_GESTOR_H_ */
