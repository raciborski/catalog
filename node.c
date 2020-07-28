/**************************
 * node operations        *
 * written by: raciborski *
 **************************/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "node.h"

void node_ops_init(node_ops_t *self, sqlite3 *db) {
  sqlite3_prepare_v2(db,
                     "SELECT "
                     "  id, parent, name, type, date, hash, status "
                     "FROM nodes "
                     "WHERE parent = ?1 AND name = ?2;",
                     -1, &self->select, NULL);

  sqlite3_prepare_v2(db,
                     "INSERT INTO nodes("
                     "  parent, name, type, date, hash, status"
                     ") "
                     "VALUES("
                     "  ?1, ?2, ?3, ?4, ?5, ?6"
                     ");",
                     -1, &self->insert, NULL);

  sqlite3_prepare_v2(db,
                     "UPDATE nodes "
                     "SET type = ?2, date = ?3, hash = ?4, status = ?5 "
                     "WHERE id = ?1;",
                     -1, &self->update, NULL);
  sqlite3_prepare_v2(db,
                     "SELECT "
                     "  id, parent, name, type, date, hash, status "
                     "FROM nodes "
                     "WHERE parent IS NULL;",
                     -1, &self->select_root, NULL);
}

void node_ops_dest(node_ops_t *self) {
  sqlite3_finalize(self->select);
  sqlite3_finalize(self->insert);
  sqlite3_finalize(self->update);
  sqlite3_finalize(self->select_root);
}

static void node_from_query(node_t *self, sqlite3_stmt *query) {
  memset(self, 0, sizeof(node_t));
  self->id = sqlite3_column_int(query, 0);
  self->parent = sqlite3_column_int(query, 1);
  strncpy(self->name, (const char *)sqlite3_column_text(query, 2), NAME_MAX);
  self->type = sqlite3_column_int(query, 3);
  self->date = sqlite3_column_int64(query, 4);
  if(self->type == TYPE_FILE)
    memcpy(self->hash, sqlite3_column_blob(query, 5), 16);
  self->status = sqlite3_column_int(query, 6);
}

static void node_bind_metadata(const node_t *self, sqlite3_stmt *query,
                               int offset) {
  sqlite3_bind_int(query, offset + 1, self->type);
  sqlite3_bind_int64(query, offset + 2, self->date);
  if(self->type == TYPE_FILE)
    sqlite3_bind_blob(query, offset + 3, self->hash, 16, SQLITE_STATIC);
  else
    sqlite3_bind_null(query, offset + 3);
  sqlite3_bind_int(query, offset + 4, self->status);
}

static void node_bind_with_id(const node_t *self, sqlite3_stmt *query) {
  sqlite3_bind_int(query, 1, self->id);
  node_bind_metadata(self, query, 1);
}

static void node_bind_with_path(const node_t *self, sqlite3_stmt *query) {
  sqlite3_bind_int(query, 1, self->parent);
  sqlite3_bind_text(query, 2, self->name, -1, SQLITE_STATIC);
  node_bind_metadata(self, query, 2);
}

bool node_ops_select(node_ops_t *self, node_t *node, int parent,
                     const char *name) {
  bool result;
  sqlite3_stmt *query = self->select;

  sqlite3_bind_int(query, 1, parent);
  sqlite3_bind_text(query, 2, name, -1, SQLITE_STATIC);
  if((result = sqlite3_step(query) == SQLITE_ROW))
    node_from_query(node, query);
  sqlite3_reset(query);
  return result;
}

bool node_ops_insert(node_ops_t *self, node_t *node) {
  bool result;
  sqlite3_stmt *query = self->insert;

  node_bind_with_path(node, query);
  result = sqlite3_step(query) == SQLITE_DONE;
  node->id = sqlite3_last_insert_rowid(sqlite3_db_handle(query));
  sqlite3_reset(query);
  return result;
}

bool node_ops_update(node_ops_t *self, const node_t *node) {
  bool result;
  sqlite3_stmt *query = self->update;

  node_bind_with_id(node, query);
  result = sqlite3_step(query) == SQLITE_DONE;
  sqlite3_reset(query);
  return result;
}

bool node_ops_select_root(node_ops_t *self, node_list_t *list) {
  int result;
  sqlite3_stmt *query = self->select_root;

  node_list_init(list);
  while((result = sqlite3_step(query)) == SQLITE_ROW)
    node_from_query(node_list_reserve(list), query);
  sqlite3_reset(query);
  return result == SQLITE_DONE;
}

void node_list_init(node_list_t *self) {
  self->length = 0;
  self->size = 1;
  self->items = malloc(sizeof(node_t));
}

void node_list_dest(node_list_t *self) {
  free(self->items);
}

node_t *node_list_reserve(node_list_t *self) {
  if(++self->length >= self->size) {
    self->size *= 2;
    self->items = realloc(self->items, sizeof(node_t) * self->size);
  }
  return self->items + self->length - 1;
}
