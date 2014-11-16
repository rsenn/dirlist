#ifndef _DIR_INTERNAL_H__
#define _DIR_INTERNAL_H__

#include "dir.h"

#define _POSIX_ 1

#ifdef _MSC_VER
#include <crtdefs.h>
#endif

#if defined(WIN32) || defined(__MINGW32__) || defined(__MSYS__)
#include <windows.h>
#include <limits.h>
#else
#include <dirent.h>
#endif

#include <stddef.h>
#include <stdlib.h>
#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX _MAX_PATH
#endif

struct dir_internal_s {
#if defined(WIN32) || defined(__MINGW32__) || defined(__MSYS__)
  HANDLE dir_handle;
  WIN32_FIND_DATAA dir_finddata;
  int first;
#else
  DIR *dir_handle;
  struct dirent *dir_entry;
#endif

};

#define dir_INTERNAL(d) ((struct dir_internal_s *)((d)->dir_int))

#if defined(WIN32) || defined(__MINGW32__) || defined(__MSYS__)
#define dir_NAME(d) ((d)->dir_finddata.szName)
#else
#define dir_NAME(d) ((d)->dir_entry->d_name)
#endif

#if defined(WIN32) || defined(__MINGW32__) || defined(__MSYS__)
#define dir_ATTRS(d) ((d)->dir_finddata.dwFileAttributes)
#else
#define dir_ATTRS(d) ((d)->dir_entry->st_mode)
#endif

#if defined(WIN32) || defined(__MINGW32__) || defined(__MSYS__)
#define dir_ISDIR(d) (!!(dir_ATTRS(d)&0x10))
#else
#define dir_ISDIR(d) S_ISDIR(dir_ATTRS(d))
#endif

#if defined(WIN32) || defined(__MINGW32__) || defined(__MSYS__)
#define dir_ISLINK(d) (!!(dir_ATTRS(d)&0x08))
#else
#define dir_ISLINK(d) S_ISLINK(dir_ATTRS(d))
#endif

#if defined(WIN32) || defined(__MINGW32__) || defined(__MSYS__)
#define dir_ISLINK(d) (!!(dir_ATTRS(d)&0x08))
#else
#define dir_ISLINK(d) S_ISLINK(dir_ATTRS(d))
#endif

#endif // _DIR_INTERNAL_H__
