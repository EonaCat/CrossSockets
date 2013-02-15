// CrossSockets
// Copyright Jeroen Saey
// Created 27-01-2013
// BaseCrossSocket.h
 
#pragma once

#include "CrossSocketsCore.h"
#include "CrossSocketConfig.h"
#include "CrossSocketErrors.h"

#include <unistd.h>
#include <string>

class CrossThreadsHandler;
class CrossSocket;

class BaseCrossSocket
{
public:	
	BaseCrossSocket();
	virtual ~BaseCrossSocket();

        // Blocking     :       Everything blocks until operation is done
        // noWait       :       First call time blocking only
        // nonBlocking  :       Allow everything simultaneously
	enum blockTypeEnum
        {
            blocking, 
            noWait, 
            nonblocking
        };
	
        // wait for I/O (with timeout)
	enum ioTypeEnum
        {
            ioread, 
            iowrite, 
            ioexception, 
            ioreadWrite, 
            ioall
        };
        
	virtual bool listen(int connectionLimit = 5, CrossSocketErrors* = NULL);
	virtual BaseCrossSocket* accept(CrossSocketErrors* = NULL);
	
        virtual bool isConnected();
	virtual bool disconnect(CrossSocketErrors* = NULL);
        virtual bool checkPassword(BaseCrossSocket *socket);
	
	// force to close the socket
	virtual bool closeSocketHandle();

        virtual int write(const std::string message, CrossSocketErrors *error = NULL);
        virtual int write(const std::string message, size_t length, CrossSocketErrors *error = NULL);        
	virtual int writeLine(std::string message, CrossSocketErrors *error = NULL);
        virtual int writeLine(std::string message, size_t length, CrossSocketErrors *error = NULL);
        virtual ssize_t read(char *buffer, int bytes, CrossSocketErrors *error = NULL);
        virtual ssize_t read(char *buffer, CrossSocketErrors *error = NULL);
	virtual ssize_t readLine(char *buffer, int bytes, CrossSocketErrors *error = NULL);
        virtual std::string readLine(int size, CrossSocketErrors *error = NULL);
        virtual void setPassword(const std::string password);
	
	virtual int getClientSocket(CrossSocketErrors *error);
	virtual bool getServerHost(sockaddr *host, CrossSocketErrors *error = NULL);
	virtual bool getClientHost(sockaddr *client, CrossSocketErrors *error = NULL);
	
        // Receive timeout (can only be used in blocking mode)
	void setTimeout(unsigned int seconds, unsigned int miliseconds);
	
	// Error handling
	virtual void printError();
	virtual std::string getError();

protected:
	virtual void getSocket() = 0;
	virtual BaseCrossSocket* create(int socketdescriptor, CrossSocketErrors *error) = 0;
	virtual void reset();
        
	virtual bool waitIO(ioTypeEnum &type, CrossSocketErrors *error);
	bool waitRead(CrossSocketErrors *error);
	bool waitWrite(CrossSocketErrors *error);
        
	virtual void handleError(CrossSocketErrors *error, std::string message);
	virtual void noError(CrossSocketErrors *error);
	virtual void setError(CrossSocketErrors *error, CrossSocketErrors name, std::string message);

	int _socketHandle;
        std::string _password;
        
        BaseCrossSocket *_clientSocket;
	
	// last error
	std::string _error;

	// have we received a shutdown signal?
	bool _closeSignal;

	// blocking mode
	blockTypeEnum _blockingMode;
        
        CrossThreadsHandler     *_threadHandler;
        
	// timeout for waitIO()
	int _timeoutSeconds;
	int _timeoutMicroSeconds;
};
