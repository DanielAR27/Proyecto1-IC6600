#include "queue.h"
#include <stdlib.h>
#include <stdio.h>

/* Inicializa la estructura de la cola, estableciendo los punteros a nulo
 y configurando el mutex y la variable de condición para la sincronización. */
void queue_init(queue_t *q) {
    q->head = NULL;
    q->tail = NULL;
    q->count = 0;
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cond, NULL);
}

/* Añade un nuevo proceso (PCB) al final de la cola.
 Se utiliza un mutex para garantizar la exclusión mutua durante la inserción
 y evitar condiciones de carrera entre los schedulers. */
void queue_enqueue(queue_t *q, pcb_t process) {
    node_t *new_node = (node_t *)malloc(sizeof(node_t));
    if (new_node == NULL) {
        perror("Error critico: fallo al asignar memoria para el nodo");
        exit(EXIT_FAILURE);
    }
    
    new_node->pcb = process;
    new_node->next = NULL;

    /* Bloqueo del mutex antes de modificar la estructura compartida */
    pthread_mutex_lock(&q->mutex);

    /* Cola completamente vacía, se asigna cabeza y cola al nuevo nodo*/
    if (q->tail == NULL) {
        q->head = new_node;
        q->tail = new_node;
    } else {
        /*El último en fila ahora apunta al nodo más nuevo como su next*/
        q->tail->next = new_node;
        /*El último elemento de la cola será el nodo más nuevo*/
        q->tail = new_node;
    }
    q->count++;

    /* Se notifica al CPU Scheduler que un nuevo proceso está disponible */
    pthread_cond_signal(&q->cond);
    
    /* Liberación del mutex */
    pthread_mutex_unlock(&q->mutex);
}

/* Extrae el proceso ubicado en la cabeza de la cola.
 Retorna 'true' si se extrajo correctamente el PCB apuntado por 'process',
 o 'false' si la cola se encontraba vacía. */
bool queue_dequeue(queue_t *q, pcb_t *process) {
    /* Bloqueo del mutex antes de acceder a la estructura compartida */
    pthread_mutex_lock(&q->mutex);

    if (q->head == NULL) {
        /*Si no hay elementos en la cabeza, la cola está vacía*/
        pthread_mutex_unlock(&q->mutex);
        return false;
    }

    /* La cabeza apuntará ahora al siguiente elemento después del primero */
    node_t *temp = q->head;
    *process = temp->pcb;
    q->head = q->head->next;

    /* Si la cabeza ahora apunta a vacío es por que la cola se vació */
    if (q->head == NULL) {
        q->tail = NULL;
    }
    q->count--;

    /* Liberación del mutex */
    pthread_mutex_unlock(&q->mutex);
    
    /* Se libera la memoria del nodo extraído de forma segura */
    free(temp);

    return true;
}

bool queue_dequeue_shortest(queue_t *q, pcb_t *process) {
    pthread_mutex_lock(&q->mutex);

    if (q->head == NULL) {
        pthread_mutex_unlock(&q->mutex);
        return false;
    }

    node_t *best = q->head;
    node_t *best_prev = NULL;
    node_t *prev = NULL;

    for (node_t *cur = q->head; cur != NULL; cur = cur->next) {
        int remaining_best = best->pcb.burst - best->pcb.burst_executed;
        int remaining_cur = cur->pcb.burst - cur->pcb.burst_executed;
        if (remaining_cur < remaining_best) {
            best = cur;
            best_prev = prev;
        }
        prev = cur;
    }

    if (best_prev != NULL) {
        best_prev->next = best->next;
    } else {
        q->head = best->next;
    }

    if (best == q->tail) {
        q->tail = best_prev;
    }

    q->count--;
    *process = best->pcb;

    pthread_mutex_unlock(&q->mutex);
    free(best);
    return true;
}

bool queue_dequeue_highest_priority(queue_t *q, pcb_t *process) {
    pthread_mutex_lock(&q->mutex);

    if (q->head == NULL) {
        pthread_mutex_unlock(&q->mutex);
        return false;
    }

    node_t *best = q->head;
    node_t *best_prev = NULL;
    node_t *prev = NULL;

    for (node_t *cur = q->head; cur != NULL; cur = cur->next) {
        if (cur->pcb.priority > best->pcb.priority) {
            best = cur;
            best_prev = prev;
        }
        prev = cur;
    }

    if (best_prev != NULL) {
        best_prev->next = best->next;
    } else {
        q->head = best->next;
    }

    if (best == q->tail) {
        q->tail = best_prev;
    }

    q->count--;
    *process = best->pcb;

    pthread_mutex_unlock(&q->mutex);
    free(best);
    return true;
}

/* Destruye los mecanismos de sincronización de la cola.
 Se asume que la cola fue vaciada previamente antes de invocar esta función. */
void queue_destroy(queue_t *q) {
    pthread_mutex_destroy(&q->mutex);
    pthread_cond_destroy(&q->cond);
}

void queue_print(queue_t *q) {
    pthread_mutex_lock(&q->mutex);

    printf("\n-- Ready Queue (%d procesos) --\n", q->count);
    node_t *cur = q->head;
    while (cur != NULL) {
        int remaining = cur->pcb.burst - cur->pcb.burst_executed;
        if (remaining < 0) remaining = 0;
        printf("PID: %d | Burst: %d | Ejecutado: %d | Restante: %d | Prioridad: %d\n",
               cur->pcb.pid,
               cur->pcb.burst,
               cur->pcb.burst_executed,
               remaining,
               cur->pcb.priority);
        cur = cur->next;
    }
    if (q->count == 0) {
        printf("(cola vacia)\n");
    }
    printf("-----------------------------\n\n");

    pthread_mutex_unlock(&q->mutex);
}