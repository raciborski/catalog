/**************************
 * print functions        *
 * written by: raciborski *
 **************************/

#include <stdio.h>

#include "print.h"

void print_help() {
  printf("|> %scatalog%s [-fad] [path]\n"
         "------------------------\n"
         "  * %s-f%s       Force re-hashing of every node.\n"
         "  * %s-a path%s  Add new path to the database.\n"
         "  * %s-d path%s  Delete path from the database.\n\n",
         BCYAN, RESET, BWHITE, RESET, BWHITE, RESET, BWHITE, RESET);
}

void print_info(const char *message) {
  printf("[%sinfo%s] %s\n", BLUE, RESET, message);
}

void print_fail(const char *message) {
  printf("[%sfail%s] %s\n", RED, RESET, message);
}
