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

#define GREY   "\e[30;1m"
#define RED    "\e[31m"
#define BGREEN "\e[32;1m"
#define BWHITE "\e[37;1m"
#define RESET  "\e[0m"

typedef struct node_ops_t {
  sqlite3_stmt *select, *insert, *update, *select_root, *mark_all,
    *print_changes;
} node_ops_t;

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

typedef struct node_list_t {
  node_t *items;
  size_t length, size;
} node_list_t;

void node_ops_init(node_ops_t *self, sqlite3 *db);
void node_ops_dest(node_ops_t *self);
bool node_ops_select(node_ops_t *self, node_t *node, int parent,
                     const char *name);
bool node_ops_insert(node_ops_t *self, node_t *node);
bool node_ops_update(node_ops_t *self, const node_t *node);
bool node_ops_select_root(node_ops_t *self, node_list_t *list);
bool node_ops_mark_all(node_ops_t *self);
bool node_ops_print_changes(node_ops_t *self);
void node_list_init(node_list_t *self);
void node_list_dest(node_list_t *self);
node_t *node_list_reserve(node_list_t *self);

#endif
