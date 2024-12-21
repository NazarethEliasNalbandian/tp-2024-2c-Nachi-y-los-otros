#ifndef C_GESTOR_H_
#define C_GESTOR_H_

#include "../../utils/src/shared.h"

typedef enum {
    THREAD,
    PROCESS,
    NADA
} tipo_exit;

extern t_log* cpu_log_debug;
extern t_log* cpu_log_obligatorio;
extern t_config* cpu_config;

extern char* IP_MEMORIA;
extern char* PUERTO_MEMORIA;
extern char* PUERTO_ESCUCHA_DISPATCH;
extern char* PUERTO_ESCUCHA_INTERRUPT;
extern char* LOG_LEVEL;

extern int fd_cpu_dispatch;
extern int fd_cpu_interrupt;
extern int fd_kernel_dispatch;
extern int fd_kernel_interrupt;
extern int fd_memoria;

extern sem_t sem_fetch;
extern sem_t sem_decode;
extern sem_t sem_execute;
extern sem_t sem_val_leido;
extern sem_t sem_val_escrito;
extern sem_t sem_sol_marco;
extern sem_t sem_rta_kernel;
extern sem_t termino_ciclo_instruccion;
extern sem_t llego_contexto;
extern sem_t llego_contexto_una_vez;
extern sem_t se_desconecto_kernel;
extern sem_t contexto_actualizado;

extern bool interrupt_flag;
extern bool hubo_quantum;
extern int interrupt_pid;
extern int interrupt_tid;
extern int interrupt_verificador;
extern char* interrupt_motivo;
extern bool hay_exit;
extern bool instancia_ocupada;
extern bool existe_hilo_a_joinear;
extern char* instruccion;
extern int VERIFICADOR;
extern uint32_t valor_leido;

extern char* rta_escritura;

extern char** op_autorizada;
extern char** instruccion_split;

extern t_contexto* un_contexto;

extern op_code tipo_desalojo;
extern nombre_instruccion_comando nombre_instruccion_enum;

extern pthread_mutex_t mutex_interruptFlag;
extern pthread_mutex_t mutex_manejo_contexto;
extern pthread_mutex_t mutex_interrupt_motivo;
extern pthread_mutex_t mutex_instruccion_split;
extern pthread_mutex_t mutex_tipo_desalojo;
extern pthread_mutex_t mutex_VERIFICADOR;
extern pthread_mutex_t mutex_existe_hilo_a_joinear;
extern pthread_mutex_t mutex_hubo_quantum;

extern tipo_exit TIPO_EXIT;

extern char* rta_peticion_contexto;
extern char* rta_actualizacion_contexto;

extern char* rta_mutex_create;
extern char* rta_mutex_lock;
extern char* rta_mutex_unlock;
extern char* rta_thread_create;
extern char* rta_process_create;
extern char* rta_thread_cancel;
extern char* rta_thread_join;

extern sem_t sem_rta_mutex_create;
extern sem_t sem_rta_mutex_lock;
extern sem_t sem_rta_mutex_unlock;
extern sem_t sem_rta_thread_create;
extern sem_t sem_rta_process_create;
extern sem_t sem_rta_thread_cancel;
extern sem_t sem_rta_thread_join;
extern sem_t sem_hilo_en_ejecucion_desalojado;

#endif 