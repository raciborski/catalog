/**************************
 * entry point            *
 * written by: raciborski *
 **************************/

#include <stdio.h>

#include <sqlite3.h>

#include "node.h"
#include "visitor.h"

static const char *DB_PATH = "/home/keccak/Documents/catalog.db";

static void print_error(const char *message) {
  printf("[%serr%s] %s\n", RED, RESET, message);
}

int main(void) {
  sqlite3 *db;
  node_ops_t ops;

  if(!sqlite3_open(DB_PATH, &db)) {
    node_ops_init(&ops, db);
    node_ops_mark_branches(&ops, STATUS_DEL);
    if(node_ops_select_root(&ops, visit_root_path)) {
      node_ops_select_changes(&ops, visit_path_with_status);
      node_ops_remove_marked(&ops);
      node_ops_mark_branches(&ops, STATUS_NORM);
    }
    else
      print_error("Failed to acccess one or multiple folders.");
    node_ops_dest(&ops);
  }
  else
    print_error("Cannot open database.");

  sqlite3_close(db);
  return 0;
}
