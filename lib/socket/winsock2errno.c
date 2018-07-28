#if defined(_WIN32) || defined(_WIN64)
#include <winsock.h>
#include <errno.h>
#include <stdio.h>
#include "../socket_internal.h"

int winsock2errno(long l) {
  long x;
  if(l == -1)
    switch((x = WSAGetLastError())) {
    case WSANOTINITIALISED:
      printf("WSANOTINITIALISED!\n");
      exit(111);
    case WSAENETDOWN:
      printf("WSAENETDOWN!\n");
      exit(111);
    case WSAEINTR:
      errno = EINTR; break;
    case WSAEBADF:
      errno = EBADF; break;
    case WSAEACCES:
      errno = EACCES; break;
    case WSAEFAULT:
      errno = EFAULT; break;
    case WSAEINVAL:
      errno = EINVAL; break;
    case WSAEMFILE:
      errno = EMFILE; break;
    case WSAENAMETOOLONG:
      errno = ENAMETOOLONG; break;
    case WSAENOTEMPTY:
      errno = ENOTEMPTY; break;
    case WSAEPROTONOSUPPORT:
      errno = EPROTONOSUPPORT; break;
    default:
      errno = x;
      break;
    }
  return l;
}
#endif