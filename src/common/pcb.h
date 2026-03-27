#ifndef PCB_H
#define PCB_H

/* Estructura que representa el Bloque de Control de Proceso (PCB).
 Encapsula toda la información necesaria para que el planificador del
 sistema operativo administre y ejecute las tareas. */
typedef struct {
    int pid;              /* Identificador único asignado por el servidor. */
    int burst;            /* Tiempo total de ejecución requerido en el CPU. */
    int priority;         /* Nivel de prioridad asignado al proceso (1 a 10). */
    int burst_executed;   /* Tiempo de ejecución acumulado (requerido para el algoritmo Round Robin). */
    long time_arrival;    /* Marca de tiempo en la que el proceso ingresó a la cola de listos. */
    long time_completion; /* Marca de tiempo en la que el proceso finalizó su ejecución. */
    long time_waiting;    /* Tiempo total que el proceso permaneció en estado de espera. */
} pcb_t;

#endif 