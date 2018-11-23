#include "../byte.h"
#include "../stralloc.h"

void
stralloc_trim(stralloc* sa, const char* trimchars, unsigned int trimcharslen) {
  if(!sa->s)
    return;
  while(sa->len > 0) {
    if(byte_chr(trimchars, trimcharslen, sa->s[sa->len - 1]) == trimcharslen)
      return;
    --sa->len;
  }
}
