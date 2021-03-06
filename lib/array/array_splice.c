#include "../array.h"
#include "../byte.h"
#include "../uint64.h"
#include "../safemult.h"

int64
array_splice(array* a, uint64 membersize, uint64 start, uint64 del, uint64 insert, const void* x) {
  char* s = a->p;
  uint64 i, len, newlen;

  len = array_length(a, membersize);

  if(start + del > len)
    del = len - start;

  newlen = len - del + insert;

  if(insert) {
    if(!umult64(membersize, insert, &i))
      return -1;
  }

  if(insert != del) {
    size_t bytes;
    char* newend;
    s = (char*)array_get(a, membersize, start + del);
    bytes = a->p + a->initialized - s;
    newend = (char*)array_allocate(a, membersize, newlen);

    if(bytes) {
      s = array_get(a, membersize, start + del);

      (insert > del ? byte_copyr : byte_copy)(newend - bytes, bytes, s);
    }
  }

  if(insert)
    byte_copy(array_get(a, membersize, start), i, x);

  array_truncate(a, membersize, newlen);

  return newlen;
}
