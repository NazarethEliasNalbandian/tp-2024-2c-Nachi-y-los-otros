#ifndef FCB_H_
#define FCB_H_

#include "fs_gestor.h"

void crear_fcb(char* nombre_archivo, size_t tamanio);
void crear_archivo_metadata(t_fcb* un_fcb);
bool existe_fcb(char* nombre_archivo);
t_fcb* obtener_fcb(char* nombre_archivo);
void setear_valor_entero_en_fcb(t_fcb* una_fcb, char* clave, int valor);
void destruir_listas_fcbs();

#endif