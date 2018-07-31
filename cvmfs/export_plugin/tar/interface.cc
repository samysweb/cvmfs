/**
 * This file is part of the CernVM File System.
 */
#include "interface.h"

#include <stdio.h>

#include "export_plugin/fs_traversal_interface.h"
#include "util/posix.h"

// Some tar inspirations from https://github.com/LucasSeveryn/pintOS/blob/master/src/lib/ustar.c


void tar_list_dir(struct fs_traversal_context *ctx,
  const char *dir,
  char ***buf,
  size_t *len) {

}

int tar_get_stat(struct fs_traversal_context *ctx,
  const char *path, struct cvmfs_attr *stat_result, bool get_hash) {
  
}

int tar_set_meta(struct fs_traversal_context *ctx,
  const char *path, const struct cvmfs_attr *stat_info) {

}
const char *tar_get_identifier(struct fs_traversal_context *ctx,
  const struct cvmfs_attr *stat) {
  
}

bool tar_has_file(struct fs_traversal_context *ctx,
  const char *ident) {
  
}

int tar_do_unlink(struct fs_traversal_context *ctx,
  const char *path) {
  
}

int tar_do_unlink_cb(const char *fpath,
  const struct stat *sb,
  int typeflag,
  struct FTW *ftwbuf) {
  
}

int tar_do_rmdir(struct fs_traversal_context *ctx,
              const char *path) {
  
}

int tar_do_link(struct fs_traversal_context *ctx,
  const char *path,
  const char *identifier) {
  
}

int tar_do_mkdir(struct fs_traversal_context *ctx,
              const char *path,
              const struct cvmfs_attr *stat_info) {
  
}

int tar_do_symlink(struct fs_traversal_context *ctx,
              const char *src,
              const char *dest,
              const struct cvmfs_attr *stat_info) {
  
}

int tar_touch(struct fs_traversal_context *ctx,
              const struct cvmfs_attr *stat_info) {
  
}

void *tar_get_handle(struct fs_traversal_context *ctx,
              const char *identifier) {
  
}

int tar_do_fopen(void *file_ctx, fs_open_type op_mode) {
  
}

int tar_do_fclose(void *file_ctx) {
  
}

int tar_do_fread(void *file_ctx, char *buff, size_t len, size_t *read_len) {
  
}

int tar_do_fwrite(void *file_ctx, const char *buff, size_t len) {
  
}

void tar_do_ffree(void *file_ctx) {
  
}

bool tar_is_hash_consistent(struct fs_traversal_context *ctx,
                const struct cvmfs_attr *stat_info) {
  
}

int tar_garbage_collector(struct fs_traversal_context *ctx) {
  // TODO(steuber): Implementation
  return -1;
}

struct fs_traversal_tar_context {
  FILE *fd;
};

int setup_basic_tar_file(std::string path) {
  // TODO(steuber): Make more robust (error handling)
  struct ustar_header cvmfs_dir = BuildHeader(
    "cvmfs",
    0777,
    0,
    0,
    0,
    time(NULL),
    USTAR_DIRECTORY,
    "root",
    "root");
  struct ustar_header data_dir = BuildHeader(
    "cvmfs/.data",
    0777,
    0,
    0,
    0,
    time(NULL),
    USTAR_DIRECTORY,
    "root",
    "root");
  FILE *tar_file = fopen(path.c_str(), "w");
  fwrite(&cvmfs_dir, sizeof(struct ustar_header), 1, tar_file);
  fwrite(&data_dir, sizeof(struct ustar_header), 1, tar_file);
  fclose(tar_file);
  printf("%s", path.c_str());
}

struct fs_traversal_context *tar_initialize(
  const char *repo,
  const char *base,
  const char *data,
  const char *config) {
  fs_traversal_context *result = new struct fs_traversal_context;
  result->version = 1;
  result->repo = strdup(repo);
  if (base == NULL) {
    base = "/tmp/cvmfs-shrinkwrap.tar";
  }
  if (!FileExists(base)) {
    setup_basic_tar_file(base);
  }
}

void tar_finalize(struct fs_traversal_context *ctx) {
  
}



struct fs_traversal *tar_get_interface() {
  struct fs_traversal *result = new struct fs_traversal;
  result->initialize = tar_initialize;
  result->finalize = tar_finalize;
  result->list_dir = tar_list_dir;
  result->get_stat = tar_get_stat;
  result->is_hash_consistent = tar_is_hash_consistent;
  result->set_meta = tar_set_meta;
  result->has_file = tar_has_file;
  result->get_identifier = tar_get_identifier;
  result->do_link = tar_do_link;
  result->do_unlink = tar_do_unlink;
  result->do_mkdir = tar_do_mkdir;
  result->do_rmdir = tar_do_rmdir;
  result->touch = tar_touch;
  result->get_handle = tar_get_handle;
  result->do_symlink = tar_do_symlink;
  result->garbage_collector = tar_garbage_collector;

  result->do_fopen = tar_do_fopen;
  result->do_fclose = tar_do_fclose;
  result->do_fread = tar_do_fread;
  result->do_fwrite = tar_do_fwrite;
  result->do_ffree = tar_do_ffree;

  return result;
}
