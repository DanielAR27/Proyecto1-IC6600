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

        bool has_process = false;

        if (args->scheduling_algorithm == 1) {
            has_process = queue_dequeue(queue, &current_process);
        } else if (args->scheduling_algorithm == 2) {
            has_process = queue_dequeue_shortest(queue, &current_process);
        } else if (args->scheduling_algorithm == 3) {
            has_process = queue_dequeue_highest_priority(queue, &current_process);
        } else if (args->scheduling_algorithm == 4) {
            has_process = queue_dequeue(queue, &current_process);
        } else {
            has_process = queue_dequeue(queue, &current_process);
        }

        if (!has_process) {
            usleep(500000);
            metrics_add_idle(args->metrics, 1); /* 0.5s -> sumamos 1/2; usamos entero de segundos acumulando cada tick */
            continue;
        }

        char log_msg[180];
        snprintf(log_msg, sizeof(log_msg),
                 "Proceso PID %d: Burst %d - Prioridad %d entra en ejecucion",
                 current_process.pid, current_process.burst, current_process.priority);
        print_success(log_msg);

        int remaining = current_process.burst - current_process.burst_executed;
        if (remaining <= 0) {
            continue;
        }

        if (args->scheduling_algorithm == 1) {
            sleep(remaining);
            current_process.burst_executed = current_process.burst;
            current_process.time_completion = time(NULL);
            snprintf(log_msg, sizeof(log_msg),
                     "Proceso PID %d completo su ejecucion (FIFO).", current_process.pid);
            print_info(log_msg);
            metrics_record_completion(args->metrics, &current_process);

        } else if (args->scheduling_algorithm == 2) {
            sleep(remaining);
            current_process.burst_executed = current_process.burst;
            current_process.time_completion = time(NULL);
            snprintf(log_msg, sizeof(log_msg),
                     "Proceso PID %d completo su ejecucion (SJF).", current_process.pid);
            print_info(log_msg);
            metrics_record_completion(args->metrics, &current_process);

        } else if (args->scheduling_algorithm == 3) {
            sleep(remaining);
            current_process.burst_executed = current_process.burst;
            current_process.time_completion = time(NULL);
            snprintf(log_msg, sizeof(log_msg),
                     "Proceso PID %d completo su ejecucion (HPF).", current_process.pid);
            print_info(log_msg);
            metrics_record_completion(args->metrics, &current_process);

        } else if (args->scheduling_algorithm == 4) {
            int quantum = args->quantum > 0 ? args->quantum : remaining;
            int slice = (remaining < quantum) ? remaining : quantum;

            sleep(slice);
            current_process.burst_executed += slice;

            if (current_process.burst_executed >= current_process.burst) {
                current_process.time_completion = time(NULL);
                snprintf(log_msg, sizeof(log_msg),
                         "Proceso PID %d completo su ejecucion (RR).", current_process.pid);
                print_info(log_msg);
                metrics_record_completion(args->metrics, &current_process);
            } else {
                queue_enqueue(queue, current_process);
                int remaining_after = current_process.burst - current_process.burst_executed;
                snprintf(log_msg, sizeof(log_msg),
                         "Proceso PID %d reencolado (RR). Ejecutado %d/%d, restante %d.",
                         current_process.pid, current_process.burst_executed,
                         current_process.burst, remaining_after);
                print_info(log_msg);
            }
        } else {
            sleep(remaining);
            current_process.burst_executed = current_process.burst;
            current_process.time_completion = time(NULL);
            metrics_record_completion(args->metrics, &current_process);
        }
    }

    return NULL;
}

/* =========================================================
 * Implementacion de helpers de metricas
 * ========================================================= */

void metrics_init(metrics_t *m) {
    m->head = NULL;
    m->tail = NULL;
    m->idle_half_ticks = 0;
    m->total_completed = 0;
    pthread_mutex_init(&m->mutex, NULL);
}

void metrics_record_completion(metrics_t *m, pcb_t *pcb) {
    metrics_node_t *node = (metrics_node_t *)malloc(sizeof(metrics_node_t));
    if (!node) return;
    node->pcb = *pcb;
    node->next = NULL;

    pthread_mutex_lock(&m->mutex);
    if (m->tail == NULL) {
        m->head = node;
        m->tail = node;
    } else {
        m->tail->next = node;
        m->tail = node;
    }
    m->total_completed++;
    pthread_mutex_unlock(&m->mutex);
}

double metrics_get_idle(metrics_t *m) {
    pthread_mutex_lock(&m->mutex);
    long v = m->idle_half_ticks;
    pthread_mutex_unlock(&m->mutex);
    return v * 0.5; /* segundos */
}

void metrics_add_idle(metrics_t *m, long half_ticks) {
    pthread_mutex_lock(&m->mutex);
    m->idle_half_ticks += half_ticks;
    pthread_mutex_unlock(&m->mutex);
}

metrics_node_t *metrics_get_list(metrics_t *m) {
    pthread_mutex_lock(&m->mutex);
    metrics_node_t *head = m->head;
    pthread_mutex_unlock(&m->mutex);
    return head;
}