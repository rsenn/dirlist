#include "byte.h"
#include "buffer.h"

/* skips bytes in buffer until a char in charset occurs, the char itself will be skipped also */
int buffer_skip_until(buffer* b, const char* charset, size_t setlen) {
  int r;
  int n = 0;
  for(;;) {
    if(b->p == b->n)
      if((r = buffer_feed(b)) <= 0)
        return r;    
    n++;
    if(byte_chr(charset, setlen, b->x[b->p++]) < setlen) break;
  }
  return n;
}
