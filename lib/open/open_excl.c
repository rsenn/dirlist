#define _FILE_OFFSET_BITS 64
#include "../open.h"
#include <fcntl.h>
#include <unistd.h>

#ifndef O_NDELAY
#define O_NDELAY 0
#endif

int
open_excl(const char* filename) {
  return open(filename, O_WRONLY | O_NDELAY | O_TRUNC | O_CREAT | O_EXCL, 0600);
}