#include "schedulers.h"
#include "../common/sockets.h"
#include "../common/ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

void *job_scheduler_thread(void *arg) {
    /* Se extraen los argumentos proporcionados al hilo */
    scheduler_args_t *args = (scheduler_args_t *)arg;
    int server_fd = args->server_socket;
    queue_t *queue = args->ready_queue;

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    /* Variable estática local para la asignación de PIDs.
     Al existir un único hilo de JOB Scheduler, no se requiere 
     un mutex adicional para proteger este contador. */
    static int next_pid = 1;

    print_info("JOB Scheduler inicializado y en espera de procesos...");

    /* Bucle infinito del planificador de trabajos */
    while (1) {
        /* 1. Aceptar la conexión (Llamada bloqueante).
         El hilo se detendrá aquí hasta que un cliente intente conectarse. */
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd == -1) {
            print_error("JOB Scheduler: Fallo al aceptar la conexion entrante.");
            continue; /* Se ignora el error y se espera la siguiente conexion */
        }

        pcb_t new_process;

        /* 2. Recibir la estructura del proceso desde el cliente. */
        if (receive_pcb(client_fd, &new_process)) {
            
            /* 3. Asignación del PID y registro del tiempo de llegada */
            new_process.pid = next_pid++;
            new_process.time_arrival = time(NULL);
            new_process.burst_executed = 0;
            new_process.time_waiting = 0;

            /* 4. Enviar mensaje de confirmación (PID) al cliente. */
            if (send_pid(client_fd, new_process.pid)) {
                char msg_buffer[100];
                snprintf(msg_buffer, sizeof(msg_buffer), 
                         "Proceso recibido y confirmado. PID Asignado: %d (Burst: %d, Prioridad: %d)", 
                         new_process.pid, new_process.burst, new_process.priority);
                print_success(msg_buffer);
                
                /* 5. "Crear" su PCB definitivo y guardarlo en la lista de espera.
                 La función queue_enqueue ya maneja la exclusión mutua (mutex). */
                queue_enqueue(queue, new_process);
            } else {
                print_error("JOB Scheduler: Fallo al enviar la confirmacion del PID.");
            }
        } else {
            print_error("JOB Scheduler: Fallo al recibir el PCB del cliente.");
        }

        /* 6. Cierre de la conexión.
         El socket se cierra por el lado del servidor, asumiendo que el cliente
         hará lo mismo tras recibir su PID. */
        close(client_fd);
    }

    /* El hilo nunca debería alcanzar este punto bajo ejecución normal */
    return NULL;
}