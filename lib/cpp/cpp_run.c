#include "../cpp_internal.h"

int
cpp_run(cpp_t* cpp, buffer* in, buffer* out, const char* inname) {
  return cpp_parse_file(cpp, in, inname, out);
}
