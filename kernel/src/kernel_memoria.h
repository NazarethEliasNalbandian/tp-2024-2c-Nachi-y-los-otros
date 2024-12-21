#ifndef KERNEL_MEMORIA_H_
#define KERNEL_MEMORIA_H_

#include "k_gestor.h"

void atender_kernel_memoria();
void enviar_y_recibir_peticion_creacion_proceso(int pid, int tamanio);
void enviar_y_recibir_peticion_creacion_hilo(int pid, int tid, char* archivo_instrucciones);
void avisar_a_memoria_para_liberar_estructuras_hilo(int tid, int pid);
void avisar_a_memoria_para_liberar_estructuras_proceso(int pid);
void avisar_a_memoria_para_dumpeo(int tid, int pid);

#endif /* KERNEL_MEMORIA_H_ */
