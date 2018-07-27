#include "../io_internal.h"
#include <stdio.h>
#if defined(_WIN32) || defined(_WIN32) || defined(__MINGW64__) || defined(_WIN64)
#include <windows.h>
#else
#include <unistd.h>
#include <sys/mman.h>
#endif
#include "../open.h"
#include "../mmap.h"

extern char* mmap_read(const char* filename, size_t * filesize) {
#if defined(_WIN32) || defined(_WIN32) || defined(__MINGW64__) || defined(_WIN64)
  HANDLE fd, m;
  char* map;
  fd = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
  if(fd == INVALID_HANDLE_VALUE)
    return 0;
  m = CreateFileMapping(fd, 0, PAGE_READONLY, 0, 0, NULL);
  map = 0;
  if(m)
    if((map = MapViewOfFile(m, FILE_MAP_READ, 0, 0, 0)))
      * filesize = GetFileSize(fd, NULL);
  CloseHandle(m);
  CloseHandle(fd);
  return map;
#else
  int fd = open_read(filename);
  char* map;
  if(fd >= 0) {
    *filesize = LSEEK(fd, 0, SEEK_END);
    map = mmap(0, *filesize, PROT_READ, MAP_SHARED, fd, 0);
    if(map == (char *) - 1)
      map = 0;
    close(fd);
    return map;
  }
  return 0;
#endif
}
