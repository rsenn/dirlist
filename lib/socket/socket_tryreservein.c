#define USE_WS2_32 1
#include "../socket.h"
#include <sys/types.h>

void
socket_tryreservein(int s, int size) {
  while(size >= 1024) {
    if(winsock2errno(setsockopt(s, SOL_SOCKET, SO_RCVBUF, &size, sizeof size)) == 0) return;
    size -= (size >> 5);
  }
}
