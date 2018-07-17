/**
 * This file is part of the CernVM File System.
 */

#include <stdio.h>

#include <vector>
#include <string>

#include "util.h"

#include "hash.h"
#include "libcvmfs.h"
#include "xattr.h"

shash::Any HashMeta(const struct cvmfs_attr *stat_info) {
  // TODO(steuber): Can we do any better here?
  shash::Any meta_hash(shash::kMd5);
  unsigned min_buffer_size = sizeof(mode_t)/sizeof(unsigned char)
    + 1
    + sizeof(uid_t)/sizeof(unsigned char)
    + 1
    + sizeof(gid_t)/sizeof(unsigned char)
    + 1;
  XattrList *xlist = reinterpret_cast<XattrList *>(stat_info->cvm_xattrs);
  unsigned xlist_buffer_size = 0;
  unsigned char *xlist_buffer;
  std::vector<std::string> serialize_blacklist;
  serialize_blacklist.push_back("security.selinux");
  if (xlist) {
    xlist->Serialize(&xlist_buffer, &xlist_buffer_size, &serialize_blacklist);
  }
  unsigned char buffer[min_buffer_size+xlist_buffer_size];
  for (unsigned i = 0; i < (min_buffer_size+xlist_buffer_size); i++) {
    buffer[i] = 255;
  }
  unsigned offset = 0;
  // Add mode
  mode_t hash_mode = stat_info->st_mode & 0777;
  memcpy(buffer+offset, &hash_mode, sizeof(mode_t));
  offset+=sizeof(mode_t)/sizeof(unsigned char);
  *(buffer+offset) = 0;
  offset+=1;
  // Add uid
  memcpy(buffer+offset, &(stat_info->st_uid), sizeof(uid_t));
  offset+=sizeof(uid_t)/sizeof(unsigned char);
  *(buffer+offset) = 0;
  offset+=1;
  // Add gid
  memcpy(buffer+offset, &(stat_info->st_gid), sizeof(gid_t));
  offset+=sizeof(gid_t)/sizeof(unsigned char);
  *(buffer+offset) = 0;
  offset+=1;
  // Add xlist
  if (xlist) {
    memcpy(buffer+offset, xlist_buffer, xlist_buffer_size);
    delete xlist_buffer;
  }
  // Hash
  shash::HashMem(buffer, min_buffer_size+xlist_buffer_size, &meta_hash);
  return meta_hash;
}
