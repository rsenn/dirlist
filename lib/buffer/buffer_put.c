#include "../buffer.h"
#include <string.h>
#ifdef __x86_64__
#include <x86intrin.h>
#endif

extern int buffer_stubborn(buffer_op_fn* op, int fd, const char* buf, size_t len, void* cookie);

#ifdef __dietlibc__
#undef __unlikely
#endif

#ifndef __unlikely
#define __unlikely(x) (x)
#endif

int
buffer_put(buffer* b, const char* buf, size_t len) {
  if(__unlikely(len > b->a - b->p)) { /* doesn't fit */
    if(buffer_flush(b) == -1) return -1;
    if(len > b->a) {
      if(buffer_stubborn(b->op, b->fd, buf, len, b) < 0) return -1;
      return 0;
    }
  }
  memcpy(b->x + b->p, buf, len);
  b->p += len;
  return 0;
}
