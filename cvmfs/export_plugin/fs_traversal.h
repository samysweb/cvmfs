/**
 * This file is part of the CernVM File System.
 */

#ifndef CVMFS_EXPORT_PLUGIN_FS_TRAVERSAL_H_
#define CVMFS_EXPORT_PLUGIN_FS_TRAVERSAL_H_

#include <string>

#include "fs_traversal.h"
#include "fs_traversal_interface.h"
#include "statistics.h"

#define SHRINKWRAP_STAT_BYTE_COUNT "byteCnt"
#define SHRINKWRAP_STAT_FILE_COUNT "fileCnt"
#define SHRINKWRAP_STAT_SRC_ENTRIES "srcEntries"
#define SHRINKWRAP_STAT_DEST_ENTRIES "destEntries"
#define SHRINKWRAP_STAT_LAST_PRINT "timeOfLastPrint"

namespace shrinkwrap {

struct fs_traversal* FindInterface(const char * type);

int SyncInit(struct fs_traversal *src,
             struct fs_traversal *dest,
             const char *base,
             const char *spec,
             unsigned parallel,
             unsigned retries);

int GarbageCollect(struct fs_traversal *fs);

// Exported for testing purposes:
bool Sync(const char *dir,
          struct fs_traversal *src,
          struct fs_traversal *dest,
          bool recursive,
          perf::Statistics *pstats);

// Exported for testing purposes:
perf::Statistics *GetSyncStatTemplate();

}  // namespace shrinkwrap

#endif  // CVMFS_EXPORT_PLUGIN_FS_TRAVERSAL_H_
