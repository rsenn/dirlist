#include "../byte.h"
#include <stdint.h>

unsigned int
byte_case_start(const void* x1, unsigned int len, const void* x2) {
  const unsigned char* s;
  const unsigned char* t;
  int x;
  int y;
  s = x1;
  t = x2;
  for(;;) {
    y = *t++;
    if(y >= 'A' && y <= 'Z') { /* upper case */
      y += 'a' - 'A';
    }
    if(!y) return 1;
    if(!len) return 0;
    --len;
    x = *s++;
    if(x >= 'A' && x <= 'Z') { /* upper case */
      x += 'a' - 'A';
    }
    if(x != y) return 0;
  }
}