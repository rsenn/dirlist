#include "../errmsg.h"
#include "../errmsg_internal.h"
#include "../str.h"
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>

void
errmsg_infosys(const char* message, ...) {
  va_list a;
  va_start(a, message);
  errmsg_write(1, strerror(errno), message, a);
}
