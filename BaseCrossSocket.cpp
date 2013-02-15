// CrossSockets
// Copyright Jeroen Saey
// Created 27-01-2013
// BaseCrossSocket.cpp
 
#include "BaseCrossSocket.h"
#include "CrossSocketPlatform.h"
#include "CrossSocket.h"
#include "CrossThreads.h"
#include "CrossThreadsHandler.h"

#include <errno.h>
#include <new>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

// Create the constructor
BaseCrossSocket::BaseCrossSocket()
{
    _socketHandle = -1;
    _closeSignal = false;

    // Initialize default values
    _error = "";
    _blockingMode = nonblocking;
    _timeoutSeconds = 0;
    _timeoutMicroSeconds = 0;
    _password = "";
    _threadHandler = new CrossThreadsHandler();

    #ifdef __WIN32__
        static bool firstTime = true;
        if (firstTime)
        {
            WSAData wsaData;
            
            if ((int i = WSAStartup(MAKEWORD(1, 1), &wsaData)) != 0)
            {
                handleError(NULL, "BaseCrossSocket - WSAStartup() failed: ");
                exit(-1);
            }
            
            // cleanup
            atexit(WSA_exit);
            firstTime = false;
        }
    #endif          
}

BaseCrossSocket::~BaseCrossSocket()
{	
    if (_socketHandle > 0)
    {
        close(_socketHandle);
    }
}

bool BaseCrossSocket::listen(int connectionLimit, CrossSocketErrors *error)
{
    fprintf(stderr,"This application is using: CrossSockets\n"
            "          Created by Jeroen Saey\n"
            "========================================\n"); 
    
    getSocket();

    char alreadyInUse = 1;
    setsockopt(_socketHandle, SOL_SOCKET, SO_REUSEADDR, &alreadyInUse, sizeof(int));
	
    if(::listen(_socketHandle, connectionLimit) == -1)
    {	
        handleError(error, "BaseCrossSocket::listen() error: ");
        return false;
    }

    noError(error);
    return true;
}

BaseCrossSocket* BaseCrossSocket::accept(CrossSocketErrors *error)
{
    int remoteSocketHandle = -1;
    sockaddr remoteAddress;

    if (!waitRead(error))
    {
        return NULL;
    }
	
    sw_socklen_t socketSize = sizeof(sockaddr);

    if ((remoteSocketHandle = ::accept(_socketHandle, &remoteAddress, &socketSize)) == int(SOCKETFAIL))
    {
        handleError(error, "BaseCrossSocket::accept() error: ");
        return NULL;
    }
	
    // Check if we are in non blocking mode
    if (_blockingMode == nonblocking)
    {
        fcntl(remoteSocketHandle, F_SETFL, O_NONBLOCK);
    }

   // Create a new class of the BaseCrossSocket
    BaseCrossSocket* clientSocket = create(remoteSocketHandle, error);
         
    noError(error);
    
    stringstream clientInformation;
    clientInformation       << ((CrossSocket*)clientSocket)->getClientAddress() << " is connected!\n"
                            << ((CrossSocket*)clientSocket)->getClientAddress() << " : " << ((CrossSocket*)clientSocket)->getClientPort() << "\n";
    
    cout << clientInformation.str();
    
    return clientSocket;
}

bool BaseCrossSocket::checkPassword(BaseCrossSocket *socket)
{
    if (_password.length() > 0)
    {
        socket->writeLine("This server is protected with a password, please insert the password now.");
        
        int passwordCounter = 0;
        string givenPassword;
        while (givenPassword != _password)
        {
            // Just read 50 bytes for the password
            givenPassword = socket->readLine(50);
            if (givenPassword != _password)
            {
                socket->writeLine("Incorrect password given.");
                passwordCounter++;
                if (passwordCounter >= 5)
                {
                    socket->writeLine("Too many attempts where made to enter the correct password.");
                    socket->writeLine("Have a nice day!");
                    socket->writeLine("Disconnecting...");
                    socket->writeLine("403");
                    disconnect();
                    return false;
                }
            }
        }
    }
    socket->writeLine("The password is correct. Access granted");
    return true;
}

bool BaseCrossSocket::isConnected()
{
    return _socketHandle > 0;
}

bool BaseCrossSocket::disconnect(CrossSocketErrors *error)
{
    int i = 0;
    char buffer[256];

    if (_socketHandle < 0)
    {
        setError(error, notConnected, "BaseCrossSocket::disconnect() - No connection");
        return false;
    }

    // check if the client closed the connection 
    if (shutdown(_socketHandle, 1) != 0)
    {
        handleError(error, "BaseCrossSocket::disconnect() error: ");
        return false;
    }
	
    CrossSocketErrors crossSocketError;

    // Check if we got a signal from the client that he closed the connection
    if (!_closeSignal)
    {
        while (true)
        {
            if (!waitRead(error))
            {
                return false;
            }

            i = readLine(buffer, 256, &crossSocketError);

            if (i <= 0)
            {
                break;
            }
            
            // If we do not want to wait we need to throw an error
            if (_blockingMode == noWait)
            {
                setError(error, notReady, "BaseCrossSocket::disconnect() - We need more time, please call again");
                return false;
            }
        }
    }

    if (i != 0)
    {
        setError(error, crossSocketError, _error);
        return false;
    }

    // Reset the state
    reset();

    close(_socketHandle);
    _socketHandle = -1;

    noError(error);
    cout << "CrossSockets: Client has been disconnected!\n";
    return true;
}

bool BaseCrossSocket::closeSocketHandle()
{
    if (_socketHandle > 0)
    {
        close(_socketHandle);
        _socketHandle = -1;

        //reset state
        reset();
        
        return true;
    }
    return false;
}

int BaseCrossSocket::writeLine(string message, CrossSocketErrors *error)
{
    //TODO: Why cant we append the data?
    //int returnValue = send(_socketHandle, message.append("\r\n").data(), strlen(message.data()), 0);
    int returnValue = send(_socketHandle, message.data(), message.size(), 0);
    send(_socketHandle, "\r\n", 2, 0);
    return returnValue;
}

int BaseCrossSocket::writeLine(string message, size_t length, CrossSocketErrors *error)
{	
    int returnValue = send(_socketHandle, message.data(), length, 0);
    send(_socketHandle, "\r\n", 2, 0);
    return returnValue;     
}

int BaseCrossSocket::write(const string message, size_t length, CrossSocketErrors *error)
{	
    return send(_socketHandle, message.data(), length, 0);
}

int BaseCrossSocket::write(const string message, CrossSocketErrors *error)
{	
    return send(_socketHandle, message.data(), message.size(), 0);
}

void BaseCrossSocket::setPassword(const string password)
{
   _password = password;
}

ssize_t BaseCrossSocket::read(char *buffer, CrossSocketErrors *error)
{
    ssize_t returnValue = 0;
    char singleCharacter;

    if (_socketHandle < 0)
    {
        setError(error, notConnected, "BaseCrossSocket::read() - No connection");
        return -1;
    }

    if (!waitRead(error))
    {
        return -1;
    }
    
    while (size_t i = recv(_socketHandle, buffer + returnValue, int(sizeof(buffer) - returnValue), 0))
    {
        if (i == 0)
        {
            break;
        }
        returnValue += i;
        if (returnValue > sizeof(buffer) -1)
        {
            break;
        }
    }
    return returnValue;
}

ssize_t BaseCrossSocket::read(char *buffer, int bytes, CrossSocketErrors *error)
{
    ssize_t returnValue = 0;
    char singleCharacter;

    if (_socketHandle < 0)
    {
        setError(error, notConnected, "BaseCrossSocket::read() - No connection");
        return -1;
    }

    if (!waitRead(error))
    {
        return -1;
    }
    
    while (size_t i = recv(_socketHandle, buffer + returnValue, int(bytes - returnValue), 0))
    {
        if (i == 0)
        {
            break;
        }
        returnValue += i;
        if (returnValue > bytes -1)
        {
            break;
        }
    }
    return returnValue;
}

string BaseCrossSocket::readLine(int size, CrossSocketErrors *error)
{
    char *buffer = new char[size];
    ssize_t returnValue = 0;
    char singleCharacter;

    if (_socketHandle < 0)
    {
        setError(error, notConnected, "BaseCrossSocket::readLine() - No connection");
    }

    if (!waitRead(error))
    {
        return string();
    }
    
    while (size_t i = recv(_socketHandle, &singleCharacter, 1, 0))
    {  
        if (returnValue >= size || singleCharacter == '\n')
        {
            break;
        }        
        
        if (singleCharacter != '\r') 
        {
            buffer[returnValue++] = singleCharacter;
        }
    }    
    buffer[returnValue] = '\0';    
    
    if (returnValue < 0)
    {
        handleError(error, "BaseCrossSocket::readLine() error: ");
    }
    else if (returnValue == 0)
    {
        _closeSignal = true;  //we  received a close signal from client
        setError(error, terminated, "BaseCrossSocket::readLine() - Connection terminated by client");	
    }
    else
    {
        noError(error);
    }
    return string(buffer);
}

ssize_t BaseCrossSocket::readLine(char *buffer, int bytes, CrossSocketErrors *error)
{
    ssize_t returnValue = 0;
    char singleCharacter;

    if (_socketHandle < 0)
    {
        setError(error, notConnected, "BaseCrossSocket::readLine() - No connection");
        return -1;
    }

    if (!waitRead(error))
    {
        return -1;
    }
    
    while (size_t i = recv(_socketHandle, &singleCharacter, 1, 0))
    {  
        if (returnValue >= sizeof(buffer) || singleCharacter == '\n')
        {
            break;
        }        
        
        if (singleCharacter != '\r') 
        {
            buffer[returnValue++] = singleCharacter;
        }
    }  
    buffer[returnValue] = '\0';
    
    if (returnValue < 0)
    {
        handleError(error, "BaseCrossSocket::readLine() error: ");
    }
    else if (returnValue == 0)
    {
        _closeSignal = true;  //we  received a close signal from client
        setError(error, terminated, "BaseCrossSocket::readLine() - Connection terminated by client");	
    }
    else
    {
        noError(error);
    }
    return returnValue;
}

int BaseCrossSocket::getClientSocket(CrossSocketErrors *error)
{
    if (_socketHandle > 0)
    {
        noError(error);
        return _socketHandle;
    }

    setError(error, notConnected, "BaseCrossSocket::getClientSocket() - No descriptor");		
    return -1;
}

bool BaseCrossSocket::getServerHost(sockaddr *host, CrossSocketErrors *error)
{
    if (host == NULL)
    {
        setError(error, fatal, "BaseCrossSocket::getServerHost() - Got NULL pointer");
        return false;
    }

    if (_socketHandle < 0)
    {
        setError(error, notConnected, "BaseCrossSocket::getServerHost() - No socket");
        return false;
    }

    sw_socklen_t temp = sizeof(sockaddr);
    if (getsockname(_socketHandle, host, &temp) != 0)
    {
        handleError(error, "BaseCrossSocket::getServerHost() error: ");
        return false;
    }

    noError(error);
    return true;
}

void BaseCrossSocket::setTimeout(unsigned int seconds, unsigned int miliSeconds)
{
    _timeoutSeconds = seconds;
    _timeoutMicroSeconds = miliSeconds;
}

bool BaseCrossSocket::getClientHost(sockaddr *client, CrossSocketErrors *error)
{
    if (client == NULL)
    {
        setError(error, fatal, "BaseCrossSocket::getClientHost() - Got NULL pointer");
        return false;
    }

    if (_socketHandle > 0)
    {
        sw_socklen_t temp = sizeof(sockaddr);
        
        if (getpeername(_socketHandle, client, &temp) != 0)
        {
            handleError(error, "BaseCrossSocket::getClientHost() error: ");
            return false;
        }
    }
    else
    {
        setError(error, notConnected, "BaseCrossSocket::getClientHost() - No connection");
        return false;
    }

    noError(error);
    return true;
}

void BaseCrossSocket::reset()
{
    _closeSignal = false;	
}

bool BaseCrossSocket::waitIO(ioTypeEnum &type, CrossSocketErrors *error)
{
    if (_blockingMode != blocking)
    {
        noError(error);
        return true;
    }

    // Wait with select() even if no timeout is set

    timeval time; 
    timeval *timeForever = NULL;  // Forever waiting
    time.tv_sec = _timeoutSeconds;
    time.tv_usec = _timeoutMicroSeconds;

    if (_timeoutSeconds > 0 || _timeoutMicroSeconds > 0)
    {
        timeForever = &time;
    }

    fd_set readFileDescriptorSet;
    fd_set writeFileDescriptorSet;
    fd_set exceptionFileDescriptorSet;

    FD_ZERO     (&readFileDescriptorSet);
    FD_ZERO     (&writeFileDescriptorSet);
    FD_ZERO     (&exceptionFileDescriptorSet);
    FD_SET      (_socketHandle, &readFileDescriptorSet);
    FD_SET      (_socketHandle, &writeFileDescriptorSet);
    FD_SET      (_socketHandle, &exceptionFileDescriptorSet);

    int returnValue = 0;

    switch (type)
    {
        case ioread:
            returnValue = select(_socketHandle+1, &readFileDescriptorSet, NULL, NULL, timeForever);
            break;
        case iowrite:
            returnValue = select(_socketHandle+1, NULL, &writeFileDescriptorSet, NULL, timeForever);
            break;
        case ioexception:
            returnValue = select(_socketHandle+1, NULL, NULL, &exceptionFileDescriptorSet, timeForever);
            break;
        case ioreadWrite:
            returnValue = select(_socketHandle+1, &readFileDescriptorSet, &writeFileDescriptorSet, NULL, timeForever);
            break;
        case ioall:
            returnValue = select(_socketHandle+1, &readFileDescriptorSet, &writeFileDescriptorSet, &exceptionFileDescriptorSet, timeForever);
            break;
    }

    if (returnValue == 0)
    {
        setError(error, timeout, "BaseCrossSocket::waitIO() timeout");
        return false;
    } 
    else if (returnValue < 0)
    {
        handleError(error, "BaseCrossSocket::waitIO() error: ");
        return false;
    }

    if (FD_ISSET(_socketHandle, &readFileDescriptorSet))
    {
        noError(error);
        type = ioread;
        return true;
    }
    
    if (FD_ISSET(_socketHandle, &writeFileDescriptorSet))
    {
        noError(error);
        type = iowrite;
        return true;
    }
    
    if (FD_ISSET(_socketHandle, &exceptionFileDescriptorSet))
    {
        noError(error);
        type = ioexception;
        return true;
    }

    setError(error, fatal, "BaseCrossSocket::waitIO() failed on select()");
    return false;
}

bool BaseCrossSocket::waitRead(CrossSocketErrors *error)
{
    ioTypeEnum temp = ioread;
    return waitIO(temp, error);
}

bool BaseCrossSocket::waitWrite(CrossSocketErrors *error)
{
    ioTypeEnum temp = iowrite;
    return waitIO(temp, error);
}

string BaseCrossSocket::getError()
{
   return _error;
}

void BaseCrossSocket::printError()
{
    if (_error.size() > 0)
    {
        fprintf(stderr, "CrossSocket error:\n%s!\n", _error.data());
    }
}

void BaseCrossSocket::handleError(CrossSocketErrors *error, string message)
{
    #ifdef __WIN32__
        // Winsock2 errorList (taken from winsock2.h)
        switch (WSAGetLastError())
        {
            case 0:                   message += "No error"; break;
            case WSAEINTR:            message += "Interrupted system call"; break;
            case WSAEBADF:            message += "Bad file number"; break;
            case WSAEACCES:           message += "Permission denied"; break;
            case WSAEFAULT:           message += "Bad address"; break;
            case WSAEINVAL:           message += "Invalid argument"; break;
            case WSAEMFILE:           message += "Too many open sockets"; break;
            case WSAEWOULDBLOCK:      message += "Operation would block"; break;
            case WSAEINPROGRESS:      message += "Operation now in progress"; break;
            case WSAEALREADY:         message += "Operation already in progress"; break;
            case WSAENOTSOCK:         message += "Socket operation on non-socket"; break;
            case WSAEDESTADDRREQ:     message += "Destination address required"; break;
            case WSAEMSGSIZE:         message += "Message too long"; break;
            case WSAEPROTOTYPE:       message += "Protocol wrong type for socket"; break;
            case WSAENOPROTOOPT:      message += "Bad protocol option"; break;
            case WSAEPROTONOSUPPORT:  message += "Protocol not supported"; break;
            case WSAESOCKTNOSUPPORT:  message += "Socket type not supported"; break;
            case WSAEOPNOTSUPP:       message += "Operation not supported on socket"; break;
            case WSAEPFNOSUPPORT:     message += "Protocol family not supported"; break;
            case WSAEAFNOSUPPORT:     message += "Address family not supported"; break;
            case WSAEADDRINUSE:       message += "Address already in use"; break;
            case WSAEADDRNOTAVAIL:    message += "Can't assign requested address"; break;
            case WSAENETDOWN:         message += "Network is down"; break;
            case WSAENETUNREACH:      message += "Network is unreachable"; break;
            case WSAENETRESET:        message += "Net connection reset"; break;
            case WSAECONNABORTED:     message += "Software caused connection abort"; break;
            case WSAECONNRESET:       message += "Connection reset by client"; break;
            case WSAENOBUFS:          message += "No buffer space available"; break;
            case WSAEISCONN:          message += "Socket is already connected"; break;
            case WSAENOTCONN:         message += "Socket is not connected"; break;
            case WSAESHUTDOWN:        message += "Can't send after socket shutdown"; break;
            case WSAETOOMANYREFS:     message += "Too many references"; break;
            case WSAETIMEDOUT:        message += "Connection timed out"; break;
            case WSAECONNREFUSED:     message += "Connection refused"; break;
            case WSAELOOP:            message += "Too many levels of symbolic links"; break;
            case WSAENAMETOOLONG:     message += "File name too long"; break;
            case WSAEHOSTDOWN:        message += "Host is down"; break;
            case WSAEHOSTUNREACH:     message += "No route to host"; break;
            case WSAENOTEMPTY:        message += "Directory not empty"; break;
            case WSAEPROCLIM:         message += "Too many processes"; break;
            case WSAEUSERS:           message += "Too many users"; break;
            case WSAEDQUOT:           message += "Disc quota exceeded"; break;
            case WSAESTALE:           message += "Stale NFS file handle"; break;
            case WSAEREMOTE:          message += "Too many levels of remote in path"; break;
            case WSASYSNOTREADY:      message += "Network system is unavailable"; break;
            case WSAVERNOTSUPPORTED:  message += "Winsock version out of range"; break;
            case WSANOTINITIALISED:   message += "WSAStartup not yet called"; break;
            case WSAEDISCON:          message += "Graceful shutdown in progress"; break;
            case WSAHOST_NOT_FOUND:   message += "Host not found"; break;
            case WSANO_DATA:          message += "No host data of that type was found"; break;
            default:                  message += "Unknown Winsock error: " + WSAGetLastError(); break;
        }
    #else
        message += strerror(errno);
    #endif
	
    int errorNumber;

    #ifdef __WIN32__
      errorNumber = WSAGetLastError();
    #else
      errorNumber = errno;
    #endif

    CrossSocketErrorsEnum errorMessage;

    if (errorNumber == EADDRINUSE)
    {
        errorMessage = portInUse;
    }
    else if (errorNumber == EAGAIN || errorNumber == EWOULDBLOCK)
    {
        errorMessage = notReady;
    }
    else if (errorNumber == EMSGSIZE)
    {
        errorMessage = messageTooLong;
    }
    else if (errorNumber == EINPROGRESS || errorNumber == EALREADY)
    {
        errorMessage = notReady;
    }
    else if (errorNumber == ECONNREFUSED || errorNumber == ETIMEDOUT)
    {
        errorMessage = noResponse;
    }
    else if (errorNumber == ENOTCONN || errorNumber == EBADF || errorNumber == ENOTSOCK)
    {
        errorMessage = notConnected;
    }
    else if (errorNumber == EPIPE)
    {
        errorMessage = terminated;
        _closeSignal = true;
    }
    else if (errorNumber == EINTR)
    {
        errorMessage = interrupted;
    }
    else
    {
        errorMessage = fatal; //default
    }

    setError(error, errorMessage, message);
}

void BaseCrossSocket::noError(CrossSocketErrors *error)
{
    if (error != NULL)
    {
        *error = ok;
        error->_error = "";
        error->failed_class = NULL;
    }
}

void BaseCrossSocket::setError(CrossSocketErrors *error, CrossSocketErrors name, string message)
{
    _error = message;

    if (error != NULL)
    {
        *error = name;
        error->_error = message;
        error->failed_class = this;
    }
    else
    {
        if (ERRORMODE == 0)
        {
            printError();
            if (name == terminated)
            {
                disconnect();
            }
        }
        else if (ERRORMODE == 1)
        {
            // Reset the state
            reset();

            close(_socketHandle);
            _socketHandle = -1;            
            
            CrossSocketErrors crossSocketError;
            crossSocketError = name;
            crossSocketError._error = message;
            crossSocketError.failed_class = this;
            throw crossSocketError;
        }
        else
        {
            exit(-1);	
        }
   }
}
