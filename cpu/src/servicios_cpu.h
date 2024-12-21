#ifndef SERVICIOS_CPU_H_
#define SERVICIOS_CPU_H_

#include "cpu_gestor.h"

void destruir_contexto();
void enviar_hilo_a_kernel(int pid, int tid, int verificador, op_code tipo_desalojo, tipo_exit tipo_de_exit);
bool validador_de_header(char* header_string);
void asignarRegistro(t_registros registro, uint32_t** valor_a_guardar);
int obtenerIntDeRegistro(t_registros registro, uint32_t** r);
void escribir_registro(t_registros registro_a_escribir,int valor, uint32_t* registroAEscribir);
void actualizar_contexto();
void cargar_hilo_a_paquete(t_paquete* un_paquete, int pid, int tid, int verificador);
void atender_SEGMENTATION_FAULT();
bool hay_bloqueo(nombre_instruccion_comando nombre_inst);
bool hay_syscall(nombre_instruccion_comando nombre_inst);

#endif