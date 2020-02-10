#include "../http.h"

#ifdef HAVE_OPENSSL
#include <openssl/ssl.h>
#include <openssl/err.h>

ssize_t
http_ssl_write(fd_t fd, const void* buf, size_t n, void* b) {
  http* h = ((buffer*)b)->cookie;
  ssize_t ret;
  errno = 0;
  ret = SSL_write(h->ssl, buf, n);

  if(ret <= 0) {
    ret = http_ssl_error(ret, h, 0);

    if(ret == -1 && (errno == EWOULDBLOCK || errno == EAGAIN))
      return ret;
  }

  buffer_puts(buffer_2, "SSL write = ");
  buffer_putlong(buffer_2, ret);
  buffer_putnlflush(buffer_2);
  return ret;
}
#endif