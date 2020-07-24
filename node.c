/**************************
 * node operations        *
 * written by: raciborski *
 **************************/

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "node.h"

typedef struct node_ops_t {
  sqlite3_stmt *select, *insert, *update, *select_all;
} node_ops_t;

static node_ops_t node_ops;

void node_ops_init(sqlite3 *db) {
  sqlite3_prepare_v2(db,
                     "SELECT "
                     "  id, parent, name, type, date, hash, status "
                     "FROM nodes "
                     "WHERE parent = ?1 AND name = ?2;",
                     -1, &node_ops.select, NULL);

  sqlite3_prepare_v2(db,
                     "INSERT INTO nodes("
                     "  parent, name, type, date, hash, status"
                     ") "
                     "VALUES("
                     "  ?1, ?2, ?3, ?4, ?5, ?6"
                     ");",
                     -1, &node_ops.insert, NULL);

  sqlite3_prepare_v2(db,
                     "UPDATE nodes SET type = ?3, date = ?4, hash = ?5, "
                     "status = ?6 WHERE parent = ?1 AND name = ?2;",
                     -1, &node_ops.update, NULL);
}

void node_ops_dest(void) {
  sqlite3_finalize(node_ops.select);
  sqlite3_finalize(node_ops.insert);
  sqlite3_finalize(node_ops.update);
  sqlite3_finalize(node_ops.select_all);
}

bool node_select(node_t *self, int parent, const char *name) {
  bool result;
  sqlite3_stmt *query = node_ops.select;

  sqlite3_bind_int(query, 1, parent);
  sqlite3_bind_text(query, 2, name, -1, SQLITE_STATIC);
  if((result = sqlite3_step(query) == SQLITE_ROW)) {
    self->id = sqlite3_column_int(query, 0);
    self->parent = sqlite3_column_int(query, 1);
    strncpy(self->name, (const char *)sqlite3_column_text(query, 2), NAME_MAX);
    self->type = sqlite3_column_int(query, 3);
    self->date = sqlite3_column_int64(query, 4);
    if(self->type == TYPE_FILE)
      memcpy(self->hash, sqlite3_column_blob(query, 5), 16);
    self->status = sqlite3_column_int(query, 6);
  }
  sqlite3_reset(query);
  return result;
}

bool node_insert(node_t *self) {
  bool result;
  sqlite3_stmt *query = node_ops.insert;

  sqlite3_bind_int(query, 1, self->parent);
  sqlite3_bind_text(query, 2, self->name, -1, SQLITE_STATIC);
  sqlite3_bind_int(query, 3, self->type);
  sqlite3_bind_int64(query, 4, self->date);
  if(self->type == TYPE_FILE)
    sqlite3_bind_blob(query, 5, self->hash, 16, SQLITE_STATIC);
  else
    sqlite3_bind_null(query, 5);
  sqlite3_bind_int(query, 6, self->status);
  result = sqlite3_step(query) == SQLITE_DONE;
  // self->id = sqlite3_last_insert_rowid(db);
  self->id = 0;
  sqlite3_reset(query);
  return result;
}

bool node_update(const node_t *node) {
  bool result;
  sqlite3_stmt *query = node_ops.update;

  sqlite3_bind_int(query, 1, node->parent);
  sqlite3_bind_text(query, 2, node->name, -1, SQLITE_STATIC);
  sqlite3_bind_int(query, 3, node->type);
  sqlite3_bind_int64(query, 4, node->date);
  if(node->type == TYPE_FILE)
    sqlite3_bind_blob(query, 5, node->hash, 16, SQLITE_STATIC);
  else
    sqlite3_bind_null(query, 5);
  sqlite3_bind_int(query, 6, node->status);
  result = sqlite3_step(query) == SQLITE_DONE;
  sqlite3_reset(query);
  return result;
}
