#include "../src/kernel.h"
#include "../src/kernel_cpu_interrupt.h"
#include "../src/inicializar_kernel.h"
#include "../src/finalizar_kernel.h"
#include "../src/planificador_largo_plazo.h"
#include "../src/servicios_kernel.h"
#include "../src/tcb.h"

int main(int argc, char* argv[]) {

    if (argc < 4) {
        error_show("NO SE ENVIO LA CANTIDAD CORRECTA DE ARGUMENTOS\n");
        return EXIT_FAILURE;
    }
    
    char* archivo_instrucciones = argv[1];
    int tamanio_proceso = atoi(argv[2]);
    char* archivo_config = argv[3];
    
    inicializar_kernel(archivo_config);

    signal(SIGINT, sighandler);
    
    // EL KERNEL SE CONECTA A CPU DISPATCH
    if ((fd_cpu_dispatch = crear_conexion(IP_CPU, PUERTO_CPU_DISPATCH)))
        log_info(kernel_log_debug, "CONEXION CON CPU DISPATCH");

    // EL KERNEL SE CONECTA A CPU INTERRUPT
    if ((fd_cpu_interrupt = crear_conexion(IP_CPU, PUERTO_CPU_INTERRUPT)))
        log_info(kernel_log_debug, "CONEXION CON CPU INTERRUPT");

    // ATENDER LOS MENSAJES DE CPU DISPATCH
    pthread_t hilo_cpu_dispatch;
    pthread_create(&hilo_cpu_dispatch, NULL, (void*) atender_kernel_cpu_dispatch, NULL);
    pthread_detach(hilo_cpu_dispatch);

    // ATENDER LOS MENSAJES DE CPU INTERRUPT
    pthread_t hilo_cpu_interrupt_quantum;
    pthread_create(&hilo_cpu_interrupt_quantum, NULL, (void*) gestionar_interrupt_quantum, NULL);
    pthread_detach(hilo_cpu_interrupt_quantum);

    enviar_handshake_memoria();
    enviar_handshake_cpu_dispatch();
    enviar_handshake_cpu_interrupt();

    t_args_creacion * proceso_args = malloc(sizeof(t_args_creacion));
    proceso_args->archivo_instrucciones = archivo_instrucciones;
    proceso_args->tamanio = tamanio_proceso;

    pthread_t hilo_crear_proceso;
    pthread_create(&hilo_crear_proceso, NULL, (void*) crear_proceso_inicial, (void*) proceso_args);
    pthread_join(hilo_crear_proceso, NULL);

    return EXIT_SUCCESS;
}

void enviar_handshake_cpu_dispatch(){
    t_paquete* un_paquete = crear_paquete(HANDSHAKE_KERNEL_CPU_DISPATCH);
    cargar_string_al_paquete(un_paquete, "MENSAJE INICIAL DE KERNEL A CPU DISPATCH");
    enviar_paquete(un_paquete, fd_cpu_dispatch);
    log_info(kernel_log_debug, "ENVIO MENSAJE A CPU DISPATCH");
    eliminar_paquete(un_paquete);
}

void enviar_handshake_cpu_interrupt(){
    t_paquete* un_paquete = crear_paquete(HANDSHAKE_KERNEL_CPU_INTERRUPT);
    cargar_string_al_paquete(un_paquete, "MENSAJE INICIAL DE KERNEL A CPU INTERRUPT");
    enviar_paquete(un_paquete, fd_cpu_interrupt);
    verif = verificador();
    log_info(kernel_log_debug, "ENVIO MENSAJE A CPU INTERRUPT");
    eliminar_paquete(un_paquete);
}

void enviar_handshake_memoria(){

	int fd_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);

    t_paquete* un_paquete = crear_paquete(HANDSHAKE_KERNEL_MEMORIA);
    cargar_string_al_paquete(un_paquete, "MENSAJE INICIAL DE KERNEL A MEMORIA");
    enviar_paquete(un_paquete, fd_memoria);
    log_info(kernel_log_debug, "ENVIO MENSAJE MEMORIA");
    eliminar_paquete(un_paquete);

	atender_kernel_memoria(fd_memoria);

	liberar_conexion(fd_memoria);
}
