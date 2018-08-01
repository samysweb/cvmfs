/**
 * This file is part of the CernVM File System.
 */
#include "tar_object.h"

TarObject::TarObject(std::string path) {
  FILE *f = fopen(path.c_str(), "r");
  int empty_blocks = 0;
  struct ustar_header cur_header;
  while (empty_blocks < 2) {
    size_t read = fread(&cur_header, sizeof(cur_header), 1, f);
    if (read < sizeof(cur_header)) {
      break;
    }
  }
}
