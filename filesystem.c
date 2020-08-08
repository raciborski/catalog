/**************************
 * filesystem operations  *
 * written by: raciborski *
 **************************/

#define __USE_XOPEN2K8
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "filesystem.h"
#include "print.h"

static const char *node_status_sym[] = {GREY "not" RESET, BGREEN "add" RESET,
                                        RED "del" RESET, BWHITE "mod" RESET};

static bool visit_root_path(node_ops_t *ops, int dir_id, const char *path,
                            bool force);
static bool sync_folder(node_ops_t *ops, int dir_id, int dir_fd, bool force);
static void visit_path_with_status(const char *path, node_status_t status);

void add_root_path(node_ops_t *ops, const char *path) {
  node_t node;
  struct stat props;
  char new_path[PATH_MAX] = {0};

  if(realpath(path, new_path) && !stat(new_path, &props)) {
    if(S_ISDIR(props.st_mode)) {
      if(!node_ops_select(ops, &node, -1, new_path)) {
        node_init(&node, -1, -1, new_path, TYPE_FOLDER, props.st_mtime);
        node.status = STATUS_NORM;
        node_ops_insert(ops, &node);
        print_info("Path added successfully.");
      }
      else
        print_fail("Path has been already added.");
    }
    else
      print_fail("Path is not a folder.");
  }
  else
    print_fail("Path does not exist.");
}

void delete_root_path(node_ops_t *ops, const char *path) {
  char new_path[PATH_MAX] = {0};

  if(realpath(path, new_path) && node_ops_remove(ops, -1, new_path))
    print_info("Path deleted successfully.");
  else
    print_fail("Path does not exist in the database.");
}

void check_root_paths(node_ops_t *ops, bool force) {
  node_ops_mark_branches(ops, STATUS_DEL);
  if(node_ops_select_root(ops, visit_root_path, force)) {
    node_ops_select_changes(ops, visit_path_with_status);
    node_ops_remove_marked(ops);
    node_ops_mark_branches(ops, STATUS_NORM);
  }
  else
    print_fail("Failed to acccess one or multiple folders.");
}

static bool visit_root_path(node_ops_t *ops, int dir_id, const char *path,
                            bool force) {
  int dir_fd = open(path, O_RDONLY);
  bool result;

  if((result = sync_folder(ops, dir_id, dir_fd, force)))
    close(dir_fd);
  return result;
}

static bool sync_folder(node_ops_t *ops, int dir_id, int dir_fd, bool force) {
  DIR *stream;
  node_t node;
  int subdir_fd;
  struct stat props;
  struct dirent *entry;

  if(dir_fd < 0 || (stream = fdopendir(dir_fd)) == NULL)
    return false;

  while((entry = readdir(stream)) != NULL) {
    if(!strcmp(".", entry->d_name) || !strcmp("..", entry->d_name) ||
       fstatat(dir_fd, entry->d_name, &props, 0))
      continue;

    if(!node_ops_select(ops, &node, dir_id, entry->d_name)) {
      node_init(&node, dir_fd, dir_id, entry->d_name,
                S_ISREG(props.st_mode) ? TYPE_FILE : TYPE_FOLDER,
                props.st_mtime);
      node_ops_insert(ops, &node);
    }
    else {
      node_sync(&node, dir_fd, props.st_mtime, force);
      node_ops_update(ops, &node);
    }
    if(node.type == TYPE_FOLDER) {
      subdir_fd = openat(dir_fd, entry->d_name, O_RDONLY);
      if(sync_folder(ops, node.id, subdir_fd, force))
        close(subdir_fd);
    }
  }

  closedir(stream);
  return true;
}

static void visit_path_with_status(const char *path, node_status_t status) {
  printf("[%s] %s\n", node_status_sym[status], path);
}
