#include "../charbuf.h"

int
charbuf_nextc(charbuf* b, char* ch) {
  int ret;

  if((ret = charbuf_skip(b)) > 0)
    ret = charbuf_peekc(b, ch);
  return ret;
}
