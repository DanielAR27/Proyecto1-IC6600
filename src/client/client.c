#include "client.h"
#include "../common/sockets.h"
#include "../common/ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

/* Variable global para controlar la ejecución del bucle en el modo automático.
 Su modificación es segura frente a interrupciones asíncronas. */
volatile sig_atomic_t client_running = 1;

/* Manejador de señales para capturar SIGINT (Ctrl+C) en el cliente.
 Permite detener la generación automática de procesos de manera controlada. */
void handle_client_sigint(int sig) {
    (void)sig;
    client_running = 0;
    print_info("\nSeñal recibida. Deteniendo la generacion automatica de procesos...");
}

/* Función ejecutada por cada hilo cliente.
 Establece una conexión con el servidor, transmite el PCB, espera la
 asignación de un PID y finaliza su ejecución. */
void *process_thread(void *arg) {
    pcb_t *process = (pcb_t *)arg;

    /* 1. Conexión al servidor */
    int socket_fd = client_connect(DEFAULT_SERVER_IP, DEFAULT_SERVER_PORT);
    if (socket_fd == -1) {
        print_error("Cliente: No se pudo conectar al servidor.");
        free(process);
        pthread_exit(NULL);
    }

    /* 2. Envío de la estructura PCB al servidor */
    if (!send_pcb(socket_fd, process)) {
        print_error("Cliente: Fallo al enviar el proceso.");
        close(socket_fd);
        free(process);
        pthread_exit(NULL);
    }

    /* 3. Espera bloqueante de la confirmación (PID) */
    int assigned_pid;
    if (receive_pid(socket_fd, &assigned_pid)) {
        char msg[120];
        snprintf(msg, sizeof(msg), "Proceso despachado. PID asignado: %d (Burst: %d, Prioridad: %d)", 
                 assigned_pid, process->burst, process->priority);
        print_success(msg);
    } else {
        print_error("Cliente: No se recibio el PID del servidor.");
    }

    /* 4. Cierre de la conexión y liberación de memoria dinámica */
    close(socket_fd);
    free(process);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    print_welcome_banner("Cliente OS");

    /* Validación inicial de argumentos */
    if (argc < 2) {
        print_error("Uso Manual: ./bin/client manual <ruta_archivo>");
        print_error("Uso Automatico: ./bin/client auto <burst_min> <burst_max> <tasa_min> <tasa_max>");
        return EXIT_FAILURE;
    }

    /* Inicialización de la semilla para la generación de números pseudoaleatorios */
    srand(time(NULL));

    /* -- MODALIDAD MANUAL -- */
    if (strcmp(argv[1], "manual") == 0) {
        if (argc != 3) {
            print_error("Uso Manual: ./bin/client manual <ruta_archivo>");
            return EXIT_FAILURE;
        }

        FILE *file = fopen(argv[2], "r");
        if (!file) {
            print_fatal_error("No se pudo abrir el archivo de procesos.");
            return EXIT_FAILURE;
        }

        int burst, priority;
        print_info("Iniciando lectura de procesos en Modo Manual...");

        while (fscanf(file, "%d %d", &burst, &priority) == 2) {
            pcb_t *new_process = malloc(sizeof(pcb_t));
            if (!new_process) continue;

            new_process->burst = burst;
            new_process->priority = priority;

            pthread_t tid;
            if (pthread_create(&tid, NULL, process_thread, new_process) == 0) {
                pthread_detach(tid);
            } else {
                free(new_process);
            }

            int sleep_time = (rand() % 6) + 3;
            sleep(sleep_time);
        }
        fclose(file);
        print_success("Lectura manual finalizada. Esperando finalizacion de hilos...");
        sleep(2);
    } 
    /* -- MODALIDAD AUTOMÁTICA -- */
    else if (strcmp(argv[1], "auto") == 0) {
        if (argc != 6) {
            print_error("Uso Automatico: ./bin/client auto <burst_min> <burst_max> <tasa_min> <tasa_max>");
            return EXIT_FAILURE;
        }

        int burst_min = atoi(argv[2]);
        int burst_max = atoi(argv[3]);
        int rate_min = atoi(argv[4]);
        int rate_max = atoi(argv[5]);

        /* Se configura la captura de la señal para detener el ciclo infinito */
        signal(SIGINT, handle_client_sigint);

        char info_msg[150];
        snprintf(info_msg, sizeof(info_msg), 
                 "Modo Automatico activo. Burst [%d-%d], Tasa de creacion [%d-%d] seg.", 
                 burst_min, burst_max, rate_min, rate_max);
        print_info(info_msg);
        print_info("Presione Ctrl+C para detener la generacion.");

        /* El ciclo persiste creando procesos hasta que el usuario decida abortar
         mediante la señal del sistema (Ctrl+C). */
        while (client_running) {
            pcb_t *new_process = malloc(sizeof(pcb_t));
            if (!new_process) continue;

            /* Generación aleatoria de características  */
            new_process->burst = (rand() % (burst_max - burst_min + 1)) + burst_min;
            new_process->priority = (rand() % 10) + 1; /* Rango fijo de 1 a 10 */

            pthread_t tid;
            if (pthread_create(&tid, NULL, process_thread, new_process) == 0) {
                pthread_detach(tid);
            } else {
                free(new_process);
            }

            /* Retardo simulado para la tasa de creación. */
            int sleep_time = (rand() % (rate_max - rate_min + 1)) + rate_min;
            sleep(sleep_time);
        }
        
        print_success("Generacion automatica detenida. Esperando finalizacion de hilos...");
        sleep(2);
    } else {
        print_error("Modalidad no reconocida. Utilice 'manual' o 'auto'.");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}