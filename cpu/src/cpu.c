#include "../src/cpu.h"

int main(int argc, char* argv[]) {

    if (argc < 2) {
        error_show("NO SE ENVIO LA CANTIDAD CORRECTA DE ARGUMENTOS\n");
        return EXIT_FAILURE;
    }

    char* archivo_config = argv[1];

    inicializar_cpu(archivo_config);

    fd_cpu_dispatch = iniciar_servidor(PUERTO_ESCUCHA_DISPATCH, cpu_log_debug, "CPU DISPATCH");
    fd_cpu_interrupt = iniciar_servidor(PUERTO_ESCUCHA_INTERRUPT, cpu_log_debug, "CPU INTERRUPT");

    fd_memoria = crear_conexion(IP_MEMORIA,PUERTO_MEMORIA);
    log_info(cpu_log_debug,"MEMORIA");

    fd_kernel_dispatch = esperar_cliente(fd_cpu_dispatch, cpu_log_debug, "KERNEL DISPATCH");
    fd_kernel_interrupt = esperar_cliente(fd_cpu_interrupt, cpu_log_debug, "KERNEL INTERRUPT");

    pthread_t hilo_kernel_dispatch, hilo_kernel_interrupt, hilo_memoria;
    pthread_create(&hilo_kernel_dispatch,NULL,(void*)atender_cpu_kernel_dispatch,NULL);
    pthread_detach(hilo_kernel_dispatch);
    
    pthread_create(&hilo_kernel_interrupt,NULL,(void*)atender_cpu_kernel_interrupt,NULL);
    pthread_detach(hilo_kernel_interrupt);

    pthread_create(&hilo_memoria,NULL,(void*)atender_cpu_memoria,NULL);
    pthread_detach(hilo_memoria);

    enviar_handshake_memoria();

    sem_wait(&se_desconecto_kernel);
    sem_wait(&se_desconecto_kernel);

    // finalizar_cpu();
    printf("CPU SE FINALIZO CORRECTAMENTE...\n");

    return EXIT_SUCCESS;
}

void enviar_handshake_memoria(){
    t_paquete* un_paquete = crear_paquete(HANDSHAKE_CPU_MEMORIA);
    cargar_string_al_paquete(un_paquete, "MENSAJE INICIAL DE CPU A MEMORIA");
    enviar_paquete(un_paquete, fd_memoria);
    log_info(cpu_log_debug, "ENVIO MENSAJE A MEMORIA");
    eliminar_paquete(un_paquete);
}