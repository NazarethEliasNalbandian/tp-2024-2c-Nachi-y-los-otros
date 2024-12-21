#include "../src/servicios_filesystem.h"

void retardo_acceso_bloque(){
	usleep(RETARDO_ACCESO_BLOQUE*1000);
}

void responder_OK(int file_descriptor){
	t_paquete* un_paquete = crear_paquete(MEMORY_DUMP_MEMORIA_FILESYSTEM);
	cargar_string_al_paquete(un_paquete, "OK");
	enviar_paquete(un_paquete, file_descriptor);
	eliminar_paquete(un_paquete);
}

void responder_ERROR(int file_descriptor){
	t_paquete* un_paquete = crear_paquete(MEMORY_DUMP_MEMORIA_FILESYSTEM);
	cargar_string_al_paquete(un_paquete, "ERROR");
	enviar_paquete(un_paquete, file_descriptor);
	eliminar_paquete(un_paquete);
}