#include "byte.h"
#include "stdint.h"

unsigned int
byte_case_start(register const void* x1, unsigned int len, register const void* x2) {
  register const unsigned char* s;
  register const unsigned char* t;
  register int x;
  register int y;
  s = x1;
  t = x2;
  for(;;) {
    y = *t++;
    if(y >= 'A' && y <= 'Z') { /* upper case */
      y += 'a' - 'A';
    }
    if(!y)
      return 1;
    if(!len)
      return 0;
    --len;
    x = *s++;
    if(x >= 'A' && x <= 'Z') { /* upper case */
      x += 'a' - 'A';
    }
    if(x != y)
      return 0;
  }
}
