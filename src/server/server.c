#include "schedulers.h"
#include "../common/sockets.h"
#include "../common/ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

#define SERVER_PORT 8080

/* Variable global para controlar el ciclo de vida del servidor */
volatile sig_atomic_t server_running = 1;

/*
 * Manejador de señales para capturar SIGINT (Ctrl+C).
 * Permite un cierre controlado del servidor, habilitando la generación
 * del resumen de métricas final.
 */
void handle_sigint(int sig) {
    (void)sig;
    server_running = 0;
    print_info("\nSeñal de apagado recibida. Iniciando secuencia de cierre...");
}

int main(int argc, char *argv[]) {
    /* Validación básica de argumentos */
    int algoritmo = 1; /* 1: FIFO, 2: SJF, 3: HPF, 4: RR */
    int quantum = 0;

    if (argc >= 2) {
        algoritmo = atoi(argv[1]);
        if (algoritmo == 4 && argc == 3) {
            quantum = atoi(argv[2]);
        }
    }

    /* Despliegue del banner dinámico según el algoritmo seleccionado */
    if (algoritmo == 1) {
        print_welcome_banner("FIFO");
    } else if (algoritmo == 2) {
        print_welcome_banner("SJF");
    } else if (algoritmo == 3) {
        print_welcome_banner("HPF");
    } else if (algoritmo == 4) {
        print_welcome_banner("RR");
    } else {
        /* Fallback de seguridad en caso de recibir un número inválido */
        print_welcome_banner("FIFO"); 
    }

    /* Configuración del manejador de señales para apagado seguro */
    signal(SIGINT, handle_sigint);

    /* 1. Inicialización de la estructura de datos compartida (Cola) */
    queue_t ready_queue;
    queue_init(&ready_queue);
    print_success("Cola de procesos (Ready Queue) inicializada.");

    /* 2. Configuración de la red */
    int server_fd = server_setup_socket(SERVER_PORT);
    if (server_fd == -1) {
        print_fatal_error("No se pudo inicializar el socket del servidor.");
        exit(EXIT_FAILURE);
    }
    
    char msg[100];
    snprintf(msg, sizeof(msg), "Servidor escuchando en el puerto %d.", SERVER_PORT);
    print_success(msg);

    /* 3. Preparación de argumentos para los hilos planificadores */
    scheduler_args_t args;
    args.server_socket = server_fd;
    args.ready_queue = &ready_queue;
    args.scheduling_algorithm = algoritmo;
    args.quantum = quantum;

    /* 4. Creación de los hilos del Sistema Operativo */
    pthread_t job_thread_id, cpu_thread_id;

    if (pthread_create(&job_thread_id, NULL, job_scheduler_thread, &args) != 0) {
        print_fatal_error("Fallo al crear el hilo del JOB Scheduler.");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&cpu_thread_id, NULL, cpu_scheduler_thread, &args) != 0) {
        print_fatal_error("Fallo al crear el hilo del CPU Scheduler.");
        exit(EXIT_FAILURE);
    }

    /* 5. Ciclo de espera del servidor principal */
    print_info("El simulador esta en ejecucion. Presione Ctrl+C para detener y ver metricas.");
    while (server_running) {
        sleep(1); /* El hilo principal duerme mientras los schedulers trabajan */
    }

    /* 6. Secuencia de apagado y volcado de logs */
    print_info("Apagando schedulers y volcando el log de resultados...");
    
    /* NOTA IMPORTANTE: 
     En este punto se debe implementar la lógica para cancelar los hilos,
     cerrar el socket (close(server_fd)), destruir la cola (queue_destroy)
     e imprimir las tablas de TAT y WT como indica la especificacion. */

    close(server_fd);
    queue_destroy(&ready_queue);
    print_success("Servidor apagado correctamente.");

    return 0;
}