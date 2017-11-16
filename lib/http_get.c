#include "http.h"
#include "socket.h"
#include "str.h"
#include "byte.h"
#include "io.h"
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int
http_get(http* h, const char* location) {
  http_request* r;
  struct hostent *he;
  struct in_addr a;

  stralloc_0(&h->host);
  he = gethostbyname(h->host.s);

  if(he == NULL)
    return 0;
  a = *(struct in_addr*)he->h_addr_list[0];

  if(a.s_addr == 0)
    return 0;

//  stralloc_ready(&h->addr, 16);
  stralloc_copys(&h->addr, inet_ntoa(a));
  h->addr.len = str_len(h->addr.s);

  h->sock = socket_tcp4();
  io_nonblock(h->sock);

  for(r = h->requests; r->next; r = r->next)
  {}

  r->next = malloc(sizeof(http_request));
  byte_zero(&r->next, sizeof(http_request));
  r->next->serial = r->serial+1;
  r = r->next;

  r->type = GET;

  stralloc_init(&r->location);
  stralloc_copys(&r->location, location);

  return socket_connect4(h->sock, h->addr.s, h->port);
}
