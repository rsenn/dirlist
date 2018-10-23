#include "../readlink.h"
#include "../windoze.h"

#define _MISC_SOURCE 1
#define _GNU_SOURCE 1
#define _POSIX_SOURCE 1
#define _POSIX_C_SOURCE 1
#define _XOPEN_SOURCE 1
#define _FILE_OFFSET_BITS 64
//#define _LARGEFILE64_SOURCE 1
#define _LARGEFILE_SOURCE 1

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>

#include "../buffer.h"
#include "../byte.h"
#include "../path_internal.h"
#include "../str.h"

#if !WINDOWS_NATIVE
#define HAVE_LSTAT 1
#endif

#ifndef HAVE_LSTAT
#define lstat stat
#endif

#if WINDOWS
int is_symlink(const char*);
static int
is_link(const char* path) {
  if(is_symlink(path)) return 1;
#ifdef HAVE_LSTAT
  {
    struct stat st;
    if(lstat(path, &st) != -1) return S_ISLNK(st.st_mode);
  }
#endif
  return 0;
}
#elif defined(HAVE_LSTAT)
static int
is_link(const char* path) {
  struct stat st;
  if(lstat(path, &st) == -1) return 0;
  return S_ISLNK(st.st_mode);
}
#endif

//#define lstat lstat64

#define issep(c) ((c) == '/' || (c) == '\\')

/* canonicalizes a <path> and puts it into <sa>
 *
 * <path>, without trailing '\0', should not be longer than PATH_MAX or it
 * is truncated!
 *
 * if symbolic is zero then it reads symlinks and puts the physical
 * path into the destination buffer
 *
 * returns zero on error and 1 if the whole path has no symlink,
 * so the return value - 1 is the count of symlinks
 *
 * the <path> should be absolute though it will work on relative paths,
 * but they should be relative to the current dir and path_canonicalize()
 * will again return a relative path.
 * because of that the behaviour of this function differs from usual path
 * canonicalizing functions like realpath() in libc, but there is a
 * path_realpath() function which provides similar behaviour and will
 * resolve relative paths to absolute ones.
 * ----------------------------------------------------------------------- */
int
path_canonicalize(const char* path, stralloc* sa, int symbolic) {
  size_t l1, l2;
  size_t n;
  struct stat st;
  int ret = 1;
  int (*stat_fn)() = stat;
  char buf[PATH_MAX + 1];
#ifdef HAVE_LSTAT

#if !WINDOWS_NATIVE
  if(symbolic) stat_fn = lstat;
#endif
#endif
  if(path_issep(*path)) {
    stralloc_catc(sa, PATHSEP_C);
    path++;
  }

start:
  /* loop once for every /path/component/
     we canonicalize absolute paths, so we must always have a '/' here */
  while(*path) {
    while(path_issep(*path)) path++;

    /* check for various relative directory parts beginning with '.' */
    if(path[0] == '.') {
      /* strip any "./" inside the path or a trailing "." */
      if(path_issep(path[1]) || path[1] == '\0') {
        path++;
        continue;
      }

      /* if we have ".." we have to truncate the resulting path */
      if(path[1] == '.' && (path_issep(path[2]) || path[2] == '\0')) {
        sa->len = path_right(sa->s, sa->len);
        path += 2;
        continue;
      }
    }

    /* exit now if we're done */
    if(*path == '\0') break;

    /* begin a new path component */
    if(sa->len && (sa->s[sa->len - 1] != '/' && sa->s[sa->len - 1] != '\\')) stralloc_catc(sa, PATHSEP_C);

    /* look for the next path separator and then copy the component */
    n = path_len_s(path);
    stralloc_catb(sa, path, n);
    if(n == 2 && path[1] == ':') stralloc_catc(sa, PATHSEP_C);
    stralloc_nul(sa);

    path += n;

    /* now stat() the thing to verify it */
    byte_zero(&st, sizeof(st));
    if(stat_fn(sa->s, &st) == -1) return 0;

    /* is it a symbolic link? */
    if(is_link(sa->s)) {
      ret++;

      /* read the link, return if failed and then nul-terminate the buffer */
      if((n = readlink(sa->s, buf, PATH_MAX)) == -1) return 0;

      // buf[n] = '\0';

      /* if the symlink is absolute we clear the stralloc,
         set the path to buf and repeat the whole procedure */
      if(path_issep(buf[0])) {
        str_copyn(&buf[n], path, PATH_MAX - n);
        stralloc_zero(sa);
        stralloc_catc(sa, PATHSEP_C);

        path = buf;
        goto start;

        /* if the symlink is relative we remove the symlink path
         component and recurse */
      } else {
        //        size_t l = path_len_s(buf);
        //
        sa->len = path_right(sa->s, sa->len);
        //
        //        stralloc_catc(sa, PATHSEP_C);
        //        stralloc_catb(sa, buf, l);
        //        stralloc_nul(sa);

        buf[n] = '\0';
        // str_copyn(buf, buf+l+1, sizeof(buf));

        // stralloc_zero(sa);

        buffer_puts(buffer_2, "recursive path_canonicalize(\"");
        buffer_puts(buffer_2, buf);
        buffer_puts(buffer_2, "\", \"");
        buffer_putsa(buffer_2, sa);
        buffer_puts(buffer_2, "\", ");
        buffer_putlong(buffer_2, symbolic);
        buffer_puts(buffer_2, ")");
        buffer_putnlflush(buffer_2);

        if(!path_canonicalize(buf, sa, symbolic)) return 0;
      }
    }

#ifdef S_ISDIR
    /* it isn't a directory :( */
    if(!S_ISDIR(st.st_mode)) {
      errno = ENOTDIR;
      return 0;
    }
#endif
  }

  if(sa->len == 0) stralloc_catc(sa, PATHSEP_C);

  return ret;
}