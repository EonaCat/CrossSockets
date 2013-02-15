// CrossSockets
// Copyright Jeroen Saey
// Created 27-01-2013
// CrossSocketPlatform.h

#ifdef __WIN32__
// Define the errorCodes for the Windows platform (taken from CYGWIN error numbers)
  #define EOPNOTSUPP WSAEOPNOTSUPP
  #define EINTR WSAEINTR
  #define EADDRINUSE WSAEADDRINUSE
  #define EINPROGRESS WSAEINPROGRESS
  #define EWOULDBLOCK WSAEWOULDBLOCK
  #define ENOTSOCK WSAENOTSOCK
  #define EMSGSIZE WSAEMSGSIZE
  #define ETIMEDOUT WSAETIMEDOUT
  #define EALREADY WSAEALREADY
  #define EBADF WSAEBADF 
  #define ECONNREFUSED WSAECONNREFUSED
  #define ENOTCONN WSAENOTCONN
#else 
  #include <netdb.h>
  #include <arpa/inet.h>
  #include <fcntl.h>
  #include <sys/select.h>
  #include <sys/time.h>
  #include <sstream>
  
  #define SOCKETFAIL -1
#endif

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

// Redefine the socklen_t parameter because it is different on Windows
#ifdef __WIN32__
  #define sw_socklen_t int
#else
  #define sw_socklen_t socklen_t
#endif

using namespace std;

#ifdef __WIN32__

int fcntl(int fd, int cmd, long arg)
{
    unsigned long mode = arg;

    return WSAIoctl(fd, cmd, &mode, sizeof(unsigned long), NULL, 0, NULL, NULL, NULL);
}

int close(int fd)
{
    return closesocket(fd);
}

void WSA_exit(void)
{
    WSACleanup();
}
#endif
