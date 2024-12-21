#ifndef BITMAP_H_
#define BITMAP_H_

#include "fs_gestor.h"

void imprimir_bitarray(t_bitarray* bitarray); 
int encontrar_primer_bit_libre(t_bitarray* bitarray);
int cantidad_bloques_libres(t_bitarray* bitarray);
void actualizar_bitmap_en_memoria();
void setear_bloque_bits(t_bitarray *bitarray, int indice);
void cleanear_bloque_bits(t_bitarray *bitarray, int indice);

#endif