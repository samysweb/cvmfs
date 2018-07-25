#include "export_plugin/fs_traversal.h"
#include "export_plugin/fs_traversal_tar.h"


int main() {
  struct fs_traversal *interface = tar_get_interface();
  struct fs_traversal_context *res = interface->initialize("sft.cern.ch", NULL, NULL, NULL);
  __asm__ __volatile__("" :: "m" (res));
  return 0;
}
