#include "../src/memoria.h"
#include "../src/memoria_kernel.h"
#include "../src/finalizar_memoria.h"

int main(int argc, char* argv[]) {

	if (argc < 2) {
        error_show("NO SE ENVIO LA CANTIDAD CORRECTA DE ARGUMENTOS\n");
        return EXIT_FAILURE;
    }

	char* archivo_config = argv[1];
	inicializar_memoria(archivo_config);

	pthread_mutex_lock(&mutex_fd_memoria);
	fd_memoria = iniciar_servidor(PUERTO_ESCUCHA, memoria_log_debug, "MEMORIA INICIADA");

	log_info(memoria_log_debug, "ESPERANDO A CPU");
	fd_cpu = esperar_cliente(fd_memoria, memoria_log_debug, "CPU");
	pthread_mutex_unlock(&mutex_fd_memoria);

	// // ATENDER LOS MENSAJES DE CPU
	pthread_t hilo_cpu;
	pthread_create(&hilo_cpu, NULL, (void*) atender_memoria_cpu, NULL);
	pthread_detach(hilo_cpu);

	pthread_t hilo_escuchar_kernel;
	pthread_create(&hilo_escuchar_kernel, NULL, (void*) escuchar_kernel, NULL);
	pthread_detach(hilo_escuchar_kernel);

	enviar_handshake_memoria_filesystem();

	sem_wait(&se_desconecto_cpu);

	// finalizar_memoria();

	printf("MEMORIA SE FINALIZO CORRECTAMENTE...\n");
	return EXIT_SUCCESS;
}

void escuchar_kernel(){
    while(server_escucha_kernel());
}

int server_escucha_kernel(){
	if(memoria_log_debug != NULL)
		log_info(memoria_log_debug, "ESCUCHANDO KERNEL");

	pthread_mutex_lock(&mutex_fd_memoria);
	int fd_kernel = esperar_cliente(fd_memoria, memoria_log_debug, "KERNEL");
	pthread_mutex_unlock(&mutex_fd_memoria);
	
	if(memoria_log_obligatorio != NULL)
		log_info(memoria_log_obligatorio, "## Kernel Conectado - FD del socket: %d", fd_kernel);

	if(fd_kernel != -1){

        t_args * socket_args = malloc(sizeof(t_args));
        socket_args->socket = fd_kernel;

		pthread_t hilo_kernel;
		pthread_create(&hilo_kernel, NULL, (void*) atender_memoria_kernel, (void *) socket_args); 
		pthread_detach(hilo_kernel);

		return 1;
	}

	return EXIT_SUCCESS;
}

void enviar_handshake_memoria_filesystem(){
    int fd_filesystem = crear_conexion(IP_FILESYSTEM, PUERTO_FILESYSTEM);

    t_paquete* un_paquete = crear_paquete(HANDSHAKE_MEMORIA_FILESYSTEM);
    cargar_string_al_paquete(un_paquete, "MENSAJE INICIAL DE MEMORIA A FILESYSTEM");
    enviar_paquete(un_paquete, fd_filesystem);
    eliminar_paquete(un_paquete);

	log_info(memoria_log_debug, "ENVIO MENSAJE A FILESYSTEM");

    liberar_conexion(fd_filesystem);
}