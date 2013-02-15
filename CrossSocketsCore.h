// CrossSockets
// Copyright Jeroen Saey
// Created 27-01-2013
// CrossSocketsCore.h

#pragma once

#ifdef __WIN32__
  #include <winsock2.h>
  
  #define F_SETFL FIONBIO
  #define O_NONBLOCK 1
#else
  #include <sys/types.h> 
  #include <sys/socket.h> 
  #include <netinet/in.h>
  #include <sys/un.h>
#endif
