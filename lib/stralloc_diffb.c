#include "byte.h"
#include "stralloc.h"

int stralloc_diffb(register const stralloc *sa, const void *d, register unsigned int dlen)
{
  register unsigned int len;
  int r;
/*
  get shortest len
*/
  len = sa->len;
  if(len > dlen) {
    len = dlen;
  }
/*
  compare common lengths
*/
  r = byte_diff(sa->s, len, d);
  if(r) return r;
  return (int)sa->len - (int)dlen;
}
