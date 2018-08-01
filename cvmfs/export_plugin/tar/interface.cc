/**
 * This file is part of the CernVM File System.
 */
#include "interface.h"

#include <stdio.h>
#include <string>

#include "export_plugin/fs_traversal_interface.h"
#include "logging.h"
#include "util/posix.h"

#include "tar_object.h"


void tar_list_dir(struct fs_traversal_context *ctx,
  const char *dir,
  char ***buf,
  size_t *len) {
  TarObject *tar_ctx = reinterpret_cast<TarObject *>(ctx->ctx);
  tar_ctx->ListDir(std::string(dir), buf, len);
}

int tar_get_stat(struct fs_traversal_context *ctx,
  const char *path, struct cvmfs_attr *stat_result, bool get_hash) {
  TarObject *tar_ctx = reinterpret_cast<TarObject *>(ctx->ctx);
  if (tar_ctx->Stat(std::string(path), stat_result)) {
    return 0;
  }
  return -1;
  // TODO(steuber): Include get_hash!
}

int tar_set_meta(struct fs_traversal_context *ctx,
  const char *path, const struct cvmfs_attr *stat_info) {
  LogCvmfs(kLogCvmfs, kLogStderr, "set_meta currently not supported by tar");
  // TODO(steuber): Remove from interface?
  return -1;
}
char *tar_get_identifier(struct fs_traversal_context *ctx,
  const struct cvmfs_attr *stat) {
  TarObject *tar_ctx = reinterpret_cast<TarObject *>(ctx->ctx);
  return strdup(tar_ctx->GetIdentifier(stat).c_str());
}

bool tar_has_file(struct fs_traversal_context *ctx,
  const char *ident) {
  LogCvmfs(kLogCvmfs, kLogStderr, "has_file currently not supported by tar");
  // TODO(steuber): Remove from interface?
  return -1;
}

int tar_do_unlink(struct fs_traversal_context *ctx,
  const char *path) {
  TarObject *tar_ctx = reinterpret_cast<TarObject *>(ctx->ctx);
  if (tar_ctx->Remove(std::string(path))) {
    return 0;
  }
  return -1;
}

int tar_do_rmdir(struct fs_traversal_context *ctx,
              const char *path) {
  TarObject *tar_ctx = reinterpret_cast<TarObject *>(ctx->ctx);
  if (tar_ctx->Remove(std::string(path))) {
    return 0;
  }
  return -1;
}

int tar_do_link(struct fs_traversal_context *ctx,
  const char *path,
  const char *identifier) {
  TarObject *tar_ctx = reinterpret_cast<TarObject *>(ctx->ctx);
  if (tar_ctx->MkLink(std::string(path), std::string(identifier))) {
    return 0;
  }
  return -1;
}

int tar_do_mkdir(struct fs_traversal_context *ctx,
              const char *path,
              const struct cvmfs_attr *stat_info) {
  TarObject *tar_ctx = reinterpret_cast<TarObject *>(ctx->ctx);
  if (tar_ctx->MkDir(std::string(path), stat_info)) {
    return 0;
  }
  return -1;
}

int tar_do_symlink(struct fs_traversal_context *ctx,
              const char *src,
              const char *dest,
              const struct cvmfs_attr *stat_info) {
  TarObject *tar_ctx = reinterpret_cast<TarObject *>(ctx->ctx);
  if (tar_ctx->MkSym(std::string(src), std::string(dest), stat_info)) {
    return 0;
  }
  return -1;
}

int tar_touch(struct fs_traversal_context *ctx,
              const struct cvmfs_attr *stat_info) {
  TarObject *tar_ctx = reinterpret_cast<TarObject *>(ctx->ctx);
  if (tar_ctx->TouchDataObject(stat_info)) {
    return 0;
  }
  return -1;
}

void *tar_get_handle(struct fs_traversal_context *ctx,
              const char *identifier) {
  TarObject *tar_ctx = reinterpret_cast<TarObject *>(ctx->ctx);
  return tar_ctx->GetHandle(std::string(identifier));
}

int tar_do_fopen(void *file_ctx, fs_open_type op_mode) {
  TarDataObjectHandle *ctx
    = reinterpret_cast<TarDataObjectHandle *>(file_ctx);
  if (ctx->Open(op_mode)) {
    return 0;
  }
  return -1;
}

int tar_do_fclose(void *file_ctx) {
  TarDataObjectHandle *ctx
    = reinterpret_cast<TarDataObjectHandle *>(file_ctx);
  if (ctx->Close()) {
    return 0;
  }
  return -1;
}

int tar_do_fread(void *file_ctx, char *buff, size_t len, size_t *read_len) {
  TarDataObjectHandle *ctx
    = reinterpret_cast<TarDataObjectHandle *>(file_ctx);
  if (ctx->Read(buff, len, read_len)) {
    return 0;
  }
  return -1;
}

int tar_do_fwrite(void *file_ctx, const char *buff, size_t len) {
  TarDataObjectHandle *ctx
    = reinterpret_cast<TarDataObjectHandle *>(file_ctx);
  if (ctx->Write(buff, len)) {
    return 0;
  }
  return -1;
}

void tar_do_ffree(void *file_ctx) {
  TarDataObjectHandle *ctx
    = reinterpret_cast<TarDataObjectHandle *>(file_ctx);
  delete ctx;
}

bool tar_is_hash_consistent(struct fs_traversal_context *ctx,
                const struct cvmfs_attr *stat_info) {
  // TODO(steuber): Is this C compliant?
  TarObject *tar_ctx = reinterpret_cast<TarObject *>(ctx->ctx);
  return tar_ctx->IsHashConsistent(stat_info);
}

int tar_garbage_collector(struct fs_traversal_context *ctx) {
  // TODO(steuber): Implementation
  return -1;
}

struct fs_traversal_tar_context {
  FILE *fd;
};



struct fs_traversal_context *tar_initialize(
  const char *repo,
  const char *base,
  const char *data,
  int thread_num,
  const char *config) {
  fs_traversal_context *result = new struct fs_traversal_context;
  result->version = 1;
  result->repo = strdup(repo);
  if (base == NULL) {
    base = "/tmp/cvmfs-shrinkwrap.tar";
  }
  TarObject *destination;
  if (FileExists(base)) {
     destination = new TarObject(base);
  } else {
    destination = new TarObject();
  }
  result->ctx = destination;
  return result;
}

void tar_finalize(struct fs_traversal_context *ctx) {
  TarObject *tar_ctx = reinterpret_cast<TarObject *>(ctx->ctx);
  tar_ctx->WriteFsStructure();
  delete tar_ctx;
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
