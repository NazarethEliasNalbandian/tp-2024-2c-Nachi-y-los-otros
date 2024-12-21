#ifndef MEMORIA_H_
#define MEMORIA_H_

#include "m_gestor.h"
#include "inicializar_memoria.h"
#include "memoria_filesystem.h"
#include "memoria_cpu.h"
#include "memoria_kernel.h"

// VARIABLES GLOBALES
t_log* memoria_log_debug;
t_log* memoria_log_obligatorio;
t_log* memoria_log_espacio_usuario;
t_log* memoria_log_particion;
t_config* memoria_config;

int fd_memoria;
int fd_cpu;

char* PUERTO_ESCUCHA;
char* IP_FILESYSTEM;
char* PUERTO_FILESYSTEM;
int TAM_MEMORIA;
char* PATH_INSTRUCCIONES;
int RETARDO_RESPUESTA;
char* esquema;
char* algoritmo_busqueda;
t_algoritmo_busqueda ALGORITMO_BUSQUEDA;
t_esquema ESQUEMA;
char** PARTICIONES;
char* LOG_LEVEL;
char* respuesta_memory_dump_fs;

t_list* lista_procesos_recibidos;
t_list* list_hilos_recibidos;
t_list* lista_particiones;
void* espacio_usuario;

pthread_mutex_t mutex_espacio_usuario;
pthread_mutex_t mutex_lst_procss_recibidos;
pthread_mutex_t mutex_lst_hilos_recibidos;
pthread_mutex_t mutex_particionID;
pthread_mutex_t mutex_lista_particiones;
pthread_mutex_t mutex_respuesta_memory_dump_fs;
pthread_mutex_t mutex_fd_memoria;

sem_t se_desconecto_cpu;
sem_t llego_respuesta_memory_dump_fs;

int particionID;
int baseSiguienteParticion;

void escuchar_kernel();
int server_escucha_kernel();
void enviar_handshake_memoria_filesystem();

#endif /* MEMORIA_H_ */
