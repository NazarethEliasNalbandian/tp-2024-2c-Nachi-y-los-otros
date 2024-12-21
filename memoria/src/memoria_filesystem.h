#ifndef MEMORIA_FILESYSTEM_H_
#define MEMORIA_FILESYSTEM_H_

#include "m_gestor.h"

void atender_memoria_filesystem(int fd_filesystem);
void enviar_memory_dump_filesystem(int pid, int tid, void* mensaje, size_t tamanio_particion, size_t tamanio);

#endif
