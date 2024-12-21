#ifndef CPU_H_
#define CPU_H_

#include "cpu_gestor.h"
#include "incializar_cpu.h"
#include "cpu_kernel_dispatch.h"
#include "cpu_kernel_interrupt.h"
#include "cpu_memoria.h"
#include "finalizar_cpu.h"
#include "dic_operaciones.h"

t_log* cpu_log_debug;
t_log* cpu_log_obligatorio;
t_config* cpu_config;

char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* PUERTO_ESCUCHA_DISPATCH;
char* PUERTO_ESCUCHA_INTERRUPT;
char* LOG_LEVEL;

int fd_cpu_dispatch;
int fd_cpu_interrupt;
int fd_kernel_dispatch;
int fd_kernel_interrupt;
int fd_memoria;

sem_t sem_fetch;
sem_t sem_decode;
sem_t sem_execute;
sem_t sem_val_leido;
sem_t sem_val_escrito;
sem_t sem_rta_kernel;
sem_t termino_ciclo_instruccion;
sem_t llego_contexto;
sem_t llego_contexto_una_vez;
sem_t se_desconecto_kernel;
sem_t contexto_actualizado;

int interrupt_pid;
int interrupt_tid;
bool hubo_quantum;
int interrupt_verificador;
char* interrupt_motivo;
bool interrupt_flag;
bool hay_exit;
bool instancia_ocupada;
bool existe_hilo_a_joinear;
char* instruccion;
int VERIFICADOR;

uint32_t valor_leido;

char* rta_escritura;

char** op_autorizada;
char** instruccion_split;

t_contexto* un_contexto;

op_code tipo_desalojo;
nombre_instruccion_comando nombre_instruccion_enum;

pthread_mutex_t mutex_interruptFlag;
pthread_mutex_t mutex_manejo_contexto;
pthread_mutex_t mutex_interrupt_motivo;
pthread_mutex_t mutex_instruccion_split;
pthread_mutex_t mutex_tipo_desalojo;
pthread_mutex_t mutex_VERIFICADOR;
pthread_mutex_t mutex_existe_hilo_a_joinear;
pthread_mutex_t mutex_hubo_quantum;

tipo_exit TIPO_EXIT;

char* rta_peticion_contexto;
char* rta_actualizacion_contexto;

char* rta_mutex_create;
char* rta_mutex_lock;
char* rta_mutex_unlock;
char* rta_thread_create;
char* rta_process_create;
char* rta_thread_cancel;
char* rta_thread_join;

sem_t sem_rta_mutex_create;
sem_t sem_rta_mutex_lock;
sem_t sem_rta_mutex_unlock;
sem_t sem_rta_thread_create;
sem_t sem_rta_process_create;
sem_t sem_rta_thread_cancel;
sem_t sem_rta_thread_join;
sem_t sem_hilo_en_ejecucion_desalojado;

// FUNCIONES
void enviar_handshake_memoria();

#endif
