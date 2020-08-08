/**************************
 * print functions        *
 * written by: raciborski *
 **************************/

#ifndef PRINT_H
#define PRINT_H

#define GREY   "\e[30;1m"
#define RED    "\e[31m"
#define BGREEN "\e[32;1m"
#define BLUE   "\e[34m"
#define BCYAN  "\e[36;1m"
#define BWHITE "\e[37;1m"
#define RESET  "\e[0m"

void print_help();
void print_info(const char *message);
void print_fail(const char *message);

#endif
