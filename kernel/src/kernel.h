#ifndef KERNEL_H_
#define KERNEL_H_

#include "k_gestor.h"
#include "inicializar_kernel.h"
#include "kernel_memoria.h"
#include "kernel_cpu_dispatch.h"
#include "kernel_cpu_interrupt.h"

// VARIABLES GLOBALES
t_log* kernel_log_debug;
t_log* kernel_log_obligatorio;
t_log* kernel_log_largo_plazo;
t_log* kernel_log_listas_pcb;
t_log* kernel_log_listas_tcb;
t_log* kernel_log_listas_io;
t_config* kernel_config;

int fd_kernel;
int fd_cpu_dispatch;
int fd_cpu_interrupt;

char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* IP_CPU;
char* verf;
char* PUERTO_CPU_DISPATCH;
char* PUERTO_CPU_INTERRUPT;
char* algoritmo_planificacion;
t_algoritmo ALGORITMO_PLANIFICACION;
float QUANTUM;
char* LOG_LEVEL;


// ------ Listas ------
t_list* lista_pcb_new;
t_list* lista_pcb_enMemoria;
t_list* lista_pcb_exit;

t_list* lista_tcb_ready;
t_list* lista_tcb_exec;
t_list* lista_tcb_blocked;
t_list* lista_tcb_exit;

t_list* lista_cola_multinivel;

t_list* lista_en_espera_de_io;

// ------ PTHREAD_MUTEX ------
pthread_mutex_t mutex_lista_pcb_new;
pthread_mutex_t mutex_lista_pcb_enMemoria;
pthread_mutex_t mutex_lista_pcb_exit;

pthread_mutex_t mutex_lista_tcb_ready;
pthread_mutex_t mutex_lista_tcb_exec;
pthread_mutex_t mutex_lista_tcb_blocked;
pthread_mutex_t mutex_lista_tcb_exit;

pthread_mutex_t mutex_process_id;
pthread_mutex_t mutex_verificador;

pthread_mutex_t mutex_lista_cola_multinivel;

pthread_mutex_t mutex_lista_en_espera_de_io;

pthread_mutex_t mutex_rta_creacion_proceso;
pthread_mutex_t mutex_rta_creacion_hilo;
pthread_mutex_t mutex_rta_finalizacion_hilo;
pthread_mutex_t mutex_rta_finalizacion_proceso;
pthread_mutex_t mutex_rta_dumpeo;

int process_id;
int var_verificador;

char* rta_creacion_proceso;
int inst;
char* rta_creacion_hilo;
char* rta_finalizacion_hilo;
bool verif;
char* rta_finalizacion_proceso;
char* rta_dumpeo;

sem_t llego_respuesta_creacion_proceso;
sem_t llego_respuesta_creacion_hilo;
sem_t llego_respuesta_finalizacion_hilo;
sem_t llego_respuesta_finalizacion_proceso;
sem_t llego_respuesta_dumpeo;
sem_t enviar_interrupcion_quantum;
sem_t finalizo_proceso_inicial;
sem_t hilo_desalojado;

// FUNCIONES
void enviar_handshake_cpu_dispatch();
void enviar_handshake_cpu_interrupt();
void enviar_handshake_memoria();

#endif /* KERNEL_H_ */



