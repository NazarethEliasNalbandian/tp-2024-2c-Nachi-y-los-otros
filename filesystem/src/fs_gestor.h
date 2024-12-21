#ifndef FS_GESTOR_H_
#define FS_GESTOR_H_

#include "../../utils/src/shared.h"  

typedef struct{
	char* nombre_archivo;
	size_t tamanio;
    int tamanio_en_bloques;
	t_list* indice_bloques_de_datos;
	int indice_bloque_indice;
	t_config * archivo_metadata;
	pthread_mutex_t mutex_fcb;
}t_fcb;

// Declaraciones de variables globales
extern t_log* fs_log_debug;
extern t_log* fs_log_obligatorio;
extern t_config* fs_config;
extern int fd_memoria;

extern char* PUERTO_ESCUCHA;
extern char* MOUNT_DIR;
extern int BLOCK_SIZE; 
extern int BLOCK_COUNT;
extern int RETARDO_ACCESO_BLOQUE;
extern char* LOG_LEVEL;

extern sem_t se_desconecto_memoria;

extern int fd_archivoBloques;
extern int fd_bitmap;

extern char* bitmapChar;
extern t_bitarray* bitmap;
extern t_list* lista_fcb;

extern void* bloquesEnMemoria;

extern pthread_mutex_t mutex_bloquesEnMemoria;
extern pthread_mutex_t mutex_bitmap;
extern pthread_mutex_t mutex_lista_fcb;

extern char* PATH_ARCHIVO_BLOQUES;
extern char* PATH_BITMAP;

#endif 

