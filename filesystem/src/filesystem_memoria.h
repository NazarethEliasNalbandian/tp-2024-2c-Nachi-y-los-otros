#ifndef FILESYSTEM_MEMORIA_H_
#define FILESYSTEM_MEMORIA_H_

#include "fs_gestor.h"

void atender_filesystem_memoria(void* arg);
void atender_handshake_memoria_filesystem(t_buffer* unBuffer, int socket);
void atender_memory_dump(t_buffer* unBuffer, int socket);

#endif /* FILESYSTEM_MEMORIA_H_ */
