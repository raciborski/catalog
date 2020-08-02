/**************************
 * visitor functions      *
 * written by: raciborski *
 **************************/

#define __USE_XOPEN2K8
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "visitor.h"

#define GREY   "\e[30;1m"
#define RED    "\e[31m"
#define BGREEN "\e[32;1m"
#define BWHITE "\e[37;1m"
#define RESET  "\e[0m"

static const char *node_status_sym[] = {GREY "not" RESET, BGREEN "add" RESET,
                                        RED "del" RESET, BWHITE "mod" RESET};

static bool sync_folder(node_ops_t *ops, int dir_id, int dir_fd);

bool visit_root_path(node_ops_t *ops, const char *path) {
  int dir_fd = open(path, O_RDONLY);
  bool result;

  if((result = sync_folder(ops, 1, dir_fd)))
    close(dir_fd);
  return result;
}

static bool sync_folder(node_ops_t *ops, int dir_id, int dir_fd) {
  DIR *stream;
  node_t node;
  int subdir_fd;
  struct stat props;
  struct dirent *entry;

  if(dir_fd < 0 || (stream = fdopendir(dir_fd)) == NULL)
    return false;

  while((entry = readdir(stream)) != NULL) {
    if(!strcmp(".", entry->d_name) || !strcmp("..", entry->d_name))
      continue;

    fstatat(dir_fd, entry->d_name, &props, 0);
    if(!node_ops_select(ops, &node, dir_id, entry->d_name)) {
      node_init(&node, dir_id, entry->d_name, props.st_mtime,
                S_ISREG(props.st_mode) ? TYPE_FILE : TYPE_FOLDER);
      node_ops_insert(ops, &node);
    }
    else {
      node_sync(&node, props.st_mtime);
      node_ops_update(ops, &node);
    }
    if(node.type == TYPE_FOLDER) {
      subdir_fd = openat(dir_fd, entry->d_name, O_RDONLY);
      if(sync_folder(ops, node.id, subdir_fd))
        close(subdir_fd);
    }
  }

  closedir(stream);
  return true;
}

void visit_path_with_status(const char *path, node_status_t status) {
  printf("[%s] %s\n", node_status_sym[status], path);
}
