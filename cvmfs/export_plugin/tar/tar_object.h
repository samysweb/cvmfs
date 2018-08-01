/**
 * This file is part of the CernVM File System.
 */
#ifndef CVMFS_EXPORT_PLUGIN_TAR_TAR_OBJECT_H_
#define CVMFS_EXPORT_PLUGIN_TAR_TAR_OBJECT_H_

#include <map>
#include <string>

#include "export_plugin/fs_traversal_interface.h"
#include "libcvmfs.h"

#include "tar_header.h"

#define WARNING_FILE_NAME "DO_NOT_MODIFY_THIS_IS_AN_AUTOGENERATED_DIRECTORY"

class TarDataObject {
 private:
  struct ustar_header *entry_;
  std::string hash_path_;
  size_t offset;
};

class TarDataObjectHandle {
 public:
  explicit TarDataObjectHandle(const TarDataObject &obj);
  bool Open(fs_open_type op_mode);
  bool Read(char *buff, size_t len, size_t *read_len);
  bool Write(const char *buff, size_t len);
  bool Close();
 private:
  TarDataObject *obj_;
  fs_open_type mode;
  int fd_;
  size_t pos_;
};

class TarNode {
 private:
  std::map<std::string, TarNode *> elements_;
  struct ustar_header entry_;
};

class TarObject {
 public:
  // Constructors
  TarObject() {
    root_ = new TarNode();
  }
  // Can be memory expensive depending on file size
  explicit TarObject(std::string path);

  void WriteFsStructure();

  // Non-Threadsafe Operations
  void ListDir(std::string dir,
    char ***buf,
    size_t *len);
  bool MkDir(
    const std::string &path,
    const struct cvmfs_attr *stat);
  bool MkLink(
    const std::string &path,
    const std::string &dest);
  bool MkSym(
    const std::string &path,
    const std::string &dest,
    const struct cvmfs_attr *stat_info);

  bool Remove(
    const std::string &path);

  bool Stat(
    const std::string &path,
    struct cvmfs_attr *stat);
  bool IsHashConsistent(const struct cvmfs_attr *stat_info);

  TarDataObject *TouchDataObject(const struct cvmfs_attr *stat);
  std::string GetIdentifier(const struct cvmfs_attr *stat);

  TarDataObjectHandle *GetHandle(const std::string path);

 private:
  int fd;
  size_t data_table_end;
  std::map<std::string, TarDataObject> data_objects_;
  TarNode *root_;
};

#endif  // CVMFS_EXPORT_PLUGIN_TAR_TAR_OBJECT_H_
