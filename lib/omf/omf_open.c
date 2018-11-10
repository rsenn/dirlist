#include "../omf.h"

int
omf_open(omf_file* omf, const char* filename) {
  if((omf->map = mmap_read(filename, &omf->size)) == 0)
    return 0;

  omf->record = omf->map;

  return 1;
}
