# Simulacion de Planificacion de Procesos de un Sistema Operativo

**Curso:** Sistemas Operativos (Escuela de Computacion)  
**Institucion:** Instituto Tecnologico de Costa Rica (ITCR)  
**Profesora:** Ing. Erika Marin Schumann  
**Semestre:** I Semestre 2026  

## Integrantes

| Nombre              | Carnet      |
|---------------------|-------------|
| Daniel Aleman Ruiz  | 2023051957  |

---

## Descripcion General

Este proyecto implementa una simulacion de un planificador de CPU bajo la arquitectura cliente-servidor. El cliente genera procesos con sus respectivas caracteristicas (burst y prioridad) y los envia al servidor mediante sockets TCP. El servidor los administra a traves de dos hilos principales: el JOB Scheduler, encargado de recibir y encolar los procesos, y el CPU Scheduler, encargado de ejecutarlos segun el algoritmo de planificacion seleccionado.

Los algoritmos de planificacion soportados son:

- FIFO (First In, First Out)
- SJF (Shortest Job First)
- HPF (Highest Priority First)
- Round Robin (con quantum definido por el usuario)

---

## Estructura del Proyecto
```
/Proyecto1-IC6600
├── Makefile     # Esencial para compilar el cliente y el servidor.
├── README.md    # Instrucciones de compilación y ejecución
├── /src         # Todo el código fuente se encuentra en esta carpeta
│   ├── /client        
│   │   ├── client.c        # Lógica principal del cliente (modos manual y automático)
│   │   └── client.h        # Cabeceras del cliente
│   ├── /server
│   │   ├── server.c        # Inicialización del servidor y selección del algoritmo
│   │   ├── server.h        # Cabeceras del servidor
│   │   ├── job_scheduler.c # Hilo que recibe mensajes del socket, asigna PID y encola
│   │   ├── cpu_scheduler.c # Hilo que despacha procesos según FIFO, SJF, HPF o RR
│   │   └── schedulers.h    # Cabeceras para los schedulers.
│   └── /common     # Código que usarán tanto el cliente como el servidor
│       ├── pcb.h            # Definición del Process Control Block (PCB)
│       ├── queue.c         # Implementación de la lista/cola para el estado Ready
│       ├── queue.h         # Cabeceras para la cola del Ready.
│       ├── sockets.c       # Lógica de comunicación por sockets
│       ├── sockets.h       # Cabeceras para los sockets.
│       ├── ui.c            # Herramientas y/o útiles para la Interfaz de Usuario
│       └── ui.h            # Cabeceras para UI.
├── /data         # Archivos de entrada.
│   └── procesos.txt        # Archivo de texto de prueba para el modo manual del cliente
├── /bin         # Ejecutables principales del proyecto.
└── /docs        # Documentación del proyecto.
    └── Documentacion.pdf
```

---

## Requisitos del Sistema

- Sistema operativo: Linux
- Compilador: GCC con soporte para C99 o superior
- Libreria: PThreads (incluida en la mayoria de distribuciones Linux)

---

## Compilacion

Desde la raiz del proyecto, ejecutar:
```bash
make
```

Esto compilara tanto el cliente como el servidor y depositara los ejecutables en el directorio `/bin`.

Para limpiar los archivos compilados:
```bash
make clean
```

En caso de ser necesario, para desocupar el puerto `8080` basta con ejecutar el siguiente comando

```bash
fuser -k 8080/tcp
```

---

## Ejecucion

### Servidor

El servidor debe iniciarse antes que el cliente. Recibe como primer argumento el numero del algoritmo de planificacion:

| Numero | Algoritmo     |
|--------|---------------|
| 1      | FIFO          |
| 2      | SJF           |
| 3      | HPF           |
| 4      | Round Robin   |
```bash
# Iniciar con FIFO
./bin/server 1

# Iniciar con SJF
./bin/server 2

# Iniciar con HPF
./bin/server 3

# Iniciar con Round Robin (quantum = 3 unidades de tiempo)
./bin/server 4 3
```

Para detener el servidor y obtener el resumen de metricas, presionar `Ctrl+C`.

### Cliente - Modo Manual

Lee los procesos desde un archivo de texto. El archivo debe contener una linea por proceso con el formato `BURST PRIORIDAD`.
```bash
./bin/client manual data/procesos.txt
```

Formato del archivo de entrada (`procesos.txt`):
```
8 3
7 2
5 7
```

Entre la lectura de cada proceso se aplica un retardo aleatorio de 3 a 8 segundos.

### Cliente - Modo Automatico

Genera procesos de forma aleatoria de manera continua hasta que el usuario presione `Ctrl+C`.
```bash
./bin/client auto <burst_min> <burst_max> <tasa_min> <tasa_max>
```

Ejemplo: burst entre 1 y 10, tasa de creacion entre 2 y 5 segundos:
```bash
./bin/client auto 1 10 2 5
```

---

## Ejemplo de Uso Completo

En una terminal, iniciar el servidor con Round Robin y quantum de 2:
```bash
./bin/server 4 2
```

En otra terminal, iniciar el cliente en modo automatico:
```bash
./bin/client auto 1 15 1 4
```

---

## Salida del Servidor al Apagar

Al presionar `Ctrl+C` en el servidor, se despliega un resumen con:

- Cantidad total de procesos ejecutados
- Segundos con CPU ocioso
- Tabla de Turn Around Time (TAT) por proceso
- Tabla de Waiting Time (WT) por proceso
- Promedio de Waiting Time