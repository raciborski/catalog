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

#include <sqlite3.h>

typedef struct node_ops_t {
  sqlite3_stmt *begin, *end, *rollback, *select, *insert, *update, *remove,
    *mark_branches, *remove_marked, *select_root, *select_changes,
    *select_total;
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

void node_ops_init(node_ops_t *self, sqlite3 *db);
void node_ops_dest(node_ops_t *self);
bool node_ops_begin(node_ops_t *self);
bool node_ops_end(node_ops_t *self);
bool node_ops_rollback(node_ops_t *self);
bool node_ops_select(node_ops_t *self, node_t *node, int parent,
                     const char *name);
bool node_ops_insert(node_ops_t *self, node_t *node);
bool node_ops_update(node_ops_t *self, const node_t *node);
bool node_ops_remove(node_ops_t *self, int parent, const char *path);
bool node_ops_mark_branches(node_ops_t *self, node_status_t status);
bool node_ops_remove_marked(node_ops_t *self);
bool node_ops_select_root(node_ops_t *self,
                          bool (*callback)(node_ops_t *, int, const char *,
                                           bool),
                          bool force);
bool node_ops_select_changes(node_ops_t *self,
                             void (*callback)(const char *, node_status_t));
bool node_ops_select_total(node_ops_t *self,
                           void (*callback)(const char *, int));
void node_init(node_t *self, int dir_fd, int parent, const char *name,
               node_type_t type, time_t date);
void node_sync(node_t *self, int dir_fd, time_t date, bool force);

#endif
