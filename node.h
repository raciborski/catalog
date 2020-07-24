/**************************
 * node operations        *
 * written by: raciborski *
 **************************/

#ifndef NODE_H
#define NODE_H

#include <linux/limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

#include <sqlite3.h>

typedef enum node_type_t { TYPE_FOLDER, TYPE_FILE } node_type_t;

typedef enum node_status_t {
  STATUS_NORM,
  STATUS_ADD,
  STATUS_DEL,
  STATUS_MOD
} node_status_t;

typedef struct node_t {
  int id, parent;
  char name[NAME_MAX + 1];
  node_type_t type;
  time_t date;
  uint8_t hash[16];
  node_status_t status;
} node_t;

void node_ops_init(sqlite3 *db);
void node_ops_dest(void);
bool node_select(node_t *self, int parent, const char *name);
bool node_insert(node_t *self);
bool node_update(const node_t *node);

#endif
