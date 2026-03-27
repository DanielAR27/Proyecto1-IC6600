#ifndef UI_H
#define UI_H

#include <stdio.h>

/* Se definen las secuencias de escape ANSI para los colores de la terminal. */
#define COLOR_RESET   "\x1b[0m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_PURPLE  "\x1b[35m"
#define COLOR_GREEN   "\x1b[32m"

/* Imprime un banner en arte ASCII.
 El diseño varía dependiendo de la cadena de caracteres proporcionada
 en el parámetro app_name. */
void print_welcome_banner(const char *app_name);

/* Muestra un mensaje informativo en la salida estándar.
 El mensaje es precedido por una etiqueta de [INFO] en color azul. */
void print_info(const char *message);

/* Muestra un mensaje de error en la salida estándar.
 El mensaje es precedido por una etiqueta de [ERROR] en color púrpura. */
void print_error(const char *message);

/* Muestra un mensaje de éxito en la salida estándar.
 El mensaje es precedido por una etiqueta de [OK] en color verde. */
void print_success(const char *message);

/* Imprime un mensaje de error crítico acompañado de arte ASCII en color púrpura.
  Se utiliza para notificar fallos irrecuperables en el sistema. */
void print_fatal_error(const char *message);

#endif 