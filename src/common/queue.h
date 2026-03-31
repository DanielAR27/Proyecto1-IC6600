#ifndef QUEUE_H
#define QUEUE_H

#include "pcb.h"
#include <pthread.h>
#include <stdbool.h>

/* Nodo de la lista enlazada que contiene un Bloque de Control de Proceso (PCB). */
typedef struct node {
    pcb_t pcb;
    struct node *next;
} node_t;

/* Estructura que representa la cola del estado Ready.
 Implementa mecanismos de exclusión mutua para garantizar la seguridad
 en un entorno multihilo (Thread-Safe). */
typedef struct {
    node_t *head;             /* Puntero al primer elemento de la cola. */
    node_t *tail;             /* Puntero al último elemento de la cola. */
    int count;                /* Cantidad actual de procesos en la cola. */
    pthread_mutex_t mutex;    /* Candado para exclusión mutua. */
    pthread_cond_t cond;      /* Variable de condición para notificar cambios. */
} queue_t;

/* Inicializa la cola y sus mecanismos de sincronización. */
void queue_init(queue_t *q);

/* Inserta un proceso al final de la cola de manera segura. */
void queue_enqueue(queue_t *q, pcb_t process);

/* Extrae el primer proceso de la cola. 
 Retorna 'true' si la operación fue exitosa, o 'false' si la cola está vacía. */
bool queue_dequeue(queue_t *q, pcb_t *process);

/* Extrae el proceso con menor tiempo restante de ejecucion. */
bool queue_dequeue_shortest(queue_t *q, pcb_t *process);

/* Extrae el proceso con mayor prioridad (valor mas alto). */
bool queue_dequeue_highest_priority(queue_t *q, pcb_t *process);

/* Imprime en consola el estado actual de la cola Ready (solo lectura). */
void queue_print(queue_t *q);

/* Libera los recursos de sincronización asociados a la cola. */
void queue_destroy(queue_t *q);

#endif 