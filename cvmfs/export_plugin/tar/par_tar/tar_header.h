/**
 * This file is part of the CernVM File System.
 */
#ifndef CVMFS_EXPORT_PLUGIN_TAR_PAR_TAR_TAR_HEADER_H_
#define CVMFS_EXPORT_PLUGIN_TAR_PAR_TAR_TAR_HEADER_H_

#include <stddef.h>
#include <string.h>

#include <string>

enum ustar_type {
  USTAR_REGULAR = '0',        // Ordinary file.
  USTAR_LINK = '1',           // Hard Link
  USTAR_SYMLINK = '2',         // Symlink
  USTAR_DIRECTORY = '5',      // Directory.
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

#endif  // CVMFS_EXPORT_PLUGIN_TAR_PAR_TAR_TAR_HEADER_H_
