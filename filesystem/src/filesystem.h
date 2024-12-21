#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include "fs_gestor.h"

// Definición de variables globales
t_log* fs_log_debug;
t_log* fs_log_obligatorio;
t_config* fs_config;
int fd_filesystem;

char* PUERTO_ESCUCHA;
char* MOUNT_DIR;
int BLOCK_SIZE; 
int BLOCK_COUNT;
int RETARDO_ACCESO_BLOQUE;
char* LOG_LEVEL;

sem_t se_desconecto_memoria;

int fd_archivoBloques;
int fd_bitmap;

char* bitmapChar;
t_bitarray* bitmap;

void* bloquesEnMemoria;
t_list* lista_fcb;

pthread_mutex_t mutex_bloquesEnMemoria;
pthread_mutex_t mutex_bitmap;
pthread_mutex_t mutex_lista_fcb;

char* PATH_ARCHIVO_BLOQUES;
char* PATH_BITMAP;

// FUNCIONES
void escuchar_memoria();
int server_escucha_memoria();


#endif
