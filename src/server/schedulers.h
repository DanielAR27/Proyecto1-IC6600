#ifndef SCHEDULERS_H
#define SCHEDULERS_H

#include "../common/queue.h"

/* Estructura de argumentos para inicializar los hilos planificadores.
 Permite pasar múltiples parámetros (como el socket del servidor y la cola)
 a través de la interfaz de pthreads, la cual solo acepta un puntero void. */
typedef struct {
    int server_socket;     /* Descriptor del socket pasivo del servidor. */
    queue_t *ready_queue;  /* Puntero a la cola compartida de procesos listos. */
    int scheduling_algorithm; /* (Opcional por ahora) Tipo de algoritmo seleccionado. */
    int quantum;           /* (Opcional por ahora) Valor del quantum para Round Robin. */
} scheduler_args_t;

/* Función principal del hilo JOB Scheduler.
 Se encarga de aceptar conexiones entrantes, recibir los PCB enviados por 
 los clientes, asignarles un PID, enviar la confirmación y encolarlos. */
void *job_scheduler_thread(void *arg);

/* Función principal del hilo CPU Scheduler.
 Actúa como el despachador del procesador, encargándose de extraer procesos 
 de la cola de listos (Ready Queue) basándose en el algoritmo de planificación 
 seleccionado (FIFO, SJF, HPF o Round Robin). Simula la ejecución de cada 
 tarea mediante retardos de tiempo proporcionales a su ráfaga de CPU (burst), 
 gestiona los cambios de contexto y registra los tiempos de finalización 
 para el cálculo posterior de métricas de rendimiento. */
void *cpu_scheduler_thread(void *arg);

#endif 