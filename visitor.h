/**************************
 * visitor functions      *
 * written by: raciborski *
 **************************/

#ifndef FOLDER_H
#define FOLDER_H

#include "node.h"
#include <stdbool.h>

bool visit_root_path(node_ops_t *ops, const char *path);
void visit_path_with_status(const char *path, node_status_t status);

#endif
