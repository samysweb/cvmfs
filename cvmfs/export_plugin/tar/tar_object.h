/**
 * This file is part of the CernVM File System.
 */
#ifndef CVMFS_EXPORT_PLUGIN_TAR_TAR_OBJECT_H_
#define CVMFS_EXPORT_PLUGIN_TAR_TAR_OBJECT_H_

#include <map>
#include <string>

#include "libcvmfs.h"

#include "tar_header.h"

#define WARNING_FILE_NAME "DO_NOT_MODIFY_THIS_IS_AN_AUTOGENERATED_DIRECTORY"

class TarObject {
 public:
  // Constructors
  TarObject();
  // Can be memory expensive depending on file size
  explicit TarObject(std::string path);

  // Non-Threadsafe Operations
  bool MkDir(
    const std::string &path,
    const struct cvmfs_attr &stat);
  bool RmDir(
    const std::string &path,
    const struct cvmfs_attr &stat);
  bool MkLink(
    const std::string &path,
    const TarDataObject &link_object);
  bool RmLink(
    const std::string &path,
    const TarDataObject &link_object);
  bool MkSym(
    const std::string &path,
    const std::string &dest);
  bool Stat(
    const std::string &path,
    struct cvmfs_attr *stat);
  TarDataObject *TouchDataObject(struct cvmfs_attr stat);

 private:
  std::map<std::string, TarDataObject> data_objects_;
  TarNode root_;
};

class TarDataObject {
 private:
  struct ustar_header entry_;
  std::string hash_path_;
  size_t offset;
};

class TarNode {
 private:
  std::map<std::string, TarNode *> elements_;
  struct ustar_header entry_;
};

#endif  // CVMFS_EXPORT_PLUGIN_TAR_TAR_OBJECT_H_
