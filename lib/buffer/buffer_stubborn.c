#include "../buffer.h"
#include <errno.h>

int
buffer_stubborn(ssize_t (*op)(intptr_t fd, void* buf, size_t len, void* arg), intptr_t fd, const char* buf, size_t len, void* cookie) {
  ssize_t w;
  errno = 0;
  while(len) {
    if((w = op(fd, (void*)buf, len, cookie)) <= 0) {
      if(errno == EINTR) continue;
      return -1;
    }
    buf += w;
    len -= (size_t)w;
  }
  return 0;
}
