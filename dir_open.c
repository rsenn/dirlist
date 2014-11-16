#include "dir_internal.h"

int dir_open(struct dir_s *d, const char *p)
{
  char path[PATH_MAX+1];
  size_t len;
  int ret;
  if(!(d->dir_int = malloc(sizeof(struct dir_internal_s))))
  return 1;
#if defined(WIN32) || defined(__MINGW32__) || defined(__MSYS__)

  strncpy(path, p, sizeof(path)-1);
  path[PATH_MAX]='\0';
  len = strlen(path);
  strncat(path, (len > 0 && (path[len -1] == '\\' || path[len - 1] == '/')) ? "*" : "\\*", sizeof(path)-1);
  
  dir_INTERNAL(d)->dir_handle = FindFirstFileA(path, &dir_INTERNAL(d)->dir_finddata);
  dir_INTERNAL(d)->first = 1;
  ret = (dir_INTERNAL(d)->dir_handle ==  INVALID_HANDLE_VALUE);
#else
  ret = !(dir_INTERNAL(d)->dir_handle = opendir(p));
#endif
return ret;
}
