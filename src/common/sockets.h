#ifndef SOCKETS_H
#define SOCKETS_H

#include "pcb.h"
#include <stdbool.h>

/* Inicializa un socket pasivo para el servidor.
 El socket se asocia (bind) a un puerto específico y se configura
 para escuchar (listen) conexiones entrantes.
 Retorna el descriptor de archivo (file descriptor) del socket,
 o -1 en caso de error crítico. */
int server_setup_socket(int port);

/* Inicializa un socket activo para el cliente y establece
 una conexión con el servidor especificado por su IP y puerto.
 Retorna el descriptor de archivo del socket conectado,
 o -1 en caso de fallo en la conexión. */
int client_connect(const char *ip, int port);

/* Transmite una estructura PCB a través del socket proporcionado.
 Se garantiza que el bloque de memoria de la estructura se envíe en su totalidad.
 Retorna 'true' si la transmisión fue exitosa, o 'false' en caso contrario. */
bool send_pcb(int socket_fd, pcb_t *pcb);

/* Recibe una estructura PCB a través del socket proporcionado.
 El hilo se bloquea hasta que se reciban los bytes correspondientes.
 Retorna 'true' si la recepción fue exitosa, o 'false' si se perdió la conexión. */
bool receive_pcb(int socket_fd, pcb_t *pcb);

/* Transmite un número entero (PID) a través del socket.
 Utilizado por el servidor para confirmar la recepción de un proceso. */
bool send_pid(int socket_fd, int pid);

/* Recibe un número entero (PID) a través del socket.
 Utilizado por el cliente para obtener la confirmación del servidor. */
bool receive_pid(int socket_fd, int *pid);

#endif /* SOCKETS_H */