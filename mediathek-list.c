#define _XOPEN_SOURCE 1
#define __POSIX_VISIBLE 199209
#define __MISC_VISIBLE 1
#include "lib/buffer.h"
#include "lib/byte.h"
#include "lib/fmt.h"
#include "lib/unix.h"
#include "lib/http.h"
#include "lib/io_internal.h"
#include "lib/iarray.h"
#include "lib/scan.h"
#include "lib/slist.h"
#include "lib/str.h"
#include "lib/stralloc.h"
#include "lib/strarray.h"
#include "lib/strlist.h"
#include "lib/windoze.h"
#include "lib/errmsg.h"
#include "lib/unix.h"

#if !defined(_WIN32) && !(defined(__MSYS__) && __MSYS__ == 1)
#include <libgen.h>
#endif

#include <string.h>
#include <time.h>

#if WINDOWS_NATIVE
#include <io.h>
#include <stdio.h>
#else
#include <unistd.h>
#endif

#define BUFSIZE 65535

#if __BORLANDC__
#undef popen
void* __declspec(dllimport) popen(const char*, const char*);

#define popen __popen_tmp
#include <stdio.h>
#undef popen
#define popen _popen
#else
#include <stdio.h>
#undef _popen
#endif

extern char strlist_dumpx[5];

static const char* dt_fmt = "%Y%m%d %H:%M";

#include "lib/http.h"

/*
http://download10.onlinetvrecorder.com/mediathekview/Filmliste-akt.xz
http://mediathekview.jankal.me/Filmliste-akt.xz
http://verteiler1.mediathekview.de/Filmliste-akt.xz
http://verteiler2.mediathekview.de/Filmliste-akt.xz
http://verteiler3.mediathekview.de/Filmliste-akt.xz
http://verteiler4.mediathekview.de/Filmliste-akt.xz
http://verteiler5.mediathekview.de/Filmliste-akt.xz
http://verteiler6.mediathekview.de/Filmliste-akt.xz
*/
const char* const mediathek_urls[] = {
    "http://download10.onlinetvrecorder.com/mediathekview/Filmliste-akt.xz",
    "http://mediathekview.jankal.me/Filmliste-akt.xz",
    "http://verteiler1.mediathekview.de/Filmliste-akt.xz",
    "http://verteiler2.mediathekview.de/Filmliste-akt.xz",
    "http://verteiler3.mediathekview.de/Filmliste-akt.xz",
    "http://verteiler4.mediathekview.de/Filmliste-akt.xz",
    "http://verteiler5.mediathekview.de/Filmliste-akt.xz",
    "http://verteiler6.mediathekview.de/Filmliste-akt.xz",
};

const char* const mediathek_url = "http://verteiler1.mediathekview.de/Filmliste-akt.xz";

static unsigned long min_length;
static int debug;
static strlist include, exclude;

// static
/*ssize_t
buffer_dummyread(int fd, char* b, size_t n) {
  (void)fd;
  (void)b;
  (void)n;
  return 0;
}*/

void
count_field_lengths(strlist* sl, int lengths[21]) {
  int i;
  for(i = 0; i < 21; i++) {
    const char* s = strlist_at(sl, i);
    if(s) {
      if(str_len(s) >= (unsigned)lengths[i])
        lengths[i] = str_len(s);
    }
  }
}

int
split_fields(strlist* sl, strlist* prev, char* buf, size_t n) {
  int ret = 0;
  char buf2[4096];
  char* p = buf;
  char* end = buf + n;

  strlist_zero(sl);

  for(;;) {
    size_t n = 0;
    while(p < end && *p != '"') ++p;

    if(p == end)
      break;

    ++p;

    while(p < end) {

      if(*p == '\\' && p[1] == '"') {
        ++p;
      } else {
        if(*p == '"') {
          ++p;
          break;
        }
      }

      buf2[n++] = *p++;
    }

    if(ret >= 1 && ret <= 2 && n == 0) {
      strlist_push(sl, strlist_at(prev, ret));
    } else {
      strlist_pushb(sl, buf2, n);
    }
    ret++;
  }

  return ret;
}

void
process_status(void) {
/* display interesting process IDs  */
#if !(defined(_WIN32) || defined(_WIN64))
  buffer_putm_internal(buffer_2, "process ", argv0, ": pid=", 0);
  buffer_putlong(buffer_2, getpid());
  buffer_puts(buffer_2, ", ppid=");
  buffer_putlong(buffer_2, getppid());
  buffer_puts(buffer_2, ", pgrp=");
  buffer_putlong(buffer_2, getpgrp());
  buffer_puts(buffer_2, ", tcpgrp=");
  buffer_putlong(buffer_2, tcgetpgrp(STDIN_FILENO));
  buffer_putnlflush(buffer_2);
#endif
}

int
read_mediathek_list(const char* url) {
  /*  int status;
    int xzpid;
    int xzpipe[2];
    int clpipe[2];
    int clpid;

    process_status();

    if(pipe(xzpipe) != 0) return -1;
    if(pipe(clpipe) != 0) return -1;

    if((xzpid = fork()) == 0) {

      close(STDOUT_FILENO);
      dup(xzpipe[1]);
      close(STDIN_FILENO);
      dup(clpipe[0]);

      execlp("xzcat", "xzcat", 0);
      exit(1);
    }

    if((clpid = fork()) == 0) {

      close(STDOUT_FILENO);
      dup(clpipe[1]);

      execlp("curl", "curl", "-s", url, 0);
      exit(1);
    }

    return xzpipe[0];*/

  stralloc cmd;
  stralloc_init(&cmd);
  stralloc_copys(&cmd, "curl -s ");
  stralloc_cats(&cmd, url);
  stralloc_cats(&cmd, "| xzcat");
  stralloc_0(&cmd);

  {
    static FILE* pfd;

    if((pfd = (void*)popen(cmd.s, "r")) == 0)
      return -1;

    return fileno(pfd);
  }
}

/* Parses a time in HH:MM:SS format and returns seconds */
unsigned long
parse_time(const char* s) {
  size_t r = 0;
  unsigned short n;
  size_t l;

  if(s == 0)
    return 0;

  for(;;) {
    l = scan_ushort(s, &n);
    if(l == 0)
      break;
    r += n;
    s += l;
    if(*s == ':') {
      r *= 60;
      ++s;
      continue;
    }
    if(*s == '\0')
      break;
  }
  return r;
}

char*
format_num(time_t num) {
  static char buf[FMT_LONG];
  byte_zero(buf, sizeof(buf));
  buf[fmt_uint64(buf, num)] = '\0';
  return buf;
}

char*
format_time(time_t ti) {

  static char buf[1024];

  unsigned short h, m, s;
  size_t i = 0;

  s = ti % 60;
  ti /= 60;
  m = ti % 60;
  ti /= 60;
  h = ti;

  if(h < 10)
    buf[i++] = '0';
  i += fmt_ulong(&buf[i], h);
  buf[i++] = ':';
  if(m < 10)
    buf[i++] = '0';
  i += fmt_ulong(&buf[i], m);
  buf[i++] = ':';
  if(s < 10)
    buf[i++] = '0';
  i += fmt_ulong(&buf[i], s);
  buf[i] = '\0';

  return buf;
}

time_t
parse_datetime(const char* s, const char* fmt) {
  struct tm tm_s;
  byte_zero(&tm_s, sizeof(struct tm));
  if(str_ptime(s, fmt, &tm_s) == s)
    return 0;
  return mktime(&tm_s);
}

time_t
parse_anydate(const char* s) {
  const char* fmt;
  size_t len = str_len(s);
  if(len != 8) /* len - str_rchr(s, '.') == 4) */
    fmt = "%d.%m.%Y";
  else
    fmt = "%Y%m%d";
  return parse_datetime(s, fmt);
}

char*
format_datetime(size_t t, const char* fmt) {
  static char buf[1024];
  time_t tm = t;
  /*size_t n =*/strftime(buf, sizeof(buf), fmt, localtime(&tm));

  return buf; /*  buffer_put(b, buf, n); */
}

char*
make_url(const char* base, const char* trail) {
  static stralloc url;

  unsigned int n = 0;
  size_t i;

  if(trail == 0)
    return 0;
  i = scan_uint(trail, &n);
  if(trail[i] != '|')
    return 0;
  stralloc_copyb(&url, base, n);

  stralloc_cats(&url, &trail[++i]);
  stralloc_0(&url);
  return url.s;
}

typedef struct mediathek_entry {
  stralloc channel;
  stralloc topic;
  stralloc title;
  time_t tm;
  time_t dr;
  unsigned int mbytes;
  stralloc desc;
  stralloc url;
  stralloc link;
} mediathek_entry_t;

mediathek_entry_t*
new_mediathek_entry() {
  mediathek_entry_t* e = malloc(sizeof(mediathek_entry_t));
  if(e == 0)
    return 0;
  byte_zero(e, sizeof(mediathek_entry_t));
  return e;
}

mediathek_entry_t*
create_mediathek_entry(
    const char* ch, const char* tpc, const char* tit, const char* dsc, const char* ur, const char* ln) {
  mediathek_entry_t* e = new_mediathek_entry();
  if(e == 0)
    return 0;

  stralloc_copys(&e->channel, ch);
  stralloc_0(&e->channel);
  stralloc_copys(&e->topic, tpc);
  stralloc_0(&e->topic);
  stralloc_copys(&e->title, tit);
  stralloc_0(&e->title);

  stralloc_copys(&e->desc, dsc ? dsc : "");
  stralloc_0(&e->desc);
  stralloc_copys(&e->url, ur ? ur : "");
  stralloc_0(&e->url);
  stralloc_copys(&e->link, ln ? ln : "");
  stralloc_0(&e->link);

  return e;
}

void
delete_mediathek_entry(mediathek_entry_t* e) {
  stralloc_free(&e->channel);
  stralloc_free(&e->topic);
  stralloc_free(&e->title);

  stralloc_free(&e->desc);
  stralloc_free(&e->url);
  stralloc_free(&e->link);
  free(e);
}

static mediathek_entry_t* e;


/**
 * @brief match_tokens
 * @param toks
 * @param str
 * @return 1 if all tokens match
 */
int
match_tokens(char* toks, const char* str) {
  size_t i;
  int ret = 1;
  strlist t;
  strlist_init(&t, '\0');
  strlist_push_tokens(&t, toks, "/+,|*;");

  for(i = 0; i < strlist_count(&t); ++i) {
    char* tok = strlist_at(&t, i);

    if(str_istr((char*)str, tok) == 0) {
      ret = 0;
      break;
    }
  }

  if(ret && debug > 1) {
    buffer_putm_5(buffer_2, "token list '", toks, "' matched '", str, "'.");
    buffer_putnlflush(buffer_2);
  }

  strlist_free(&t);
  return ret;
}

/* returns 1 if any of the token lists match */
int
match_toklists(strlist* sl) {
  stralloc sa;
  int ret = 0;
  size_t i, n;
  stralloc_init(&sa);
  strlist_join(sl, &sa, '|');
  stralloc_0(&sa);

  n = strlist_count(&include);
  if(n == 0)
    ret = 1;

  for(i = 0; i < n; ++i) {
    char* toklist = strlist_at(&include, i);

    if(match_tokens(toklist, sa.s)) {
      ret = 1;
      break;
    }
  }

  if(ret == 0)
    return 0;

  n = strlist_count(&exclude);

  for(i = 0; i < n; ++i) {
    char* toklist = strlist_at(&exclude, i);

    if(match_tokens(toklist, sa.s)) {
      ret = 0;
      break;
    }
  }

  stralloc_free(&sa);

  return ret;
}

mediathek_entry_t*
parse_entry(strlist* sl) {

  mediathek_entry_t* ret;
  time_t dt = parse_anydate(strlist_at(sl, 4));

  time_t tm = parse_time(strlist_at(sl, 5));
  time_t dr = parse_time(strlist_at(sl, 6)); /* duration */

  if((unsigned)dr < min_length)
    return 0;
  {
    unsigned int mbytes = 0;
    const char* mb = strlist_at(sl, 7);
    if(mb)
      scan_uint(mb, &mbytes);

    {
      const char* desc = strlist_at(sl, 8);
      const char* url = strlist_at(sl, 9);
      const char* link = strlist_at(sl, 10);

      ret = create_mediathek_entry(strlist_at(sl, 1),
                                   strlist_at(sl, 2),
                                   strlist_at(sl, 3),

                                   desc,
                                   url,
                                   link

      );

      if(ret) {
        ret->tm = dt + tm;
        ret->dr = dr;
        ret->mbytes = mbytes;
      }
      return ret;
    }
  }
}

void
print_entry(buffer* b, const mediathek_entry_t* e) {

  const char* sep = ", ";

  buffer_putm_internal(b, "Kanal:\t", e->channel.s /*strlist_at(sl, 1)*/, sep, 0);
  buffer_putm_internal(b, "Thema:\t", e->topic.s /*strlist_at(sl, 2)*/, sep, 0);
  buffer_putm_internal(b, "Titel:\t", e->title.s /*strlist_at(sl, 3)*/, sep, 0);

  buffer_putm_internal(b, "Datum:\t", format_datetime(e->tm, dt_fmt), sep, 0);
  buffer_putm_internal(b, "Dauer:\t", format_time(e->dr), sep, 0);
  buffer_putm_internal(b, "Gr�sse:\t", format_num(e->mbytes), "MB", sep, 0);

  /* buffer_putm_3(b, "URL:\t", url , sep);
   buffer_putm_5(b, "URL lo:\t", make_url(url, strlist_at(sl, 13)), sep);
   buffer_putm_5(b, "URL hi:\t", make_url(url, strlist_at(sl, 15)), sep);*/

  buffer_putnlflush(b);
}

void
output_entry(buffer* b, strlist* sl) {

  size_t i, n = strlist_count(sl);

  buffer_put(b, " ", 1);

  for(i = 0; i < n; ++i) {
    char c;
    const char* s = strlist_at(sl, i);
    buffer_put(b, " \"", 2);

    while((c = *s++)) {
      if(c == '"') {
        c = '\\';
        buffer_PUTC(b, c);
        c = '"';
      }
      buffer_PUTC(b, c);
    }

    buffer_puts(b, (i == 0 ? "\" : [" : ((i + 1 < n) ? "\"," : "\" ]")));
  }
}

int
parse_mediathek_list(int fd) {
  char buf[1024];
  static char buf2[BUFSIZE];
  size_t matched = 0, total = 0;
  ssize_t ret, ret2;
  strlist prev, prevout, sl;
  buffer b;
  buffer_init(&b, (buffer_op_sys*)&read, fd, buf, sizeof(buf));

  strlist_init(&prev, '\0');
  strlist_init(&prevout, '\0');

  buffer_put(buffer_1, "{\n", 2);

  while((ret = buffer_get_token(&b, buf2, sizeof(buf2), "]", 1)) > 0) {

    for(;;) {
      if(ret + 1 >= BUFSIZE)
        break;
      buf2[ret++] = ']';
      ret2 = buffer_get(&b, &buf2[ret], 1);
      if(ret2 > 0) {

        if(ret > 1 && buf2[ret - 2] == '"' && buf2[ret] == ',')
          break;

        ret += ret2;
        ret2 = buffer_get_token(&b, &buf2[ret], sizeof(buf2) - ret, "]", 1);
        if(ret2 > 0)
          ret += ret2;
      }
    }

    strlist_init(&sl, '\0');
    split_fields(&sl, &prev, buf2, ret);

    if((e = parse_entry(&sl))) {
      total++;
      if(debug > 2)
        print_entry(buffer_2, e);

      if(match_toklists(&sl)) {
        matched++;

        if(strlist_count(&prevout)) {
          buffer_put(buffer_1, ",\n", 2);
          buffer_flush(buffer_1);
        }

        output_entry(buffer_1, &sl);
        prevout = sl;
      }

      delete_mediathek_entry(e);
      e = 0;
    }

    prev = sl;
  }

  buffer_flush(buffer_1);

  if(debug) {
    buffer_puts(buffer_2, "\nprocessed ");
    buffer_putulong(buffer_2, matched);
    buffer_puts(buffer_2, "/");
    buffer_putulong(buffer_2, total);
    buffer_puts(buffer_2, " entries.");
    buffer_putnlflush(buffer_2);
  }

  return 0;
}

/**
 * Show command line usage
 */
void
usage(char* argv0) {
  buffer_putm_internal(buffer_1,
                       "Usage: ",
                       str_basename(argv0),
                       "[OPTIONS] [KEYWORDS...]\n",
                       "\n",
                       "Options\n",
                       "  -h, --help                show this help\n",
                       "  -F                        date/time format\n",
                       "  -t HH:MM:SS               minimum length\n",
                       "  -i KEYWORD                include entries matching\n",
                       "  -x KEYWORD                exclude entries matching\n",
                       "\n",
                       0);
  buffer_putnlflush(buffer_1);
}

int
main(int argc, char* argv[]) {

  int opt;

  min_length = 0;

  errmsg_iam(argv[0]);

  while((opt = getopt(argc, argv, "F:dt:i:x:h")) != -1) {
    switch(opt) {
      case 'h': usage(argv[0]); return 0;
      case 'F': dt_fmt = optarg; break;
      case 'd': debug++; break;
      case 't': min_length = parse_time(optarg); break;
      case 'i': strlist_push(&include, optarg); break;
      case 'x': strlist_push(&exclude, optarg); break;
      default: usage(argv[0]); return EXIT_FAILURE;
    }
  }

  while(optind < argc) {

    strlist_push(&include, argv[optind++]);
  }

  /* if(strlist_count(&include) == 0)
     strlist_push(&include, "");*/

  strlist_dump(buffer_2, &include);
  strlist_dump(buffer_2, &exclude);

  /*  stralloc sa;
    stralloc_init(&sa);

    strlist_join(&include, &sa, ',');

    buffer_putsa(buffer_2, &sa);
    buffer_putnlflush(buffer_2);*/
  /*
    fprintf(stderr, "%p\n", str_istr("blah", ""));
    fprintf(stderr, "%p\n", str_istr("[", "blah"));
    fflush(stderr);
    */

  /*   if(optind >= argc) {
         fprintf(stderr,
                 "Nach den Optionen wurde ein Argument erwartet\n");
         exit(EXIT_FAILURE);
     }
  */

  strlist_dumpx[1] = '\n';
  strlist_dumpx[2] = '\t';

  argv0 = argv[0];

  if(argv0[str_rchr(argv0, '/')] != '\0')
    argv0 += str_rchr(argv0, '/') + 1;

  return parse_mediathek_list(read_mediathek_list(mediathek_url));
}
