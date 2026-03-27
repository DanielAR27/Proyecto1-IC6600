#include "schedulers.h"
#include "../common/ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

void *cpu_scheduler_thread(void *arg) {
    scheduler_args_t *args = (scheduler_args_t *)arg;
    queue_t *queue = args->ready_queue;
    
    print_info("CPU Scheduler inicializado.");

    while (1) {
        pcb_t current_process;

        /* Se intenta extraer un proceso de la cola.
         Si la cola está vacía, el hilo debe esperar (se puede optimizar con
         pthread_cond_wait para no hacer "busy waiting"). */
        if (queue_dequeue(queue, &current_process)) {
            char log_msg[150];
            
            /* Impresión del Context Switch */
            snprintf(log_msg, sizeof(log_msg), 
                     "Proceso con PID %d: Burst %d - Prioridad %d entra en ejecucion", 
                     current_process.pid, current_process.burst, current_process.priority);
            print_success(log_msg); 

            /* =========================================================
             * SECCION DE ALGORITMOS DE PLANIFICACION
             * ========================================================= */
            
            if (args->scheduling_algorithm == 1) {
                /* ALGORITMO FIFO (First In, First Out)
                Ejecuta el proceso en su totalidad sin interrupciones. */
                sleep(current_process.burst);
                current_process.burst_executed = current_process.burst;
                
                /* Impresión de finalización de proceso */
                snprintf(log_msg, sizeof(log_msg), "Proceso con PID %d termino completamente su ejecucion.", current_process.pid);
                print_info(log_msg);

            } else if (args->scheduling_algorithm == 4) {
                /* * ALGORITMO ROUND ROBIN (RR)
                 * TODO: Implementar lógica basada en args->quantum.
                 * Si el burst es mayor al quantum, ejecutar por el quantum,
                 * actualizar burst_executed, y volver a encolar (queue_enqueue).
                 */
            }
            /* TODO: Implementar SJF y HPF */

            /* ========================================================= */

        } else {
            /* * Si no hay procesos, el CPU se encuentra ocioso.
             * Se duerme brevemente para evitar saturar el procesador real.
             */
            usleep(500000); /* 0.5 segundos */
        }
    }

    return NULL;
}