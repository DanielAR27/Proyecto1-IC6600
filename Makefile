# Variables de compilación
CC = gcc
CFLAGS = -Wall -Wextra -pthread -I./src/common

# Definición de directorios
SRC_DIR = src
BIN_DIR = bin
COMMON_DIR = $(SRC_DIR)/common
CLIENT_DIR = $(SRC_DIR)/client
SERVER_DIR = $(SRC_DIR)/server

# Fuentes y objetos compartidos
COMMON_SRC = $(COMMON_DIR)/ui.c $(COMMON_DIR)/sockets.c $(COMMON_DIR)/queue.c
COMMON_OBJ = $(COMMON_SRC:.c=.o)

# Regla principal: compila tanto el cliente como el servidor
all: directories client server

# Regla para asegurar que el directorio binario exista
directories:
	@mkdir -p $(BIN_DIR)

# Compilación del ejecutable del cliente
client: $(CLIENT_DIR)/client.o $(COMMON_OBJ)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/client $^

# Compilación del ejecutable del servidor
server: $(SERVER_DIR)/server.o $(SERVER_DIR)/job_scheduler.o $(SERVER_DIR)/cpu_scheduler.o $(COMMON_OBJ)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/server $^

# Regla genérica para compilar archivos .c a .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regla para limpiar los archivos compilados
clean:
	rm -f $(CLIENT_DIR)/*.o $(SERVER_DIR)/*.o $(COMMON_DIR)/*.o
	rm -f $(BIN_DIR)/client $(BIN_DIR)/server