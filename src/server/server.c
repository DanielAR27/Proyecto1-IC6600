#include "schedulers.h"
#include "../common/sockets.h"
#include "../common/ui.h"
#include "../common/queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#define SERVER_PORT 8080

/* Variable global para controlar el ciclo de vida del servidor */
volatile sig_atomic_t server_running = 1;

typedef struct {
    queue_t *queue;
    volatile sig_atomic_t *running_flag;
} console_args_t;

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

/* Hilo de consola para consultar la cola en tiempo real. 
   Comando soportado: "cola" (muestra Ready Queue). "salir" finaliza la lectura. */
void *console_thread(void *arg) {
    console_args_t *cargs = (console_args_t *)arg;
    char buffer[64];

    print_info("Consola interactiva lista. Escriba 'cola' para ver la Ready Queue, 'salir' para terminar.");

    while (*(cargs->running_flag)) {
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            continue;
        }

        if (strncmp(buffer, "cola", 4) == 0) {
            queue_print(cargs->queue);
        } else if (strncmp(buffer, "salir", 5) == 0) {
            print_info("Cerrando consola interactiva...");
            break;
        }
    }
    return NULL;
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
    metrics_t metrics;
    queue_init(&ready_queue);
    metrics_init(&metrics);
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
    args.metrics = &metrics;

    /* 4. Creación de los hilos del Sistema Operativo */
    pthread_t job_thread_id, cpu_thread_id;
    pthread_t console_thread_id;
    int console_created = 0;

    if (pthread_create(&job_thread_id, NULL, job_scheduler_thread, &args) != 0) {
        print_fatal_error("Fallo al crear el hilo del JOB Scheduler.");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&cpu_thread_id, NULL, cpu_scheduler_thread, &args) != 0) {
        print_fatal_error("Fallo al crear el hilo del CPU Scheduler.");
        exit(EXIT_FAILURE);
    }

    /* 4.1 Hilo de consola interactiva */
    console_args_t console_args = { .queue = &ready_queue, .running_flag = &server_running };
    if (pthread_create(&console_thread_id, NULL, console_thread, &console_args) != 0) {
        print_error("No se pudo iniciar la consola interactiva. Continuando sin ella.");
    } else {
        console_created = 1;
    }

    /* 5. Ciclo de espera del servidor principal */
    print_info("El simulador esta en ejecucion. Presione Ctrl+C para detener y ver metricas.");
    while (server_running) {
        sleep(1); /* El hilo principal duerme mientras los schedulers trabajan */
    }

    /* 6. Secuencia de apagado y volcado de logs */
    print_info("Apagando schedulers y volcando el log de resultados...");
    
    /* Señal de parada al hilo de consola y espera de su finalización */
    server_running = 0;

    pthread_cancel(job_thread_id);
    pthread_cancel(cpu_thread_id);
    pthread_join(job_thread_id, NULL);
    pthread_join(cpu_thread_id, NULL);

    if (console_created) {
        pthread_cancel(console_thread_id);
        pthread_join(console_thread_id, NULL);
    }

    close(server_fd);
    queue_destroy(&ready_queue);
    /* Volcado de métricas */
    metrics_node_t *head = metrics_get_list(&metrics);
    int total = metrics.total_completed;
    double idle_seconds = metrics_get_idle(&metrics);

    printf("\n===== RESUMEN DE EJECUCION =====\n");
    printf("Procesos completados: %d\n", total);
    printf("Tiempo CPU ocioso (s): %.1f\n", idle_seconds);
    printf("\nPID | Burst | Llegada (Hora) |   Fin (Hora)   | TAT | WT\n");
    printf("-----------------------------------------------------------\n");

    /* Se encuentra el tiempo de llegada mas temprano (Segundo 0) */
    long base_time = 0;
    if (head != NULL) {
        base_time = head->pcb.time_arrival;
        metrics_node_t *temp_cur = head->next;
        while (temp_cur != NULL) {
            /* En caso de que se ordenaran distinto al finalizar (ej. SJF), 
               se busca el menor tiempo de llegada absoluto. Se obtiene el
               verdadero primer tiempo de llegada*/
            if (temp_cur->pcb.time_arrival < base_time) {
                base_time = temp_cur->pcb.time_arrival;
            }
            temp_cur = temp_cur->next;
        }
    }

    /* Imprimir la tabla usando tiempos relativos y liberar memoria */
    double sum_wt = 0.0;
    metrics_node_t *cur = head;
    while (cur != NULL) {
        /* Los cálculos de TAT y WT usan los tiempos reales para precisión */
        long tat = cur->pcb.time_completion - cur->pcb.time_arrival;
        long wt = tat - cur->pcb.burst;
        if (wt < 0) wt = 0;
        sum_wt += wt;
        
        /* Tiempos relativos para mostrar en pantalla */
        long rel_arrival = cur->pcb.time_arrival - base_time;
        long rel_completion = cur->pcb.time_completion - base_time;

        /* Toca convertir los tiempos "epoch" de C a formato reloj (HH:MM:SS) */
        char arr_str[15], comp_str[15];
        struct tm *tm_info;

        /* Se extrae y formatea la hora de llegada */
        tm_info = localtime(&cur->pcb.time_arrival);
        strftime(arr_str, sizeof(arr_str), "%H:%M:%S", tm_info);

        /* Se extrae y formatea la hora de finalización */
        tm_info = localtime(&cur->pcb.time_completion);
        strftime(comp_str, sizeof(comp_str), "%H:%M:%S", tm_info);

        /* Se imprime combinando tu idea: relativo (absoluto) */
        printf("%3d | %5d | %3ld (%s) | %3ld (%s) | %3ld | %3ld\n",
               cur->pcb.pid,
               cur->pcb.burst,
               rel_arrival, arr_str,
               rel_completion, comp_str,
               tat,
               wt);
               
        /* Liberación de memoria */
        metrics_node_t *temp = cur; 
        cur = cur->next;
        free(temp);
    }

    double avg_wt = (total > 0) ? (sum_wt / total) : 0.0;
    printf("\nWaiting Time promedio: %.2f s\n", avg_wt);
    printf("========================================\n\n");

    print_success("Servidor apagado correctamente.");

    return 0;
}