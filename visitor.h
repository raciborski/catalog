/**************************
 * visitor functions      *
 * written by: raciborski *
 **************************/

#ifndef FOLDER_H
#define FOLDER_H

#include "node.h"
#include <stdbool.h>

#define GREY   "\e[30;1m"
#define RED    "\e[31m"
#define BGREEN "\e[32;1m"
#define BWHITE "\e[37;1m"
#define RESET  "\e[0m"

bool visit_root_path(node_ops_t *ops, const char *path);
void visit_path_with_status(const char *path, node_status_t status);

#endif
