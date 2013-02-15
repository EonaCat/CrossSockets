// CrossSockets
// Copyright Jeroen Saey
// Created 27-01-2013
// CrossSocket.cpp

#include "CrossSocket.h"

#ifndef __WIN32__
  #include <netdb.h>
  #include <arpa/inet.h>
  #include <fcntl.h>
  #include <sys/select.h>
  #include <sys/time.h>
#else
  #define F_SETFL FIONBIO
  #define O_NONBLOCK 1
  
  extern int close(int fd);
  extern int fcntl(int fd, int cmd, long arg);
#endif

using namespace std;

CrossSocket::CrossSocket(blockTypeEnum block)
{
    _blockingMode = block;    
}

CrossSocket::~CrossSocket()
{

}

void CrossSocket::getSocket()
{
    if (_socketHandle < 0)
    {
        _socketHandle = socket(PF_INET, SOCK_STREAM, 0);

        if (_blockingMode == nonblocking)
        {
            fcntl(_socketHandle, F_SETFL, O_NONBLOCK);
        }

        //reset state	
        reset();
    }
}

BaseCrossSocket* CrossSocket::create(int socketdescriptor, CrossSocketErrors *error)
{
    CrossSocket* remoteClass;

    remoteClass = new CrossSocket(_blockingMode);
    remoteClass->_socketHandle = socketdescriptor;

    noError(error);
    return remoteClass;
}

bool CrossSocket::bind(int port, CrossSocketErrors *error)
{
    return bind(port, "", error);
}

bool CrossSocket::bind(int port, string host, CrossSocketErrors *error)
{
    hostent *hostEntry;
    in_addr networkAddress; 	

    if (host.size() > 0 )
    {
        // Bind to a specific address

        if ((hostEntry = gethostbyname(host.data())) == NULL)
        {
            setError(error, fatal, "CrossSocket::bind() - Can't get host by name");
            return false;
        }

        networkAddress = *((in_addr *)hostEntry->h_addr);
     }
     else
     {
        // Bind to any address
        networkAddress.s_addr = INADDR_ANY;
     }

    getSocket();

    sockaddr_in serverAddress;

    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = networkAddress.s_addr;

    if (::bind(_socketHandle, (sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
    {
        handleError(error, "CrossSocket::bind() error: ");
        return false;
    }

    noError(error);
    return true;
}

bool CrossSocket::connect(int port, string hostname, CrossSocketErrors *error)
{
    getSocket();

    hostent *host;

    if ((host = gethostbyname(hostname.data())) == NULL )
    {
        setError(error, fatal, "CrossSocket::connect() - Can't get host by name");
        return false;
    }

    sockaddr_in clientAddress;

    memset(&clientAddress, 0, sizeof(clientAddress));
    clientAddress.sin_family = AF_INET;
    clientAddress.sin_port = htons(port);
    clientAddress.sin_addr = *((in_addr *)host->h_addr);

    if (::connect(_socketHandle, (sockaddr *)&clientAddress, sizeof(clientAddress)) == -1)
    {
        handleError(error, "CrossSocket::connect() error: ");
        return false;
    }
    
    noError(error);
    return true;
}


string CrossSocket::getClientAddress(CrossSocketErrors *error)
{
    sockaddr_in clientAddress;

    if (!getClientHost((sockaddr *)&clientAddress, error))
    {
        return "";
    }

    char *addressPointer;

    if ((addressPointer = inet_ntoa(clientAddress.sin_addr)) == NULL )
    {
        setError(error, fatal, "CrossSocket::getClientHostName() - Can't get client address");
        return "";
    }
    
    noError(error);
    return string (addressPointer);
}

int CrossSocket::getClientPort(CrossSocketErrors *error)
{
    sockaddr_in clientAddress;

    if (!getClientHost((sockaddr *)&clientAddress, error))
    {
        return -1;
    }

    noError(error);

    return ntohs(clientAddress.sin_port);
}

string CrossSocket::getClientHostName(CrossSocketErrors *error)
{
    string name = getClientAddress(error);
    
    if (name.size() < 1)
    {
        return "";
    }

    hostent *host; 	

    if ((host = gethostbyname(name.data())) == NULL )
    {
        setError(error, fatal, "CrossSocket::getClientHostName() - Can't get client by address");
        return "";
    }
    
    noError(error);
    return string(host->h_name);;
}

string CrossSocket::getServerAddress(CrossSocketErrors *error)
{
    //We need to get the real address, so we must
    //first get this computers host name and then
    //translate that into an address!

    string name = getServerHostName(error);
    if (name.size() < 1)
    {
        return "";
    }

    hostent *host;

    if ((host = gethostbyname(name.data())) == NULL )
    {
        setError(error, fatal, "CrossSocket::getServerAddress() - Can't get host by name");
        return "";
    }

    char *addressPointer;

    if ((addressPointer = inet_ntoa(*((in_addr *)host->h_addr))) == NULL)
    {
        setError(error, fatal, "CrossSocket::getServerAddress() - Can't get host address");
        return "";
    }
    return string(addressPointer);
}

int CrossSocket::getServerPort(CrossSocketErrors *error)
{
    sockaddr_in address;

    if (!getServerHost((sockaddr *)&address, error))
    {
        return -1;
    }

    noError(error);

    return ntohs(address.sin_port);
}

string CrossSocket::getServerHostName(CrossSocketErrors *error)
{
    char buffer[256];

    if (gethostname(buffer, 256) != 0)
    {
        handleError(error, "CrossSocket::gethostname() error: ");
        return "";
    }

    string message(buffer);

    noError(error);
    return message;
}
