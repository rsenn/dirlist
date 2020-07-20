#include "lib/uint32.h"
#include "lib/uint64.h"
#include "lib/windoze.h"
#include "lib/buffer.h"
#include "lib/errmsg.h"
#include "lib/dir.h"
#include "lib/path.h"
#include "lib/str.h"
#include "lib/stralloc.h"
#include "lib/slist.h"
#include "lib/scan.h"
#include "lib/mmap.h"
#include "lib/seek.h"
#include "lib/unix.h"
#include "lib/open.h"
#include "lib/array.h"
#include "lib/byte.h"
#include "lib/getopt.h"

#include <ctype.h>
#include <sys/stat.h>

#if WINDOWS_NATIVE
#include <io.h>
#else
#include <unistd.h>
#endif

#ifdef __LCC__
#define stat _stat
#endif

typedef struct procfd {
  struct procfd* next;
  int32 pid;
  int32 fd;
  struct stat st;
  stralloc info;
} procfd_t;

typedef struct {
  int64 id;
  procfd_t* list;
} pipe_t;

static array pipes;
static int verbose;
static stralloc procfd, procfdinfo;

void
usage(char* av0) {
  buffer_putm_internal(buffer_1,
                       "Usage: ",
                       str_basename(av0),
                       " [OPTIONS] <FILE.list | TARGET LINK>\n",
                       "\n",
                       "Options:\n",
                       "\n",
                       "  -h, --help              Show this help\n",
                       "  -v, --verbose           Be verbose\n",
                       "\n",
                       NULL);
  buffer_flush(buffer_1);
}

pipe_t*
get_pipe(int64 id) {
  pipe_t* p;
  array_foreach(&pipes, sizeof(pipe_t), p) {
    if(p->id == id)
      return p;
  }

  p = array_allocate(&pipes, sizeof(pipe_t), array_length(&pipes, sizeof(pipe_t)));
  p->id = id;
  p->list = NULL;
  return p;
}

void
print_number_nonl(const char* property, int64 num) {

  buffer_putm_internal(buffer_1, property, "=", 0);
  buffer_putlonglong(buffer_1, num);
}
void
print_number(const char* property, int64 num) {
  print_number_nonl(property, num);
  buffer_putnlflush(buffer_1);
}

const char*
type(uint32 bits) {
  switch(bits & S_IFMT) {
    case S_IFSOCK: return "socket";
    case S_IFLNK: return "symlink";
    case S_IFREG: return "regular";
    case S_IFBLK: return "blkdev";
    case S_IFDIR: return "directory";
    case S_IFCHR: return "chardev";
    case S_IFIFO: return "fifo";
    default: return NULL;
  }
}
void
print_stat(const char* property, const struct stat* st) {
  const char* t = type(st->st_mode);
  buffer_putm_internal(buffer_1, property, ": ", 0);
  t ? buffer_puts(buffer_1, t) : buffer_put8long(buffer_1, st->st_mode & S_IFMT);

  buffer_putm_internal(buffer_1, " [ mode 0", 0);
  buffer_put8long(buffer_1, st->st_mode & 07777);
  if(st->st_dev) {
    buffer_putm_internal(buffer_1, ", dev 0x", 0);
    buffer_putxlong0(buffer_1, st->st_dev, 3);
  }
  if(st->st_rdev) {
    buffer_putm_internal(buffer_1, ", rdev 0x", 0);
    buffer_putxlong0(buffer_1, st->st_rdev, 3);
  }
  if(st->st_ino) {
    buffer_putm_internal(buffer_1, ", inode 0x", 0);
    buffer_putxlong0(buffer_1, st->st_ino, 3);
  }
  if(st->st_size) {
    buffer_putm_internal(buffer_1, ", size ", 0);
    buffer_putulonglong(buffer_1, st->st_size);
  }
  if(st->st_blocks) {
    buffer_putm_internal(buffer_1, ", blocks ", 0);
    buffer_putulonglong(buffer_1, st->st_blocks);
  }
  if(st->st_blksize) {
    buffer_putm_internal(buffer_1, ", blksize 0x", 0);
    buffer_putxlonglong(buffer_1, st->st_blksize);
  }
  buffer_puts(buffer_1, " ]");
  buffer_putnlflush(buffer_1);
}

void
print_stralloc_nonl(const char* property, const stralloc* sa) {
  buffer_putm_internal(buffer_1, property, "='", 0);
  buffer_putsa(buffer_1, sa);
  buffer_puts(buffer_1, "'");
}
void
print_stralloc(const char* property, const stralloc* sa) {
  print_stralloc_nonl(property, sa);
  buffer_putnlflush(buffer_1);
}

void
print_string(const char* property, const char* str) {
  buffer_putm_internal(buffer_1, property, ": ", str, 0);
  buffer_putnlflush(buffer_1);
}

void
proc_fd_root(int32 pid, stralloc* out) {
  stralloc_zero(out);
  stralloc_cats(out, "/proc/");
  stralloc_catulong(out, pid <= 0 ? getpid() : pid);
  stralloc_nul(out);
}

const char*
proc_subdir_path(int32 pid, const char* subdir, stralloc* out) {
  stralloc_zero(out);
  proc_fd_root(pid, out);
  stralloc_catm_internal(out, "/", subdir, "/", 0);

  return out->s;
}

const char*
proc_fd_path(int32 pid, fd_t fd, stralloc* out) {
  proc_subdir_path(pid, "fd", out);
  if(fd >= 0)
    stralloc_catulong(out, fd);
  stralloc_nul(out);
  return out->s;
}
const char*
proc_fdinfo_path(int32 pid, fd_t fd, stralloc* out) {
  proc_subdir_path(pid, "fdinfo", out);
  if(fd >= 0)
    stralloc_catulong(out, fd);
  stralloc_nul(out);
  return out->s;
}

const char*
proc_fd_str(const procfd_t* pfd, stralloc* out) {
  return proc_fd_path(pfd->pid, pfd->fd, out);
}

void
read_proc() {
  dir_t procdir, fddir;
  uint32 pid, fd, pipeId;
  int64 n;
  fd_t tmpfd;

  pipe_t* p;
  procfd_t* pfd;
  size_t len, i;
  int32 prev;
  const char *fdPath, targetPath;
  char* x;
  struct stat lst, st;

  stralloc target, real, current;
  stralloc_init(&procfd);
  stralloc_init(&real);
  stralloc_init(&target);
  stralloc_init(&current);

  const char *fdStr, *s;
  if(dir_open(&procdir, "/proc"))
    return;

  while((s = dir_read(&procdir))) {
    if(!isdigit(s[0]))
      continue;

    if(scan_uint(s, &pid) > 0) {
      fdPath = proc_fd_path(pid, -1, &procfd);

      stralloc_copys(&current, fdPath);
      stralloc_nul(&current);

      if(dir_open(&fddir, fdPath))
        continue;

      while((fdStr = dir_read(&fddir))) {
        if(!isdigit(fdStr[0]))
          continue;

        if(scan_uint(fdStr, &fd) > 0) {

          print_number("  fd", fd);
          byte_zero(&lst, sizeof(lst));
          byte_zero(&st, sizeof(st));
          fdPath = proc_fd_path(pid, fd, &procfd);

          lstat(fdPath, &lst);
          stat(fdPath, &st);

          buffer_putm_internal(buffer_1, "  path: ", fdPath, "\n", 0);

          if(lst.st_mode)
            print_stat("   lst", &lst);

          if(st.st_mode)
            print_stat("  stat", &st);

          stralloc_zero(&target);

          if(S_ISLNK(lst.st_mode))
            path_readlink(fdPath, &target);
        }

        stralloc_zero(&real);
        path_realpath(fdPath, &real, 0, &current);
        stralloc_nul(&real);

        if(stralloc_start(&real, &current))
          stralloc_remove(&real, 0, current.len);

        /*  if(!stralloc_contains(&real, "pipe"))
            continue;*/

        n = -1;
        // print_number("  tmpfd", tmpfd);
        if((tmpfd = open_read(real.s)) != -1) {
          stralloc filename;
          stralloc_init(&filename);

          x = mmap_read_fd_range(tmpfd, &len, 0, getpagesize());
          if(x) {
            n = len;
            mmap_filename(x, &filename);
            mmap_unmap(x, len);
          } else {
            errmsg_warnsys("mmap", 0);
          }
          print_stralloc("mapped", &filename);

          if(n == -1) {
            n = seek_cur(tmpfd);
            seek_end(tmpfd);
          }
          close(tmpfd);
          stralloc_free(&filename);
        }
        if(n >= 0)
          print_number("     n", n);

        if(S_ISLNK(lst.st_mode)) {
          if(!stralloc_equal(&real, &target))
            print_stralloc("  real", &real);

          if(stralloc_starts(&target, "pipe:[")) {
            pipeId = -1;
            scan_uint(target.s + 6, &pipeId);
            print_number("  pipe", pipeId);
          }

          { print_stralloc("target", &target); }
        }

        if(S_ISFIFO(st.st_mode)) {
          p = get_pipe(pipeId);

          pfd = alloc_zero(sizeof(procfd_t));

          slist_add((void*)&p->list, (void*)pfd);

          pfd->pid = pid;
          pfd->fd = fd;

          byte_copy(&pfd->st, sizeof(struct stat), &st);

          // stralloc_copy(&pfd->path, &real);

          prev = '\0';
          openreadclose(proc_fdinfo_path(pid, fd, &procfdinfo), &pfd->info, 1024);
          /* if((n = pfd->info.len)) {
             for(x = stralloc_begin(&pfd->info); n > 0; x++, n--) {
               if(*x != '\n' && (i = scan_whitenskip(x, n)) && i >= 1)
                 x += i, n -= i;

               *x == '\n' ? buffer_puts(buffer_1, "\\n\n") : buffer_putc(buffer_1, *x);
               prev = *x;
             }
             buffer_putnlflush(buffer_1);
           }

 */
        }
        buffer_putnlflush(buffer_1);
      }
    }
    buffer_putnlflush(buffer_1);
  }
  stralloc_free(&procfd);
  stralloc_free(&real);
  stralloc_free(&target);
  stralloc_free(&current);
}

int
compare_pipes(const pipe_t* a, const pipe_t* b) {
  if(a->id != b->id)
    return a->id - b->id;

  return 0;
}
void
procfd_dump(const procfd_t* pfd) {
  print_number_nonl(" fd", pfd->fd);
  // print_stralloc_nonl(" path", &pfd->path);
  buffer_puts(buffer_1, " st_mode: ");
  buffer_putxlong(buffer_1, pfd->st.st_mode);
  print_number_nonl(" st_size", pfd->st.st_size);
  print_number_nonl(" st_dev", pfd->st.st_dev);
  print_number_nonl(" st_uid", pfd->st.st_uid);
  print_number_nonl(" st_gid", pfd->st.st_gid);
  print_number_nonl(" st_atime", pfd->st.st_atim.tv_sec);
  if(pfd->st.st_atim.tv_sec != pfd->st.st_mtim.tv_sec)
    print_number_nonl(" st_mtime", pfd->st.st_mtim.tv_sec);
  if(pfd->st.st_mtim.tv_sec != pfd->st.st_ctim.tv_sec)
    print_number_nonl(" st_ctime", pfd->st.st_ctim.tv_sec);
  buffer_puts(buffer_1, " info: ");
  buffer_put_escaped(buffer_1, pfd->info.s, pfd->info.len);
  //      print_stralloc(" info", &pfd->info);
}

int
main(int argc, char* argv[]) {
  int index = 0, c, i, prev;
  const pipe_t* p;
  static const struct longopt opts[] = {{"help", 0, NULL, 'h'}, {"verbose", 0, 0, 'v'}, {0, 0, 0, 0}};

  errmsg_iam(argv[0]);

  for(;;) {
    c = getopt_long(argc, argv, "hv", opts, &index);
    if(c == -1)
      break;
    if(c == '\0')
      continue;

    switch(c) {
      case 'h': usage(argv[0]); return 0;
      case 'v': verbose = 1; break;
      default: {
        usage(argv[0]);
        return 1;
      }
    }
  }

  if(argc <= 1) {
    argv[optind] = "-";
    argc++;
  }

  read_proc();
  qsort(array_start(&pipes), array_length(&pipes, sizeof(pipe_t)), sizeof(pipe_t), &compare_pipes);

  if(verbose) {
    i = 0;
    array_foreach(&pipes, sizeof(pipe_t), p) {
      const procfd_t* pfd;
      print_number("i", i);
      print_number_nonl("pipe#", p->id);

      prev = -1;

      slist_foreach(p->list, pfd) {
        buffer_putc(buffer_1, '\n');
        if(prev != pfd->pid)
          print_number_nonl("  pid", pfd->pid);
        else
          buffer_puts(buffer_1, "           ");
        procfd_dump(pfd);
        prev = pfd->pid;
      }

      buffer_putnlflush(buffer_1);
      i++;
    }
  }

  while(optind < argc) {
    const char* a = argv[optind++];
    fd_t fd = str_equal(a, "-") ? 0 : open_rw(a);

    buffer_putm_internal(buffer_1, "arg: '", a, "'", 0);
    buffer_putnlflush(buffer_1);
    struct stat st;

    if(fstat(fd, &st) != -1) {
      const char* fd_path = proc_fd_path(-1, fd, &procfd);
      stralloc target;
      stralloc_init(&target);

      path_readlink(fd_path, &target);

      print_string("fd_path", fd_path);
      print_stralloc("target", &target);

      print_number("fd", fd);
      print_number("tar", fd);
      print_number("size", st.st_size);
      print_number("blksize", st.st_blksize);
      print_number("blocks", st.st_blocks);
    }
  }
}