#include "../path_internal.h"
#include "../byte.h"

size_t
path_collapse(char* path, size_t n) {
  char* x = path;
  int ret = 0;
  char sep = path_getsep(path);
  char* end = x + n;

  while(x < end) {
    size_t i = byte_chr(x, end - x, sep);

    if(x + i < end) {
      i++;

      if(x + i + 2 < end) {
        if(x[i] == '.' && x[i + 1] == '.' && (x + i + 2 == end || x[i + 2] == sep)) {
          i += 3;
          byte_copy(x, n - i, &x[i]);
          end -= i;
          ret++;
          continue;
        }
      }
    }

    x += i;
    n -= i;
  }
  return x - path;
}