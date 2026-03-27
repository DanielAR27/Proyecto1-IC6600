#include "ui.h"
#include "sockets.h"
#include <errno.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

/* Constante que define el límite de conexiones encoladas esperando ser aceptadas */
#define MAX_PENDING_CONNECTIONS 10

int server_setup_socket(int port) {
    int server_fd;
    struct sockaddr_in server_addr;

    /* Paso #1: Creación del Socket.
     Se solicita al Sistema Operativo un descriptor de archivo para un socket 
     de la familia IPv4 (AF_INET) orientado a conexión (SOCK_STREAM, es decir, TCP). */
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error critico: Fallo al crear el socket del servidor");
        return -1;
    }

    /* Configuración para reutilizar el puerto inmediatamente después de 
     cerrar el servidor, evitando el error "Address already in use". */
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("Advertencia: No se pudo configurar SO_REUSEADDR");
    }

    /* Preparación de la estructura de dirección */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; /* Escucha en cualquier interfaz de red local */
    server_addr.sin_port = htons(port);       /* Conversión del puerto al orden de bytes de la red */

    /* Paso 2: Asociación (Bind).
     Se enlaza el descriptor del socket con la dirección IP y el puerto especificados. */
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        char error_msg[256];
        /* strerror(errno) traduce el código de error del sistema a texto */
        snprintf(error_msg, sizeof(error_msg), "Fallo al asociar el puerto (bind): %s", strerror(errno));
        
        print_error(error_msg);
        close(server_fd);
        return -1;
    }

    /* Paso 3: Escucha (Listen).
     El socket se marca como pasivo, indicando que se utilizará para aceptar
     peticiones de conexión entrantes. */
    if (listen(server_fd, MAX_PENDING_CONNECTIONS) == -1) {
        perror("Error critico: Fallo al poner el socket en modo escucha");
        close(server_fd);
        return -1;
    }

    return server_fd;
}

int client_connect(const char *ip, int port) {
    int client_fd;
    struct sockaddr_in server_addr;

    /* Creación del socket TCP del cliente */
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error critico: Fallo al crear el socket del cliente");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    /* Conversión de la dirección IP de formato texto a formato binario de red */
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        perror("Error critico: Direccion IP invalida o no soportada");
        close(client_fd);
        return -1;
    }

    /* Paso de Conexión.
    El cliente intenta establecer una sesión TCP (Three-way handshake) con el servidor. */
    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error critico: Fallo al conectar con el servidor");
        close(client_fd);
        return -1;
    }

    return client_fd;
}

bool send_pcb(int socket_fd, pcb_t *pcb) {
    /* Se transmite el bloque de memoria cruda que conforma la estructura PCB.
     En arquitecturas homogéneas (ej. ambas máquinas son Linux x86_64), 
     enviar la estructura completa (sizeof) es seguro. */
    ssize_t bytes_sent = send(socket_fd, pcb, sizeof(pcb_t), 0);
    return (bytes_sent == sizeof(pcb_t));
}

bool receive_pcb(int socket_fd, pcb_t *pcb) {
    /* El hilo se bloquea en esta función (recv) hasta que los datos
     lleguen a través de la red, o la conexión se cierre. */
    ssize_t bytes_received = recv(socket_fd, pcb, sizeof(pcb_t), MSG_WAITALL);
    return (bytes_received == sizeof(pcb_t));
}

bool send_pid(int socket_fd, int pid) {
    ssize_t bytes_sent = send(socket_fd, &pid, sizeof(int), 0);
    return (bytes_sent == sizeof(int));
}

bool receive_pid(int socket_fd, int *pid) {
    ssize_t bytes_received = recv(socket_fd, pid, sizeof(int), MSG_WAITALL);
    return (bytes_received == sizeof(int));
}