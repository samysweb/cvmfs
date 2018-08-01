/**
 * This file is part of the CernVM File System.
 */
#include <string>

#include "tar_helper.h"

/*bool SetupBasicTarFile(std::string path) {
  // TODO(steuber): Make more robust (error handling)
  int res = 0;
  FILE *tar_file = fopen(path.c_str(), "w");
  if (tar_file == NULL) {
    return -1;
  }
  struct ustar_header cvmfs_dir = BuildHeader(
    "cvmfs",
    0755,
    0,
    0,
    0,
    time(NULL),
    USTAR_DIRECTORY,
    "root",
    "root");
  res = fwrite(&cvmfs_dir, sizeof(struct ustar_header), 1, tar_file);
  if (res != sizeof(struct ustar_header)) {
    // Error during writing
    fclose(tar_file);
    return -1;
  }
  struct ustar_header data_dir = BuildHeader(
    "cvmfs/.data",
    0755,
    0,
    0,
    0,
    time(NULL),
    kUstarDir,
    "root",
    "root");
  fwrite(&data_dir, sizeof(struct ustar_header), 1, tar_file);
  fclose(tar_file);
  printf("%s", path.c_str());
}*/

#include "tar_header.h"
