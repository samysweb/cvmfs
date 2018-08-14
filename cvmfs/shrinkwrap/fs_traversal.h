/**
 * This file is part of the CernVM File System.
 */

#ifndef CVMFS_SHRINKWRAP_FS_TRAVERSAL_H_
#define CVMFS_SHRINKWRAP_FS_TRAVERSAL_H_

#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>

#include "fs_traversal.h"
#include "fs_traversal_interface.h"
#include "statistics.h"

#define SHRINKWRAP_STAT_BYTE_COUNT "byteCnt"
#define SHRINKWRAP_STAT_FILE_COUNT "fileCnt"
#define SHRINKWRAP_STAT_SRC_ENTRIES "srcEntries"
#define SHRINKWRAP_STAT_DEST_ENTRIES "destEntries"
#define SHRINKWRAP_STAT_DEDUPED_FILES "dedupedFiles"
#define SHRINKWRAP_STAT_DEDUPED_BYTES "dedupedBytes"

#define MAX_JOB_QUEUE 4096

using namespace std; // NOLINT

namespace shrinkwrap {

class RecDir {
 public:
  RecDir()
    : dir(NULL)
    , recursive(false) {}
  RecDir(const RecDir &other) {
    recursive = other.recursive;
    dir = strdup(other.dir);
  }

  RecDir(const char *dir, bool recursive)
    : dir(strdup(dir))
    , recursive(recursive) {}

  ~RecDir() {
    free(dir);
  }

  char *dir;
  bool recursive;
};

struct fs_traversal* FindInterface(const char * type);

int SyncInit(struct fs_traversal *src,
             struct fs_traversal *dest,
             const char *base,
             const char *spec,
             unsigned parallel,
             unsigned retries);

int GarbageCollect(struct fs_traversal *fs);

// Public for testing
bool SyncFull(
  struct fs_traversal *src,
  struct fs_traversal *dest,
  perf::Statistics *pstats,
  vector<RecDir*> *dirs_);

// Exported for testing purposes:
perf::Statistics *GetSyncStatTemplate();

}  // namespace shrinkwrap

#endif  // CVMFS_SHRINKWRAP_FS_TRAVERSAL_H_
