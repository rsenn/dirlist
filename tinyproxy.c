/*
 * Tiny TCP proxy server
 *
 * Author: Krzysztof Kliś <krzysztof.klis@gmail.com>
 * Fixes and improvements: Jérôme Poulin <jeromepoulin@gmail.com>
 * IPv6 support: 04/2019 Rafael Ferrari <rafaelbf@hotmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version with the following modification:
 *
 * As a special exception, the copyright holders of this library give you
 * permission to link this library with independent modules to produce an
 * executable, regardless of the license terms of these independent modules,
 * and to copy and distribute the resulting executable under terms of your choice,
 * provided that you also meet, for each linked independent module, the terms
 * and conditions of the license of that module. An independent module is a
 * module which is not derived from or based on this library. If you modify this
 * library, you may extend this exception to your version of the library, but
 * you are not obligated to do so. If you do not wish to do so, delete this
 * exception statement from your version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "lib/uint16.h"
#include "lib/uint64.h"
#include "lib/stralloc.h"
#include "lib/buffer.h"
#include "lib/ip4.h"
#include "lib/ip6.h"
#include "lib/open.h"
#include "lib/str.h"
#include "lib/io.h"
#include "lib/io_internal.h"
#include "lib/socket.h"
#include "lib/ndelay.h"
#include "lib/fmt.h"
#include "lib/scan.h"
#include "lib/getopt.h"
#include "lib/errmsg.h"
#include "lib/sig.h"
#include "lib/array.h"
#include "lib/slist.h"
#include <arpa/inet.h>
//#include <netdb.h>
#include <netinet/in.h>
//#include <resolv.h>
//#include <sys/socket.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/wait.h>
#ifdef USE_SYSTEMD
#include <systemd/sd-daemon.h>
#endif

#define BUF_SIZE 512

#define READ 0
#define WRITE 1

#define SERVER_SOCKET_ERROR -1
#define SERVER_SETSOCKOPT_ERROR -2
#define SERVER_BIND_ERROR -3
#define SERVER_LISTEN_ERROR -4
#define CLIENT_SOCKET_ERROR -5
#define CLIENT_RESOLVE_ERROR -6
#define CLIENT_CONNECT_ERROR -7
#define CREATE_PIPE_ERROR -8
#define BROKEN_PIPE_ERROR -9
#define SYNTAX_ERROR -10

typedef enum { TRUE = 1, FALSE = 0 } bool;
typedef struct socketbuf_s {
  fd_t sock;
  buffer buf;
  char addr[16];
  uint16 port;
  uint32 scope_id;
  int af;
  fd_t dump;
} socketbuf_t;

typedef struct connection_s {
  slink link;
  socketbuf_t client;
  socketbuf_t proxy;
  int connected;
} connection_t;

static slink* connections;

size_t dump_fds(array*);
void dump_io(void);

connection_t* connection_new(fd_t, char addr[16], uint16 port);
void connection_delete(connection_t*);
connection_t* connection_find(fd_t client, fd_t proxy);

socketbuf_t* socket_find(fd_t);
ssize_t socket_send(fd_t, void* x, size_t n, void* ptr);
int socket_connect(void);
void socket_accept(fd_t, char addr[16], uint16 port);

size_t sockbuf_fmt_addr(socketbuf_t*, char* dest, char sep);
fd_t sockbuf_open_log(socketbuf_t*, const char* prefix, const char* suffix);
void sockbuf_put_addr(buffer*, socketbuf_t* sb);
void sockbuf_close(socketbuf_t*);
void sockbuf_check(socketbuf_t*);
void sockbuf_log_data(socketbuf_t*, bool send, char* x, ssize_t len);
ssize_t sockbuf_forward_data(socketbuf_t*, socketbuf_t* destination);

fd_t server_socket(void);
fd_t server_listen(uint16);
void server_loop(void);
void server_connection_count(void);

fd_t server_sock, remote_sock;
uint16 connect_port = 0, local_port = 0;
int64 connections_processed = 0, max_length = -1;
char *remote_host, *cmd_in, *cmd_out;
static char bind_addr[16], connect_addr[16];
static int bind_af = AF_INET, connect_af = AF_INET;
bool foreground = TRUE, use_syslog = FALSE, line_buffer = FALSE, dump = FALSE;

static char logbuf[1024];
static buffer log = BUFFER_INIT(write, STDOUT_FILENO, logbuf, sizeof(logbuf));

#define BACKLOG 20 // how many pending connections queue will hold

static int
parse_addr(const char* opt, char ip[16]) {
  size_t n;

  if((n = scan_ip6(opt, ip)))
    return AF_INET6;
  if((n = scan_ip4(opt, ip)))
    return AF_INET;
  return -1;
}

static size_t
byte_numlines(char* x, size_t p, size_t* end_ptr) {
  size_t pos, i, n = 0;
  pos = byte_rchr(x, p, '\n');
  if(pos < p)
    for(i = 0; i < pos; i++)
      if(x[i] == '\n')
        n++;
      else
        pos = 0;
  if(end_ptr)
    *end_ptr = pos;
  return n;
}

static size_t
buffer_numlines(buffer* b, size_t* end_ptr) {
  return byte_numlines(b->x, b->p, end_ptr);
}

size_t
dump_fds(array* arr) {
  slink* sl;
  array_catb(arr, &server_sock, sizeof(fd_t));
  for(sl = connections; sl; sl = sl->next) {
    connection_t* c = (connection_t*)sl;
    array_catb(arr, &c->client.sock, sizeof(fd_t));
    array_catb(arr, &c->proxy.sock, sizeof(fd_t));
  }
  return array_length(arr, sizeof(fd_t));
}

/*void
iarray_dump(iarray* ia) {
  size_t i, n = iarray_length(ia);
  for(i = 0; i <= n; i++) {
    buffer_puts(buffer_2, "Entry #");
    buffer_putulong(buffer_2, i);
    buffer_puts(buffer_2, ": ");
    buffer_putptr(buffer_2, iarray_get(ia, i));
    buffer_putnlflush(buffer_2);
  }
}
*/
void
dump_io() {
  array fds;
  size_t num_fds, i;
  array_init(&fds);

  num_fds = dump_fds(&fds);
  for(i = 0; i < num_fds; i++) {
    fd_t fd = *(fd_t*)array_get(&fds, sizeof(fd_t), i);

    io_entry* e = io_getentry(fd);

    buffer_puts(buffer_2, "FD #");
    buffer_putulong(buffer_2, fd);
    if(fd == server_sock)
      buffer_puts(buffer_2, " (listen) ");

    buffer_puts(buffer_2, " want:");
    if(e->wantread)
      buffer_puts(buffer_2, " read");
    if(e->wantwrite)
      buffer_puts(buffer_2, " write");
    buffer_puts(buffer_2, " can:");
    if(e->canread)
      buffer_puts(buffer_2, " read");
    if(e->canwrite)
      buffer_puts(buffer_2, " write");
    buffer_putnlflush(buffer_2);
  }
}

connection_t*
connection_new(fd_t sock, char addr[16], uint16 port) {
  connection_t* c = alloc_zero(sizeof(connection_t));
  slink_insert(&connections, &c->link);
  c->client.sock = sock;
  c->client.port = port;
  c->client.af = bind_af;
  byte_copy(c->client.addr, 16, addr);

  c->client.dump = -1;
  return c;
}

void
connection_delete(connection_t* c) {
  connection_t** ptr;
  slink_foreach(&connections, ptr) {
    if(*ptr == c)
      break;
  }
  *ptr = (connection_t*)c->link.next;
  alloc_free(c);
}

connection_t*
connection_find(fd_t client, fd_t proxy) {
  slink* sl;
  for(sl = connections; sl; sl = sl->next) {
    connection_t* c = (connection_t*)sl;
    if(c->client.sock == client || c->proxy.sock == proxy)
      return c;
  }
  return NULL;
}

socketbuf_t*
socket_find(fd_t sock) {
  connection_t* c;
  if((c = connection_find(sock, -1)))
    return &c->client;
  if((c = connection_find(-1, sock)))
    return &c->proxy;
  return NULL;
}

ssize_t
socket_send(fd_t fd, void* x, size_t n, void* ptr) {
  ssize_t r = send(fd, x, n, 0);

  if(r > 0) {
    socketbuf_t* sb = socket_find(fd);
    sockbuf_log_data(sb, TRUE, x, n);
    if(dump) {
      if(sb->dump == -1) {
        connection_t* c = connection_find(fd, fd);
        sb->dump = sockbuf_open_log(&c->client, c->client.sock == fd ? "recv-" : "send-", ".txt");
      }
      write(sb->dump, x, n);
    }
  }
  if(r == -1 && errno == EWOULDBLOCK)
    r = 0;
  return r;
}

/* Create client connection */
int
socket_connect() {
  int sock;
  int ret;

  if((sock = connect_af == AF_INET6 ? socket_tcp6() : socket_tcp4()) < 0)
    return CLIENT_SOCKET_ERROR;

  io_fd(sock);
  io_nonblock(sock);

  if(connect_af == AF_INET6)
    ret = socket_connect6(sock, connect_addr, connect_port, 0);
  else
    ret = socket_connect4(sock, connect_addr, connect_port);

  if(ret < 0 && errno != EINPROGRESS)
    return CLIENT_CONNECT_ERROR;

  io_wantwrite(sock);

  return sock;
}

/* Handle client connection */
void
socket_accept(fd_t sock, char addr[16], uint16 port) {

  connection_t* c = connection_new(sock, addr, port);

  if((c->proxy.sock = socket_connect()) < 0)
    goto cleanup;

  byte_copy(c->proxy.addr, connect_af == AF_INET6 ? 16 : 4, connect_addr);
  c->proxy.port = connect_port;
  c->proxy.af = connect_af;

  buffer_init_free(&c->client.buf, socket_send, c->client.sock, alloc_zero(1024), 1024);
  buffer_init_free(&c->proxy.buf, socket_send, c->proxy.sock, alloc_zero(1024), 1024);

  io_fd(c->client.sock);
  io_nonblock(c->client.sock);
  io_wantread(c->client.sock);

  buffer_puts(&log, "New connection from ");
  sockbuf_put_addr(&log, &c->client);

  buffer_putnlflush(&log);
  return;

cleanup:
  io_close(c->proxy.sock);
  io_close(c->client.sock);
}

size_t
sockbuf_fmt_addr(socketbuf_t* sb, char* dest, char sep) {
  size_t n = sb->af == AF_INET6 ? fmt_ip6(dest, sb->addr) : fmt_ip4(dest, sb->addr);
  dest[n++] = sep ? sep : ':';

  n += fmt_ulong(&dest[n], sb->port);
  return n;
}

fd_t
sockbuf_open_log(socketbuf_t* sb, const char* prefix, const char* suffix) {
  char buf[256] = {0};
  fd_t fd;
  size_t i, n;

  n = str_copy(buf, prefix);

  n += sockbuf_fmt_addr(sb, &buf[n], '-');
  n += str_copy(&buf[n], suffix);

  for(i = 0; i < n; i++) {
    char c = buf[i];
    if(c == ':') //!((c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f') || (c >= '0' && c <= '9') || c == '.'))
      buf[i] = '_';
  }
  buf[i] = '\0';
  fd = open_trunc(buf);

  return fd;
}

void
sockbuf_put_addr(buffer* b, socketbuf_t* sb) {
  char buf[100];

  buffer_put(b, buf, sockbuf_fmt_addr(sb, buf, ':'));
}

void
sockbuf_close(socketbuf_t* sb) {

  buffer_flush(&sb->buf);
  buffer_close(&sb->buf);
  io_close(sb->sock);

  if(sb->dump != -1)
    close(sb->dump);
}

void
sockbuf_check(socketbuf_t* sb) {
  int wantwrite = /*line_buffer ? data_numlines(&sb->buf, NULL) > 0 :*/ sb->buf.p > 0;
  io_entry* e = io_getentry(sb->sock);

  if(wantwrite) {
    buffer_puts(&log, "Socket want write ");
    buffer_putulong(&log, sb->sock);
    buffer_putnlflush(&log);
    io_wantwrite(sb->sock);
  } else {
    io_wantread(sb->sock);
  }
}

void
sockbuf_log_data(socketbuf_t* sb, bool send, char* x, ssize_t len) {
  while(len > 0) {
    size_t end, n;

    end = n = line_buffer ? byte_chrs(x, len, "\r\n", 2) : len;
    while(n < len && byte_chr("\r\n", 2, x[n]) < 2) {
      n++;
      if(x[n - 1] == '\n')
        break;
    }
    if(!line_buffer)
      end = n;

    buffer_puts(&log, send ? "Sent " : "Received ");
    if(line_buffer) {
      buffer_puts(&log, end == n ? "data" : "line");
      buffer_puts(&log, send ? " to #" : " from #");
    } else {
      buffer_putulong(&log, n);
      buffer_puts(&log, send ? " bytes to #" : " bytes from #");
    }
    buffer_putlong(&log, sb->sock);
    buffer_puts(&log, " ");
    sockbuf_put_addr(&log, sb);
    buffer_puts(&log, " '");

    (line_buffer ? buffer_put : buffer_put_escaped)(&log, x, max_length > 0 && max_length < end ? max_length : end);

    if(max_length > 0 && max_length < end)
      buffer_puts(&log, "<shortened> ...");

    buffer_puts(&log, "'");
    buffer_putnlflush(&log);

    x += n;
    len -= n;
  }
}

/* Forward data between sockets */
ssize_t
sockbuf_forward_data(socketbuf_t* source, socketbuf_t* destination) {
  ssize_t n;
  size_t written = 0;
  char buffer[BUF_SIZE];

  if((n = recv(source->sock, buffer, sizeof(buffer), 0)) > 0) { // read data from input socket

    buffer_put(&destination->buf, buffer, n); // send data to output socket
    written += n;

    sockbuf_log_data(source, FALSE, buffer, n);
  }
  if(written > 0) {
    if(n == -1 && errno == EAGAIN)
      return written;
  }
  return n <= 0 ? n : written;
}

fd_t
server_socket() {
  fd_t s = bind_af == AF_INET ? socket_tcp4() : socket_tcp6();
  if(s == -1)
    return s;

  io_fd(s);

  if(bind_af == AF_INET6) {
    if(socket_bind6_reuse(s, bind_addr, local_port, 0) == 0)
      return s;
  } else {
    if(socket_bind4_reuse(s, bind_addr, local_port) == 0)
      return s;
  }

  io_close(s);
  return -1;
}

/* Create server socket */
fd_t
server_listen(uint16 port) {
  fd_t sock;

  if((sock = server_socket()) == -1)
    return SERVER_BIND_ERROR;

  if(socket_listen(sock, BACKLOG) < 0)
    return SERVER_LISTEN_ERROR;

  return sock;
}

/* Main server loop */
void
server_loop() {

  int sock;
  connection_t* c;
  socketbuf_t* sb;
  uint64 iteration = 0;
  ssize_t n;

#ifdef USE_SYSTEMD
  sd_notify(0, "READY=1\n");
#endif

  while(TRUE) {
    server_connection_count();

    slist_foreach(connections, c) {
      sockbuf_check(&c->client);
      sockbuf_check(&c->proxy);
    }
    io_waituntil2(1000 * 100);
#ifdef DEBUG_OUTPUT_
    buffer_puts(buffer_2, "Loop #");
    buffer_putulonglong(buffer_2, iteration++);
    buffer_putnlflush(buffer_2);
#endif
#ifdef DEBUG_OUTPUT_
    dump_io();
#endif
#ifdef DEBUG_OUTPUT_
    iarray_dump(io_getfds());
#endif
    while((sock = io_canwrite()) != -1) {
      if((c = connection_find(-1, sock)) && !c->connected) {
        buffer_puts(&log, "Socket #");
        buffer_putulong(&log, sock);
        buffer_puts(&log, " connected!");
        buffer_putnlflush(&log);
        c->connected = 1;

        c->proxy.dump = -1;

        connect_af == AF_INET6 ? socket_local6(sock,  c->proxy.addr, &c->proxy.port, &c->proxy.scope_id)
                               : socket_local4(sock,  c->proxy.addr, &c->proxy.port);

        io_dontwantwrite(sock);
        io_wantread(sock);
      } else if((sb = socket_find(sock))) {
#ifdef DEBUG_OUTPUT_
        buffer_puts(buffer_2, "Flush socket ");
        buffer_putulong(buffer_2, sock);
        buffer_puts(buffer_2, " p=");
        buffer_putulong(buffer_2, sb->buf.p);
        buffer_puts(buffer_2, " n=");
        buffer_putulong(buffer_2, sb->buf.n);
        buffer_putnlflush(buffer_2);
#endif

        /*   if(line_buffer) {
             size_t num_lines, end_pos;

             if((num_lines = data_numlines(&sb->buf, &end_pos)) > 0) {
               ssize_t r = socket_send(sb->sock, sb->buf.x, end_pos, 0);
               if(r > 0) {
                 buffer* b = &sb->buf;
                 if(r < b->p)
                   byte_copyr(b->x, b->p - r, &b->x[r]);
                 b->p -= r;
               }
             }

           } else */
        if(sb->buf.p > 0) {
          buffer_flush(&sb->buf);

          io_dontwantwrite(sock);
        }
      } else {
        buffer_puts(buffer_2, "No such fd: ");
        buffer_putulong(buffer_2, sock);
        buffer_putnlflush(buffer_2);

        exit(2);
      }
    }

    while((sock = io_canread()) != -1) {
      if(sock == server_sock) {
        fd_t sock;
        char addr[16];
        uint16 port;
        socklen_t addrlen = sizeof(addr);

        sock = bind_af ? socket_accept4(server_sock, addr, &port) : socket_accept6(server_sock, addr, &port, 0);

        if(sock == -1) {
          errmsg_warn("Accept error: ", strerror(errno), 0);
          exit(2);
        }
        socket_accept(sock, addr, port);

        connections_processed++;

      } else {

        if((c = connection_find(sock, -1))) {

          n = sockbuf_forward_data(&c->client, &c->proxy);
          if(n > 0) {
#ifdef DEBUG_OUTPUT_
            buffer_puts(buffer_2, "Client socket #");
            buffer_putulong(buffer_2, sock);
            buffer_puts(buffer_2, " forwarded ");
            buffer_putlonglong(buffer_2, n);
            buffer_puts(buffer_2, " bytes");
            buffer_putnlflush(buffer_2);
#endif
          }
        } else if((c = connection_find(-1, sock))) {

          n = sockbuf_forward_data(&c->proxy, &c->client);

          if(n > 0) {
#ifdef DEBUG_OUTPUT_
            buffer_puts(buffer_2, "Proxy socket #");
            buffer_putulong(buffer_2, sock);
            buffer_puts(buffer_2, " forwarded ");
            buffer_putlonglong(buffer_2, n);
            buffer_puts(buffer_2, " bytes");
            buffer_putnlflush(buffer_2);
#endif
          }
        } else {
          buffer_puts(buffer_2, "No such fd: ");
          buffer_putulong(buffer_2, sock);
          buffer_putnlflush(buffer_2);

          exit(2);
        }

        if(n == 0 || (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK)) {
          connection_t* c = connection_find(sock, sock);

          buffer_puts(&log, "Connection #");
          buffer_putulong(&log, c->client.sock);

          buffer_puts(&log, " ");
          sockbuf_put_addr(&log, &c->client);

          buffer_puts(&log, " <-> #");
          buffer_putulong(&log, c->proxy.sock);
          buffer_puts(&log, " ");
          sockbuf_put_addr(&log, &c->proxy);
          if(n == 0) {
            buffer_puts(&log, " closed");
          } else {
            buffer_puts(&log, " error: ");
            buffer_puts(&log, strerror(errno));
            buffer_puts(&log, " errno: ");
            buffer_putlong(&log, errno);
          }
          buffer_putnlflush(&log);

          sockbuf_close(&c->client);
          sockbuf_close(&c->proxy);

          connection_delete(c);
        }
      }
    }
  }
}

/* Update systemd status with connection count */
void
server_connection_count() {
#ifdef USE_SYSTEMD
  sd_notifyf(0, "STATUS=Ready. %d connections processed.\n", connections_processed);
#endif
}

/* Program start */
int
main(int argc, char* argv[]) {
  int pid;
  int c, index = 0;
  struct longopt opts[] = {{"local-port", 0, NULL, 'l'},
                           {"local-addr", 0, NULL, 'b'},
                           {"remote-port", 0, NULL, 'p'},
                           {"remote-addr", 0, NULL, 'h'},
                           {"input-parser", 0, NULL, 'i'},
                           {"output-parser", 0, NULL, 'O'},
                           {"foreground", 0, NULL, 'f'},
                           {"syslog", 0, NULL, 's'},
                           {"logfile", 0, NULL, 'o'},
                           {"append", 0, NULL, 'a'},
                           {"max-length", 0, NULL, 'm'},
                           {"line-buffer", 0, NULL, 'L'},
                           {"dump", 0, NULL, 'd'},
                           {0}};

  errmsg_iam(argv[0]);
  while((c = getopt_long(argc, argv, "b:l:h:p:i:O:fso:a:m:Ld", opts, &index)) != -1) {
    switch(c) {
      case 'l': scan_ushort(optarg, &local_port); break;
      case 'b': bind_af = parse_addr(optarg, bind_addr); break;
      case 'h': connect_af = parse_addr(optarg, connect_addr); break;
      case 'p': scan_ushort(optarg, &connect_port); break;
      case 'i': cmd_in = optarg; break;
      case 'O': cmd_out = optarg; break;
      case 'f': foreground = TRUE; break;
      case 's': use_syslog = TRUE; break;
      case 'm': scan_longlong(optarg, &max_length); break;
      case 'L': line_buffer = TRUE; break;
      case 'd': dump = TRUE; break;
      case 'a': log.fd = open_append(optarg); break;
      case 'o': log.fd = open_trunc(optarg); break;
    }
  }

  if(!(local_port && connect_af != -1 && connect_port))
    return SYNTAX_ERROR;

  if(local_port < 0) {
    buffer_putm_internal(buffer_2,
                         "Syntax: ",
                         argv[0],
                         "  -b ADDR     local address\n"
                         "  -l PORT     local port\n"
                         "  -h ADDR     remote address\n"
                         "  -p PORT     remote port\n"
                         "  -f          stay in foreground\n"
                         "  -s          use syslog\n"
                         "  -o          output logfile\n"
                         "  -a          append logfile\n"
                         "  -m LENGTH   max line length\n"
                         "  -L          line buffered\n",
                         0);
    buffer_flush(buffer_2);
    return local_port;
  }

  if(use_syslog)
    openlog("proxy", LOG_PID, LOG_DAEMON);

  sig_block(SIGPIPE);

  if((server_sock = server_listen(local_port)) < 0) { // start server
    errmsg_warn("Cannot run server:", 0);

    return server_sock;
  }

  io_fd(server_sock);
  io_wantread(server_sock);

  server_loop();

  if(use_syslog)
    closelog();

  return EXIT_SUCCESS;
}