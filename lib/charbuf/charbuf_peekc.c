#include "../charbuf.h"
#include "../buffer.h"

int
charbuf_peekc(charbuf* b, char* ch) {

  int ret;
  if(b->p) {
    *ch = b->ch;
    return 1;
  }

  if(b->eof || b->err)
    return b->eof ? 0 : -1;

  if((ret = charbuf_stubborn_read(b->op,  b->fd, &b->ch, 1, b) <= 0)) {
    if(ret == 0)
      b->eof = 1;
    else if(ret < 0)
      b->err = 1;
    return ret;
  } else {
    b->p = 1;
    *ch = b->ch;
    return 1;
  }

#ifdef DEBUG_CHARBUF_
  buffer_puts(buffer_2, "charbuf_peekc '");
  buffer_putc(buffer_2, b->ch);
  buffer_puts(buffer_2, "' eof=");
  buffer_putulong(buffer_2, b->eof ? 1 : 0);
  buffer_puts(buffer_2, " err=");
  buffer_putulong(buffer_2, b->err ? 1 : 0);
  buffer_puts(buffer_2, " ret=");
  buffer_putlong(buffer_2, b->err ? 1 : 0);
  buffer_putnlflush(buffer_2);
#endif

  return ret;
}
