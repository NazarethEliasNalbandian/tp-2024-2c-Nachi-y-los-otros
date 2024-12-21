#ifndef SERVICIOS_KERNEL_H_
#define SERVICIOS_KERNEL_H_

#include "k_gestor.h"

int generar_verificador();
char* algoritmo_to_string(t_algoritmo algoritmo);
t_mochila* crear_mochila();
void destruir_mochila(t_mochila* mochila);
char* motivo_blocked_to_string(t_motivo_blocked motivo_blocked);
void agregar_hilo_a_lista_ready(t_tcb* un_tcb);
void enviar_mensaje_a_CPU(op_code codigo_op, char* mensaje);
void sighandler(int signal);
void mostrarListaEsperaIO();
void mostrarListaPCBMemoria();

#endif /* SERVICIOS_KERNEL_H_ */
