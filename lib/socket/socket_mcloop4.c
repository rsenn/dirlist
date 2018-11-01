#define USE_WS2_32 1

#if WINDOWS
#define _WINSOCKAPI_
#include <winsock2.h>
#endif

#include "../socket_internal.h"
#include <sys/types.h>


int
socket_mcloop4(int s, char loop) {
#ifdef IP_MULTICAST_LOOP
  return winsock2errno(setsockopt(s, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof loop));
#else
  return -1;
#endif
}
