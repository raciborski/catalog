/**************************
 * entry point            *
 * written by: raciborski *
 **************************/

#define __USE_XOPEN2K8
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include <sqlite3.h>

#include "node.h"

void visit_folder(node_ops_t *ops, int dir_id, int dir_fd) {
  DIR *stream;
  node_t node;
  struct stat props;
  struct dirent *entry;

  if(dir_fd < 0 || (stream = fdopendir(dir_fd)) == NULL)
    return;

  while((entry = readdir(stream)) != NULL) {
    if(!strcmp(".", entry->d_name) || !strcmp("..", entry->d_name))
      continue;

    fstatat(dir_fd, entry->d_name, &props, 0);
    if(!node_ops_select(ops, &node, dir_id, entry->d_name)) {
      node.parent = dir_id;
      strncpy(node.name, entry->d_name, NAME_MAX);
      node.date = props.st_mtime;
      if(S_ISREG(props.st_mode)) {
        node.type = TYPE_FILE;
        memset(node.hash, 1, 16);
      }
      else
        node.type = TYPE_FOLDER;
      node.status = STATUS_ADD;
      node_ops_insert(ops, &node);
    }
    else {
      if(node.date != props.st_mtime) {
        node.date = props.st_mtime;
        node.status = STATUS_MOD;
        if(node.type == TYPE_FILE)
          memset(node.hash, 2, 16);
      }
      else
        node.status = STATUS_NORM;
      node_ops_update(ops, &node);
    }

    if(node.type == TYPE_FOLDER) {
      int newdir_fd = openat(dir_fd, entry->d_name, 0);

      visit_folder(ops, node.id, newdir_fd);
      close(newdir_fd);
    }
  }

  closedir(stream);
}

int main(void) {
  int i;
  sqlite3 *db;
  node_ops_t ops;
  node_list_t list;

  if(sqlite3_open("test.db", &db)) {
    printf("Cannot open database.\n");
    return 1;
  }

  node_ops_init(&ops, db);
  node_ops_mark_all(&ops);
  node_ops_select_root(&ops, &list);
  for(i = 0; i < list.length; i++) {
    printf("%d: %s\n", i, list.items[i].name);
  }
  visit_folder(&ops, 1, open("watch", O_RDONLY));
  node_list_dest(&list);
  node_ops_dest(&ops);

  sqlite3_close(db);
  return 0;
}
