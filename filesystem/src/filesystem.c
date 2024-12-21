#include "../src/filesystem.h"
#include "../src/filesystem_memoria.h"
#include "../src/inicializar_filesystem.h"
#include "../src/finalizar_filesystem.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        error_show("NO SE ENVIO LA CANTIDAD CORRECTA DE ARGUMENTOS\n");
        return EXIT_FAILURE;
    }

    char* archivo_config = argv[1];

    inicializar(archivo_config);

    fd_filesystem = iniciar_servidor(PUERTO_ESCUCHA, fs_log_debug, "FILESYSTEM");

    pthread_t hilo_escuchar_memoria;
    pthread_create(&hilo_escuchar_memoria, NULL, (void*) escuchar_memoria, NULL);
    pthread_detach(hilo_escuchar_memoria);

    sem_wait(&se_desconecto_memoria);

    // finalizar_filesystem();
    printf("FILESYSTEM SE FINALIZO CORRECTAMENTE...\n");
    return EXIT_SUCCESS;
}

void escuchar_memoria(){
    while(server_escucha_memoria());
}

int server_escucha_memoria(){
	printf("ESPERANDO MEMORIA\n");
	int fd_memoria = esperar_cliente(fd_filesystem, fs_log_debug, "MEMORIA");

    if(fd_memoria != -1){

        t_args * socket_args = malloc(sizeof(t_args));
        socket_args->socket = fd_memoria;

        pthread_t hilo_entradasalida;
        pthread_create(&hilo_entradasalida, NULL, (void*) atender_filesystem_memoria, (void*) socket_args); 
        pthread_detach(hilo_entradasalida);

        return 1;
    }
	
	return EXIT_SUCCESS;
}
