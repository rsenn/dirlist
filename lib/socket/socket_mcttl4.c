#define USE_WS2_32 1
#include "../socket.h"
#include <sys/types.h>

#if WINDOWS && !defined(IP_MULTICAST_TTL)
#include <ws2tcpip.h>
#endif

int
socket_mcttl4(int s, char TTL) {
  return winsock2errno(setsockopt(s, IPPROTO_IP, IP_MULTICAST_TTL, &TTL, sizeof TTL));
}
