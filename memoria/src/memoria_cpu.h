#ifndef MEMORIA_CPU_H_
#define MEMORIA_CPU_H_

#include "m_gestor.h"

void atender_memoria_cpu();
void atender_handshake_cpu_memoria(t_buffer* unBuffer);
void atender_peticion_contexto(t_buffer* unBuffer);
void atender_actualizacion_contexto(t_buffer* unBuffer);
void atender_peticion_instruccion(t_buffer* unBuffer);
void atender_lectura(t_buffer* unBuffer);
void atender_escritura(t_buffer* unBuffer);
void enviar_finalizacion_filesystem();
void enviar_contexto(t_contexto* un_contexto);
void enviar_instruccion(char* instruccion);
void enviar_valor_por_lectura(uint32_t valor);

#endif /* MEMORIA_CPU_H_ */
