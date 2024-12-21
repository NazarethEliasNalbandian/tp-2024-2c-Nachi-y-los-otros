#include "../src/mmu.h"
#include "../src/cpu_memoria.h"

int MMU(uint32_t dir_logica)
{
    int desplazamiento = dir_logica;
    int dir_fisica = 0;

    // log_trace(cpu_log_debug, "DIR LOG: %d Y LIMITE: %d", dir_logica, un_contexto->r_cpu->limite);
    if(un_contexto != NULL){
        if(desplazamiento < (un_contexto->r_cpu->limite - un_contexto->r_cpu->base)){

            dir_fisica = un_contexto->r_cpu->base + desplazamiento;

            // log_trace(cpu_log_debug, "DIR FIS %d", dir_fisica);

            return dir_fisica;
        }
        else{
            log_error(cpu_log_debug, "DESPLAZAMIENTO: %d \t LIMITE: %d", desplazamiento, un_contexto->r_cpu->limite);
            return -1;
        }
    } else
        return -1;

}