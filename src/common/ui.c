#include "ui.h"
#include <string.h>

/* Imprime un banner en arte ASCII dependiendo de la aplicación.
 Se han adaptado las rutinas para soportar los distintos planificadores
 del sistema operativo simulado. */
void print_welcome_banner(const char *app_name) {

    printf("%s\n", COLOR_CYAN);

    if (strstr(app_name, "Cliente") != NULL) {
        printf("   .-------------------------.\n");
        printf("   | Bienvenido a %-10s |\n", app_name);
        printf("   '-------------------------'\n");
        printf("          /\n");
        printf("                   YAao,\n");
        printf("                   Y8888b,\n");
        printf("                 ,oA8888888b,\n");
        printf("             ,aaad8888888888888888bo,\n");
        printf("          ,d888888888888888888888888888b,\n");
        printf("        ,888888888888888888888888888888888b,\n");
        printf("       d8888888888888888888888888888888888888,\n");
        printf("      d888888888888888888888888888888888888888b\n");
        printf("     d888888P'                    `Y888888888888,\n");
        printf("     88888P'                    Ybaaaa8888888888l\n");
        printf("    a8888'                      `Y8888P' `V888888\n");
        printf("  d8888888a                                `Y8888\n");
        printf(" AY/'' `\\Y8b                                 ``Y8b\n");
        printf(" Y'      `YP                                    ~~\n");
        printf("          `'\n");
    }
    else if (strstr(app_name, "FIFO") != NULL) {
        printf("   .-------------------------.\n");
        printf("   | Servidor OS: %-10s |\n", app_name);
        printf("   '-------------------------'\n");
        printf("          /\n");
        printf("        __,__\n");
        printf("  .--.  .-\"     \"-.  .--.\n");
        printf(" / .. \\/  .-. .-.  \\/ .. \\\n");
        printf("| |  '|  /   Y   \\  |'  | |\n");
        printf("| \\   \\  \\ 0 | 0 /  /   / |\n");
        printf(" \\ '- ,\\.-\"`` ``\"-./, -' /\n");
        printf("  `'-' /_   ^ ^   _\\ '-'`\n");
        printf("      |  \\._   _./  |\n");
        printf("      \\   \\ `~` /   /\n");
        printf("       '._ '-=-' _.'\n");
        printf("          '~---~'\n");
    } 
    else if (strstr(app_name, "RR") != NULL) {
        printf("   .-------------------------.\n");
        printf("   | Servidor OS: %-10s |\n", app_name);
        printf("   '-------------------------'\n");
        printf("          /\n");
        printf("      (()__(()\n");
        printf("      /       \\ \n");
        printf("     ( /    \\  )\n");
        printf("      \\ o o    /\n");
        printf("      (_()_)__/ \\ \n");
        printf("     / _,==.____ \\ \n");
        printf("     (   |--|      )\n");
        printf("     /\\_.|__|'-.__/\\_ \n");
        printf("    / (        /     \\ \n");
        printf("    \\  \\      (      /\n");
        printf("     )  '._____)    /\n");
        printf("  (((____.--(((____/\n");
    } 
    else if (strstr(app_name, "SJF") != NULL || strstr(app_name, "HPF") != NULL) {
        printf("   .-------------------------.\n");
        printf("   | Servidor OS: %-10s |\n", app_name);
        printf("   '-------------------------'\n");
        printf("          /\n");
        printf("             _         _\n");
        printf(" __   ___.--'_`.     .'_`--.___   __\n");
        printf("( _`.'. -   'o` )   ( 'o`   - .`.'_ )\n");
        printf("_\\.'_'      _.-'     `-._      `_`./_\n");
        printf("( \\`. )   //\\`         '/\\\\    ( .'/ )\n");
        printf(" \\_`-'`---'\\\\__,       ,__//`---'`-'_/\n");
        printf("  \\`        `-\\         /-'        '/\n");
        printf("   `                               '\n");
    } 
    
    printf("%s\n", COLOR_RESET);
}

void print_info(const char *message) {
    printf("[%sINFO%s] %s\n", COLOR_BLUE, COLOR_RESET, message);
}

void print_error(const char *message) {
    printf("[%sERROR%s] %s\n", COLOR_PURPLE, COLOR_RESET, message);
}

void print_success(const char *message) {
    printf("[%s OK %s] %s\n", COLOR_GREEN, COLOR_RESET, message);
}

void print_fatal_error(const char *message) {
    printf("%s", COLOR_PURPLE);
    printf("     .-\"\"\"\"\"\"-.\n");
    printf("   .'          '.\n");
    printf("  /   O      O   \\\n");
    printf(" :                :\n");
    printf(" |                |   \n");
    printf(" :    .------.    :\n");
    printf("  \\  '        '  /\n");
    printf("   '.          .'\n");
    printf("     '-......-'\n");
    printf("\n[FATAL] %s\n", message);
    printf("%s", COLOR_RESET);
}