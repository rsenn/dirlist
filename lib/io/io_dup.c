#include "../windoze.h"
#include "../io.h"
#if WINDOWS_NATIVE
#include <windows.h>
#else
#include <unistd.h>
#endif

int64
io_dup(int64 fd) {
#if WINDOWS_NATIVE
  HANDLE h = -1;

  DuplicateHandle(GetCurrentProcess(), 
                    (HANDLE)fd, 
                    GetCurrentProcess(),
                    &h, 
                    0,
                    FALSE,
                    DUPLICATE_SAME_ACCESS);
  return h;
#else
  return dup(fd);
#endif
}
