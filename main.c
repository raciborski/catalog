/**************************
 * entry point            *
 * written by: raciborski *
 **************************/

#include <stdio.h>

#include <sqlite3.h>

#include "node.h"
#include "visitor.h"

int main(void) {
  sqlite3 *db;
  node_ops_t ops;

  if(sqlite3_open("test.db", &db)) {
    printf("Cannot open database.\n");
    return 1;
  }

  node_ops_init(&ops, db);
  node_ops_mark_all(&ops);
  node_ops_select_root(&ops, visit_root_path);
  node_ops_select_changes(&ops, visit_path_with_status);
  node_ops_dest(&ops);

  sqlite3_close(db);
  return 0;
}
