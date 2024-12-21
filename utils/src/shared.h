#ifndef SHARED_H_
#define SHARED_H_

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include "commons/log.h"
#include "commons/config.h"
#include "commons/string.h"
#include "commons/error.h"
#include "commons/process.h"
#include "commons/collections/list.h"
#include "commons/collections/queue.h"
#include "commons/temporal.h"
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <readline/readline.h>
#include "commons/bitarray.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <readline/history.h>
#include "commons/txt.h"

typedef enum{
    // CPU-MEMORIA
    PETICION_CONTEXTO_CPU_MEMORIA,
    PETICION_DE_INSTRUCCION_CPU_MEMORIA,
    LECTURA_CPU_MEMORIA,
    ESCRITURA_CPU_MEMORIA,
    ACTUALIZACION_CONTEXTO_CPU_MEMORIA,
    SOLICITAR_BASE_CPU_MEMORIA,

    // CPU-KERNEL
    ATENDER_INSTRUCCION_CPU,
    ATENDER_THREAD_EXIT,
    ATENDER_PROCESS_EXIT,
    ATENDER_SEGMENTATION_FAULT,
    FORZAR_DESALOJO_CPU_KERNEL,
    ATENDER_DESALOJO_HILO_CPU,
    EJECUTAR_HILO_KERNEL_CPU,
    ATENDER_RTA_KERNEL,
    ATENDER_RTA_MUTEX_CREATE,
    ATENDER_RTA_MUTEX_LOCK,
    ATENDER_RTA_MUTEX_UNLOCK,
    ATENDER_RTA_THREAD_CREATE,
    ATENDER_RTA_THREAD_CANCEL,
    ATENDER_RTA_THREAD_JOIN,
    ATENDER_RTA_PROCESS_CREATE,

    // KERNEL-MEMORIA
    CREACION_PROCESO_KERNEL_MEMORIA,
    CREACION_HILO_KERNEL_MEMORIA,
    FINALIZACION_HILO_KERNEL_MEMORIA,
    FINALIZACION_PROCESO_KERNEL_MEMORIA,
    DUMP_MEMORY_KERNEL_MEMORIA,

    // MEMORIA-FILESYSTEM
    MEMORY_DUMP_MEMORIA_FILESYSTEM,
    FINALIZACION_MEMORIA_FILESYSTEM,

    // HANSHAKES
    HANDSHAKE_MEMORIA_FILESYSTEM,
    HANDSHAKE_CPU_MEMORIA,
    HANDSHAKE_KERNEL_CPU_DISPATCH,
    HANDSHAKE_KERNEL_CPU_INTERRUPT,
    HANDSHAKE_KERNEL_MEMORIA
}op_code;

typedef enum {
    SET,
    READ_MEM,
    WRITE_MEM,
    SUM,
    SUB,
    JNZ,
    LOG,
    DUMP_MEMORY,
    IO,
    PROCESS_CREATE,
    THREAD_CREATE,
    THREAD_CANCEL, 
    THREAD_JOIN,
    MUTEX_CREATE,
    MUTEX_LOCK,
    MUTEX_UNLOCK,
    THREAD_EXIT,
    PROCESS_EXIT
} nombre_instruccion_comando;

typedef enum{
    AX,
    BX,
    CX,
    DX,
    EX,
    FX,
    GX,
    HX,
    PC
} t_registros;

typedef struct{
	int size;
	void* stream;
} t_buffer;

typedef struct{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

typedef struct {
	nombre_instruccion_comando instruccionAsociada;
	int cantidad_parametros_inicial;
	t_queue* parametros;
    pthread_mutex_t mutex_mochila;
} t_mochila;

typedef struct {
    int socket;
} t_args;

typedef struct {
    char* pseudo_c;
    char* fst_param;
    char* snd_param;
    char* thd_param;
} t_instruccion_codigo;

typedef struct{
	uint32_t PC;
	uint32_t AX;
	uint32_t BX;
	uint32_t CX;
	uint32_t DX;
    uint32_t EX;
	uint32_t FX;
	uint32_t GX;
	uint32_t HX;
    uint32_t base;
    uint32_t limite;
}t_registrosCPU;

typedef struct{
    int pid;
    int tid;
    int verificador;
    int maximo_PC;
    t_registrosCPU* r_cpu;
}t_contexto;

typedef enum {
	T_INT,
	T_STRING,
	T_SIZE_T,
	T_UINT32,
	T_CHAR
}tipo_dato_parametro;

// SOCKETS 
int crear_conexion(char *ip, char* puerto);
int iniciar_servidor(char * puerto, t_log* un_log, char * msj_server);
int esperar_cliente(int socket_servidor, t_log* un_log, char* msj);
void liberar_conexion(int socket_cliente);
int recibir_operacion(int socket_cliente);

// PROTOCOLO
t_paquete* crear_paquete(op_code code_op);
void crear_buffer(t_paquete* paquete);
void* serializar_paquete(t_paquete* paquete, int bytes);
void enviar_paquete(t_paquete*, int);
void eliminar_paquete(t_paquete* un_paquete);

// CARGAS
void cargar_generico_al_paquete(t_paquete* paquete, void* choclo, int size);
void cargar_int_al_paquete(t_paquete* paquete, int numero);
void cargar_string_al_paquete(t_paquete* paquete, char* string);
void cargar_uint8_al_paquete(t_paquete* un_paquete, uint8_t uint8_value);
void cargar_uint32_al_paquete(t_paquete* un_paquete, uint32_t uint32_value);
void cargar_size_t_al_paquete(t_paquete* un_paquete, size_t size_t_value);
void cargar_char_al_paquete(t_paquete* paquete, char caracter);
void cargar_registros_al_paquete(t_paquete * un_paquete, t_registrosCPU* registroRecibido);
void cargar_contexto_al_paquete(t_paquete * un_paquete, t_contexto* un_contexto);
void cargar_registros_al_contexto(t_contexto * un_contexto, t_registrosCPU* registroRecibido);

// EXTRACCIÓN
t_buffer* recibir_paquete(int conexion);
void* recibir_buffer(int* size, int socket_cliente);
int recibir_int_del_buffer(t_buffer* coso);
char* recibir_string_del_buffer(t_buffer* coso);
void* recibir_generico_del_buffer(t_buffer* un_buffer);
uint32_t recibir_uint32_del_buffer(t_buffer* un_buffer);
size_t recibir_size_t_del_buffer(t_buffer* un_buffer);
char recibir_char_del_buffer(t_buffer* un_buffer);
void recibir_contexto(t_buffer * unBuffer, t_contexto* contextoRecibido);
void recibir_registros(t_buffer* unBuffer, t_contexto* contextoRecibido);
void recibir_mochila(t_buffer *unBuffer, t_mochila* mochilaRecibida, t_log* logger);
void recibir_mensaje(int,t_log*);

// SERVICIOS
void safe_free(void* elemento);
void ejecutar_en_un_hilo_nuevo_detach(void (*f)(void*) ,void* struct_arg);
void ejecutar_en_un_hilo_nuevo_join(void (*f)(void*) ,void* struct_arg);
t_list* leer_archivo_y_cargar_instrucciones(const char* path_archivo, t_log* logger);
nombre_instruccion_comando convertirInstruccionAEnum(char* nombre_instruccion);
char* convertirEnumAInstruccionString(nombre_instruccion_comando nombre_instruccion_com);
t_registros convertirRegistroAEnum(char* registro);
const char* convertirCodOpAString(op_code codigo);
bool esDumpMemory(const char* str1);
bool esProcessExit(const char* str1);
bool esThreadExit(const char* str1);
bool esMutexUnlock(const char* str1);
bool esMutexLock(const char* str1);
bool esMutexCreate(const char* str1);
bool esThreadCancel(const char* str1);
bool esThreadJoin(const char* str1);
bool esThreadCreate(const char* str1);
bool esProcessCreate(const char* str1);
bool esIO(const char* str1);
bool esLOG(const char* str1);
bool esJNZ(const char* str1);
bool esSUB(const char* str1);
bool esSUM(const char* str1);
bool esWriteMem(const char* str1);
bool esReadMem(const char* str1);
bool esSet(const char* str1);

// DESTRUCCIÓN
void destruir_buffer(t_buffer* buffer);

#endif
