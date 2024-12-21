#ifndef INICIALIZAR_FILESYSTEM_H_
#define INICIALIZAR_FILESYSTEM_H_

#include "../src/fs_gestor.h"

void inicializar(char* archivo_config);

void inicializar_logs();
void inicializar_configs(char* archivo_config);
void iniciar_semaforos();
void inicializar_archivo_bloques();
void inicializar_archivo_bitmap();
void inicializar_pthreads();

#endif