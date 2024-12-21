#ifndef BLOQUES_H_
#define BLOQUES_H_

#include "fs_gestor.h"

int tamanio_en_bloques(int tamanio_en_bytes);
void escribir_data_en_bloque(int indice, void* valor, size_t tamanio);
void* leer_data_de_bloque(int indice, size_t tamanio);
void asignar_bloques_de_datos(t_fcb* un_fcb);
void asignar_bloque_de_indice(t_fcb* un_fcb);
void escribir_bloque_de_indice(t_fcb* un_fcb);
void escribir_bloques_de_datos(t_fcb* un_fcb, void* mensaje_memoria, size_t tamanio_mensaje);

#endif