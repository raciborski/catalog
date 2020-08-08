/**************************
 * node operations        *
 * written by: raciborski *
 **************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "keccak.h"
#include "node.h"

static void node_bind_with_id(const node_t *self, sqlite3_stmt *query);
static void node_bind_only_id(sqlite3_stmt *query, int id);
static void node_bind_with_path(const node_t *self, sqlite3_stmt *query);
static void node_bind_only_path(sqlite3_stmt *query, int parent,
                                const char *name);
static void node_bind_metadata(const node_t *self, sqlite3_stmt *query,
                               int offset);

void node_ops_init(node_ops_t *self, sqlite3 *db) {
  sqlite3_exec(db,
               "PRAGMA foreign_keys = ON;"
               "CREATE TABLE IF NOT EXISTS nodes("
               "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "  parent INTEGER,"
               "  name TEXT NOT NULL CHECK(length(name) > 0),"
               "  type INTEGER NOT NULL CHECK(type BETWEEN 0 AND 1),"
               "  date INTEGER NOT NULL CHECK(date >= 0),"
               "  hash BLOB CHECK(length(hash) = 16),"
               "  status INTEGER NOT NULL CHECK(status BETWEEN 0 AND 3),"
               "  UNIQUE(parent, name),"
               "  FOREIGN KEY(parent) REFERENCES nodes(id) ON DELETE CASCADE"
               ");",
               NULL, NULL, NULL);

  sqlite3_prepare_v2(db,
                     "SELECT "
                     "  id, parent, name, type, date, hash, status "
                     "FROM nodes "
                     "WHERE parent IS ?1 AND name = ?2;",
                     -1, &self->select, NULL);

  sqlite3_prepare_v2(db,
                     "INSERT INTO nodes("
                     "  parent, name, type, date, hash, status "
                     ") "
                     "VALUES( "
                     "  ?1, ?2, ?3, ?4, ?5, ?6 "
                     ");",
                     -1, &self->insert, NULL);

  sqlite3_prepare_v2(db,
                     "UPDATE nodes "
                     "SET type = ?2, date = ?3, hash = ?4, status = ?5 "
                     "WHERE id = ?1;",
                     -1, &self->update, NULL);

  sqlite3_prepare_v2(db,
                     "DELETE FROM nodes "
                     "WHERE parent IS ?1 AND name = ?2;",
                     -1, &self->remove, NULL);

  sqlite3_prepare_v2(db,
                     "UPDATE nodes "
                     "SET status = ?1 "
                     "WHERE parent IS NOT NULL;",
                     -1, &self->mark_branches, NULL);

  sqlite3_prepare_v2(db,
                     "DELETE FROM nodes "
                     "WHERE status = 2;",
                     -1, &self->remove_marked, NULL);

  sqlite3_prepare_v2(db,
                     "SELECT "
                     "  id, name "
                     "FROM nodes "
                     "WHERE parent IS NULL;",
                     -1, &self->select_root, NULL);

  sqlite3_prepare_v2(
    db,
    "WITH RECURSIVE paths(id, name, status) AS ( "
    "  SELECT "
    "    id, name, status "
    "  FROM "
    "    nodes "
    "  WHERE "
    "    parent IS NULL "
    "  UNION SELECT "
    "    nodes.id, paths.name || '/' || nodes.name, nodes.status "
    "  FROM "
    "    nodes "
    "  JOIN "
    "    paths "
    "  ON "
    "   nodes.parent = paths.id "
    ") "
    "SELECT "
    "  name, status "
    "FROM "
    "  paths "
    "WHERE "
    "  status != 0 "
    "ORDER BY "
    "  name;",
    -1, &self->select_changes, NULL);
}

void node_ops_dest(node_ops_t *self) {
  sqlite3_finalize(self->select);
  sqlite3_finalize(self->insert);
  sqlite3_finalize(self->update);
  sqlite3_finalize(self->remove);
  sqlite3_finalize(self->mark_branches);
  sqlite3_finalize(self->remove_marked);
  sqlite3_finalize(self->select_root);
  sqlite3_finalize(self->select_changes);
}

bool node_ops_select(node_ops_t *self, node_t *node, int parent,
                     const char *name) {
  bool result;
  sqlite3_stmt *query = self->select;

  node_bind_only_path(query, parent, name);
  if((result = sqlite3_step(query) == SQLITE_ROW)) {
    memset(node, 0, sizeof(node_t));
    node->id = sqlite3_column_int(query, 0);
    node->parent = sqlite3_column_int(query, 1);
    strncpy(node->name, (const char *)sqlite3_column_text(query, 2), NAME_MAX);
    node->type = sqlite3_column_int(query, 3);
    node->date = sqlite3_column_int64(query, 4);
    if(node->type == TYPE_FILE)
      memcpy(node->hash, sqlite3_column_blob(query, 5), 16);
    node->status = sqlite3_column_int(query, 6);
  }
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

bool node_ops_remove(node_ops_t *self, int parent, const char *path) {
  bool result;
  sqlite3_stmt *query = self->remove;

  node_bind_only_path(query, parent, path);
  result = sqlite3_step(query) == SQLITE_DONE &&
           sqlite3_changes(sqlite3_db_handle(query));
  sqlite3_reset(query);
  return result;
}

bool node_ops_mark_branches(node_ops_t *self, node_status_t status) {
  bool result;
  sqlite3_stmt *query = self->mark_branches;

  sqlite3_bind_int(query, 1, status);
  result = sqlite3_step(query) == SQLITE_DONE;
  sqlite3_reset(query);
  return result;
}

bool node_ops_remove_marked(node_ops_t *self) {
  bool result;
  sqlite3_stmt *query = self->remove_marked;

  result = sqlite3_step(query) == SQLITE_DONE;
  sqlite3_reset(query);
  return result;
}

bool node_ops_select_root(node_ops_t *self,
                          bool (*callback)(node_ops_t *, int, const char *,
                                           bool),
                          bool force) {
  int id, result;
  const char *path;
  sqlite3_stmt *query = self->select_root;

  while((result = sqlite3_step(query)) == SQLITE_ROW) {
    id = sqlite3_column_int(query, 0);
    path = (const char *)sqlite3_column_text(query, 1);
    if(!callback(self, id, path, force))
      break;
  }
  sqlite3_reset(query);
  return result == SQLITE_DONE;
}

bool node_ops_select_changes(node_ops_t *self,
                             void (*callback)(const char *, node_status_t)) {
  int result;
  const char *path;
  node_status_t status;
  sqlite3_stmt *query = self->select_changes;

  while((result = sqlite3_step(query)) == SQLITE_ROW) {
    path = (const char *)sqlite3_column_text(query, 0);
    status = sqlite3_column_int(query, 1);
    callback(path, status);
  }
  sqlite3_reset(query);
  return result == SQLITE_DONE;
}

void node_init(node_t *self, int dir_fd, int parent, const char *name,
               node_type_t type, time_t date) {
  self->parent = parent;
  strncpy(self->name, name, NAME_MAX);
  self->type = type;
  if(type == TYPE_FILE)
    sha3_file(self->hash, dir_fd, name);
  self->date = date;
  self->status = STATUS_ADD;
}

void node_sync(node_t *self, int dir_fd, time_t date, bool force) {
  uint8_t new_hash[16];

  if(self->date != date || (force && self->type == TYPE_FILE)) {
    self->date = date;
    if(self->type == TYPE_FILE) {
      sha3_file(new_hash, dir_fd, self->name);
      if(memcmp(self->hash, new_hash, 16)) {
        memcpy(self->hash, new_hash, 16);
        self->status = STATUS_MOD;
      }
      else
        self->status = STATUS_NORM;
    }
    else
      self->status = STATUS_MOD;
  }
  else
    self->status = STATUS_NORM;
}

static void node_bind_with_id(const node_t *self, sqlite3_stmt *query) {
  node_bind_only_id(query, self->id);
  node_bind_metadata(self, query, 1);
}

static void node_bind_only_id(sqlite3_stmt *query, int id) {
  sqlite3_bind_int(query, 1, id);
}

static void node_bind_with_path(const node_t *self, sqlite3_stmt *query) {
  node_bind_only_path(query, self->parent, self->name);
  node_bind_metadata(self, query, 2);
}

static void node_bind_only_path(sqlite3_stmt *query, int parent,
                                const char *name) {
  if(parent >= 0)
    sqlite3_bind_int(query, 1, parent);
  else
    sqlite3_bind_null(query, 1);
  sqlite3_bind_text(query, 2, name, -1, SQLITE_STATIC);
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
