/**************************
 * filesystem operations  *
 * written by: raciborski *
 **************************/

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "node.h"
#include <stdbool.h>

void add_root_path(node_ops_t *ops, const char *path);
void delete_root_path(node_ops_t *ops, const char *path);
void check_root_paths(node_ops_t *ops, bool force);

#endif
