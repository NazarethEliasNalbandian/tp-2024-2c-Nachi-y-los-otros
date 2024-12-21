#ifndef MEMORIA_KERNEL_H_
#define MEMORIA_KERNEL_H_

#include "m_gestor.h"

void atender_memoria_kernel(void* arg);
void atender_handshake_kernel_memoria(t_buffer* unBuffer, int socket);
void atender_creacion_proceso(t_buffer* unBuffer, int socket);
void atender_creacion_hilo(t_buffer* unBuffer, int socket);
void atender_finalizacion_hilo(t_buffer* unBuffer, int socket);
void atender_finalizacion_proceso(t_buffer* unBuffer, int socket);
void atender_dump_memory(t_buffer* unBuffer, int socket);
void atender_finalizacion_hilo(t_buffer* unBuffer, int socket);
void avisar_a_memoria_para_liberar_estructuras_proceso(int pid);
void avisar_a_memoria_para_dumpeo(int tid, int pid);

#endif /* MEMORIA_KERNEL_H_ */
