#include "fs_traversal_tar.h"

#include <stdio.h>
#include <string.h>

#include <string>

#include "fs_traversal_interface.h"
#include "util/posix.h"

// Some tar inspirations from https://github.com/LucasSeveryn/pintOS/blob/master/src/lib/ustar.c

enum ustar_type {
  USTAR_DIRECTORY = '5',      // Directory.
  USTAR_REGULAR = '0',        // Ordinary file.
};

/* Header for ustar-format tar archive.  See man pages */
struct ustar_header {
    char name[100];             // File name.  Null-terminated if room.
    char mode[8];               // Permissions as octal string.
    char uid[8];                // User ID as octal string.
    char gid[8];                // Group ID as octal string.
    char size[12];              // File size in bytes as octal string.
    char mtime[12];             // Modification time in seconds
                                // from Jan 1, 1970, as octal string.
    char chksum[8];             // Sum of octets in header as octal string.
    char typeflag;              // An enum ustar_type value.
    char linkname[100];         // Name of link target.
                                // Null-terminated if room.
    char magic[6];              // "ustar\0"
    char version[2];            // "00"
    char uname[32];             // User name, always null-terminated.
    char gname[32];             // Group name, always null-terminated.
    char devmajor[8];           // Device major number as octal string.
    char devminor[8];           // Device minor number as octal string.
    char prefix[155];           // Prefix to file name.
                                // Null-terminated if room.
    char padding[12];           // Pad to 512 bytes. */
} __attribute__((__packed__));

unsigned int CalculateChecksum(const struct ustar_header *h)
{
  const uint8_t *header = (const uint8_t *) h;
  unsigned int chksum = 0;
  size_t i;
  const size_t chksum_start = offsetof(struct ustar_header, chksum);
  const size_t chksum_end = chksum_start + sizeof h->chksum;
  for (i = 0; i < sizeof(struct ustar_header); i++) {
      /* The ustar checksum is calculated as if the chksum field
         were all spaces. */
      chksum += (i >= chksum_start && i < chksum_end) ? ' ' : header[i];
  }
  return chksum;
}

struct ustar_header BuildHeader(
  std::string name,
  mode_t mode,
  uid_t uid,
  gid_t gid,
  size_t size,
  time_t mtime,
  ustar_type typeflag,
  char uname[32],
  char gname[32]) {
  struct ustar_header result;
  // TODO(steuber): Split name
  memcpy(result.name, name.c_str(), name.length()+1);
  snprintf(result.mode, sizeof(result.mode), "%07o", mode);
  snprintf(result.uid, sizeof(result.uid), "%07o", uid);
  snprintf(result.gid, sizeof(result.gid), "%07o", gid);
  snprintf(result.size, sizeof(result.size), "%11o", size);
  snprintf(result.mtime, sizeof(result.mtime), "%11o", mtime);
  result.typeflag = typeflag;

  snprintf(result.magic, sizeof(result.magic), "ustar");
  snprintf(result.uname, sizeof(result.uname), uname);
  snprintf(result.gname, sizeof(result.gname), gname);
 

  snprintf(result.chksum, sizeof(result.chksum), "%06o",
    CalculateChecksum(&result));

  return result;
}

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