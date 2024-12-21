#ifndef K_GESTOR_H_
#define K_GESTOR_H_

#include "../../utils/src/shared.h"

typedef enum{
   FIFO,
   PRIORIDADES,
   CMN
}t_algoritmo;

typedef struct {
   char* archivo_instrucciones;
   int tamanio;
} t_args_creacion;


typedef enum{
   NEW_PCB,
   EN_MEMORIA_PCB,
   EXIT_PCB
}estado_pcb;

typedef enum{
   READY,
   EXEC,
   BLOCKED,
   EXIT
}estado_tcb;

typedef enum{
   PTHREAD_JOIN,
   MUTEX,
   _IO,
   _DUMP_MEMORY
}t_motivo_blocked;

typedef struct{
   int pid;
   t_list* lista_id_hilo;
   int siguiente_tid;
   t_list* lista_mutex;
   int tamanio;
   estado_pcb estado;
   char* archivo_instrucciones;
   int prioridad_tid0;
   bool flag_proceso_finalizado;
   pthread_mutex_t mutex_pcb;
}t_pcb;

typedef struct{
   int tid;
   int pid_asociado;
}t_hilo_id;

typedef struct {
   t_hilo_id* id_hilo;
   int prioridad;
   int verificador;
   estado_tcb estado;
   char* archivo_instrucciones;
   t_list* tids_en_espera_por_join;
   t_list* lista_mutex_asignados;
   bool flag_cancelar_quantum;
   pthread_mutex_t mutex_tcb;
   float temp;
   int inst;
}t_tcb;

typedef struct {
   int tiempo_sleep;
   t_tcb* tcb;
} t_sleep;

typedef struct{
   char* nombre_recurso;
   t_list* lista_bloqueados;   
   t_hilo_id* id_hilo_asignado;  
   pthread_mutex_t mutex_bloqueados;
   pthread_mutex_t mutex_id_hilo_asignado;
   pthread_mutex_t mutex_nombre_recurso;
}t_mutex_pcb;

typedef struct{
   t_list* lista_ready;
   pthread_mutex_t mutex_lista_ready;
}t_lista_ready_y_mutex;

typedef struct {
   int prioridad_lista;
   t_lista_ready_y_mutex* lista_ready_y_mutex;
   pthread_mutex_t mutex_prioridad;
}t_cola_multinivel;

// VARIABLES GLOBALES
extern t_log* kernel_log_debug;
extern t_log* kernel_log_obligatorio;
extern t_log* kernel_log_largo_plazo;
extern t_log* kernel_log_listas_pcb;
extern t_log* kernel_log_listas_tcb;
extern t_log* kernel_log_listas_io;
extern t_config* kernel_config;

extern bool verif;

extern int fd_kernel;
extern int fd_cpu_dispatch;
extern int fd_cpu_interrupt;

extern char* IP_MEMORIA;
extern char* PUERTO_MEMORIA;
extern char* IP_CPU;
extern char* verf;
extern char* PUERTO_CPU_DISPATCH;
extern char* PUERTO_CPU_INTERRUPT;
extern int inst;
extern char* algoritmo_planificacion;
extern t_algoritmo ALGORITMO_PLANIFICACION;
extern float QUANTUM;
extern char* LOG_LEVEL;

// ------ Listas ------
extern t_list* lista_pcb_new;
extern t_list* lista_pcb_enMemoria;
extern t_list* lista_pcb_exit;

extern t_list* lista_tcb_ready;
extern t_list* lista_tcb_exec;
extern t_list* lista_tcb_blocked;
extern t_list* lista_tcb_exit;

extern t_list* lista_cola_multinivel;

extern t_list* lista_en_espera_de_io;

// ------ PTHREAD_MUTEX ------
extern pthread_mutex_t mutex_lista_pcb_new;
extern pthread_mutex_t mutex_lista_pcb_enMemoria;
extern pthread_mutex_t mutex_lista_pcb_exit;

extern pthread_mutex_t mutex_lista_tcb_ready;
extern pthread_mutex_t mutex_lista_tcb_exec;
extern pthread_mutex_t mutex_lista_tcb_blocked;
extern pthread_mutex_t mutex_lista_tcb_exit;

extern pthread_mutex_t mutex_process_id;
extern pthread_mutex_t mutex_verificador;

extern pthread_mutex_t mutex_lista_cola_multinivel;

extern pthread_mutex_t mutex_lista_en_espera_de_io;

extern pthread_mutex_t mutex_rta_creacion_proceso;
extern pthread_mutex_t mutex_rta_creacion_hilo;
extern pthread_mutex_t mutex_rta_finalizacion_hilo;
extern pthread_mutex_t mutex_rta_finalizacion_proceso;
extern pthread_mutex_t mutex_rta_dumpeo;

extern int process_id;
extern int var_verificador;

extern char* rta_creacion_proceso;
extern char* rta_creacion_hilo;
extern char* rta_finalizacion_hilo;
extern char* rta_finalizacion_proceso;
extern char* rta_dumpeo;

// ------ SEMÁFOROS -------
extern sem_t llego_respuesta_creacion_proceso;
extern sem_t llego_respuesta_creacion_hilo;
extern sem_t llego_respuesta_finalizacion_hilo;
extern sem_t llego_respuesta_finalizacion_proceso;
extern sem_t llego_respuesta_dumpeo;
extern sem_t enviar_interrupcion_quantum;
extern sem_t finalizo_proceso_inicial;
extern sem_t hilo_desalojado;

#endif /* K_GESTOR_H_ */



