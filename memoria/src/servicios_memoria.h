#ifndef SERVICIOS_MEMORIA_H_
#define SERVICIOS_MEMORIA_H_

#include "m_gestor.h"

void retardo_respuesta();
void logg_acceso_a_espacio_de_usuario(int pid, int tid, char* accion, int dir_fisica, size_t tamanio);
void responder_OK(int file_descriptor, op_code codigo_operacion);
void responder_ERROR(int file_descriptor, op_code codigo_operacion);
void loggear_instruccion_enviada(char** instruccion_split, int pid, int tid);

#endif /* SERVICIOS_MEMORIA_H_ */