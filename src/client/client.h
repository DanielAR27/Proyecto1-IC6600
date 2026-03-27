#ifndef CLIENT_H
#define CLIENT_H

#include "../common/pcb.h"

/* Constantes de red predeterminadas para la conexión del cliente.
 Se centralizan en la cabecera para facilitar la configuración 
 sin necesidad de alterar la lógica del código fuente. */
#define DEFAULT_SERVER_IP "127.0.0.1"
#define DEFAULT_SERVER_PORT 8080

/* Función principal ejecutada por cada hilo (thread) del cliente.
 Se encarga de gestionar el ciclo de vida de un único proceso:
    1. Establecer conexión con el servidor.
    2. Transmitir la estructura de datos (PCB).
    3. Esperar de forma bloqueante la asignación del PID.
    4. Cerrar la conexión y liberar la memoria dinámica.
 
 Parámetros:
    arg - Puntero genérico (void*) que debe apuntar a una estructura pcb_t 
    previamente alojada en memoria dinámica.
 Retorno:
    NULL al finalizar la ejecución del hilo. */
void *process_thread(void *arg);

#endif