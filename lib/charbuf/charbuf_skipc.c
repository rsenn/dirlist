#include "../charbuf_internal.h"

ssize_t
charbuf_skipc(charbuf* b, uint8* ch) {
  ssize_t ret = 0;

  for(;;) {
    if(b->p) {
      if(b->chrs[0] == '\n') {
        b->loc.column = 0;
        b->loc.line++;
      } else {
        b->loc.column++;
      }
      if(ch)
        *ch = b->chrs[0];
      if(--b->p > 0)
        byte_copy(b->chrs, b->p, b->chrs + 1);
      b->offset++;
      ret = 1;

    } else if((ret = charbuf_stubborn_read(b, 1)) > 0) {
      b->p = ret;
      continue;
    }
    break;
  }

#ifdef DEBUG_CHARBUF_
  if(charbuf_debug) {
    buffer_puts(buffer_2, "charbuf_skipc ");
    charbuf_dumplabel("c", buffer_2);

    if(charbuf_colors)
      buffer_puts(buffer_2, CHARBUF_CYAN);
    charbuf_dumpchar(*ch, buffer_2);
    if(charbuf_colors)
      buffer_puts(buffer_2, CHARBUF_NC);

    charbuf_dump(b, buffer_2);
    buffer_puts(buffer_2, CHARBUF_SEP);
    charbuf_dumpint64("ret", ret, buffer_2);

    buffer_putnlflush(buffer_2);
  }
#endif
  return ret;
}
