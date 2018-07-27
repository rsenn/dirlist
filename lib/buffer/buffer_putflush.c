#if !(defined(_WIN32) || defined(_WIN64))
#include <errno.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#endif
#include "../buffer.h"

#ifdef __dietlibc__
#undef __unlikely
#endif

#ifndef __unlikely
#define __unlikely(x) (x)
#endif

extern ssize_t buffer_stubborn(buffer_op_fn* op, int fd, const char* buf, size_t len, void* cookie);

int
buffer_putflush(buffer* b, const char* x, size_t len) {
  /* Since we know we are going to flush anyway, let's see if we can
   * optimize a bit */
  if(!b->p) /* if the buffer is empty, just call buffer_stubborn directly */
    return buffer_stubborn(b->op, b->fd, x, len, b);
#if !(defined(_WIN32) || defined(_WIN64))
  if(b->op == (buffer_op_fn*)&write) {
    struct iovec v[2];
    ssize_t w;
    size_t cl = b->p + len;
    v[0].iov_base = b->x;
    v[0].iov_len = b->p;
    v[1].iov_base = (char*)x;
    v[1].iov_len = len;
    while((w = writev(b->fd, v, 2)) < 0) {
      if(errno == EINTR) continue;
      return -1;
    }
    if(__unlikely((size_t)w != cl)) {
      /* partial write. ugh. */
      if((size_t)w < v[0].iov_len) {
        if(buffer_stubborn(b->op, b->fd, v[0].iov_base + w, v[0].iov_len - w, b) ||

            buffer_stubborn(b->op, b->fd, v[1].iov_base, v[0].iov_len, b))
          return -1;
      } else {
        w -= v[0].iov_len;
        return buffer_stubborn(b->op, b->fd, v[1].iov_base + w, v[1].iov_len - w, b);
      }
    }
    b->p = 0;
    return 0;
  }
#endif
  if(buffer_put(b, x, len) < 0) return -1;
  if(buffer_flush(b) < 0) return -1;
  return 0;
}
