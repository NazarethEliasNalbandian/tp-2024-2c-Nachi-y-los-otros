#ifndef SERVICIOS_FILESYSTEM_H_
#define SERVICIOS_FILESYSTEM_H_

#include "fs_gestor.h"

void retardo_acceso_bloque();
void responder_OK(int file_descriptor);
void responder_ERROR(int file_descriptor);

#endif /* SERVICIOS_MEMORIA_H_ */