#include "stralloc.h"
#undef stralloc_catlong

unsigned int stralloc_catlong(stralloc *sa, long l)
{
  return stralloc_catlong0(sa, l, 0);
}