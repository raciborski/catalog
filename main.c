/**************************
 * entry point            *
 * written by: raciborski *
 **************************/

#include <string.h>

#include <sqlite3.h>

#include "filesystem.h"
#include "node.h"
#include "print.h"

static const char *DB_PATH = "catalog.db";

int main(int argc, char *argv[]) {
  sqlite3 *db;
  node_ops_t ops;

  if(!sqlite3_open(DB_PATH, &db)) {
    node_ops_init(&ops, db);
    if(argc == 3 && !strcmp("-a", argv[1]))
      add_root_path(&ops, argv[2]);
    else if(argc == 3 && !strcmp("-d", argv[1]))
      delete_root_path(&ops, argv[2]);
    else if(argc == 2 && !strcmp("-l", argv[1]))
      list_root_paths(&ops);
    else if(argc == 2 && !strcmp("-f", argv[1]))
      check_root_paths(&ops, true);
    else if(argc == 1)
      check_root_paths(&ops, false);
    else
      print_help();
    node_ops_dest(&ops);
  }
  else
    print_fail("Cannot open database.");

  sqlite3_close(db);
  return 0;
}
