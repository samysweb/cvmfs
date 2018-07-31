/**
 * This file is part of the CernVM File System.
 */
#ifndef CVMFS_EXPORT_PLUGIN_TAR_PAR_TAR_PAR_TAR_H_
#define CVMFS_EXPORT_PLUGIN_TAR_PAR_TAR_PAR_TAR_H_


#include <stddef.h>
#include <sys/types.h>


/**
 * All functions are threadsafe/atomic
 */

/**
 * @param[in]   path  Path to the tar file (needs not to exist)
 * @param[in]   mode  Mode of opening (either 'r' for readonly or 'm' for modify)
 */
struct par_tar_handle *topen(char *path, char mode);

int tclose(const struct par_tar_handle *handle);


/**
 * IN MEMORY
 */
int tmkdir(const struct par_tar_handle *handle, const char *path);

int trmdir(const struct par_tar_handle *handle, const char *path);

int ttouch(const struct par_tar_handle *handle,
  const char *path,
  const struct stat *buf,
  char *ident);

int tlink(const struct par_tar_handle *handle,
  const char *link_path,
  const char *ident);

int tunlink(const struct par_tar_handle *handle,
  const char *link_path);

int tsymlink(const struct par_tar_handle *handle,
  const char *src_path,
  const char *dest_path);

int tstat(const struct par_tar_handle *handle,
  const char *path,
  struct stat *buf);

/**
 * Parallel reads possible, only one writer
 */
struct par_tar_fhandle *tfopen(char *path, char mode);

/**
 * Frees handle
 */
int tfclose(struct par_tar_fhandle *handle);

int tfseek(struct par_tar_fhandle *handle, long offset, int whence);

long tftell(struct par_tar_fhandle *handle);


/**
 * Underlying FS interaction
 */
int tfread(void *buf,
  const size_t size,
  const size_t nmemb,
  size_t size_read,
  struct par_tar_fhandle *handle);

int tfwrite(const void *buf,
  const size_t size,
  size_t size_written,
  struct par_tar_fhandle *handle);

#endif  // CVMFS_EXPORT_PLUGIN_TAR_PAR_TAR_PAR_TAR_H_
