// CrossSockets
// Copyright Jeroen Saey
// Created 27-01-2013
// CrossSocket.h

#pragma once

#include "CrossSocketsCore.h"
#include "BaseCrossSocket.h"
#include <string>

class CrossSocket : public BaseCrossSocket
{
public:
	CrossSocket(blockTypeEnum block = blocking);
	virtual ~CrossSocket();
	
	virtual bool bind(int port, CrossSocketErrors *error = NULL);  //use port=0 to get any free port
	virtual bool bind(int port, std::string host, CrossSocketErrors *error = NULL); //you can also specify the host interface to use
	virtual bool connect(int port, std::string hostname, CrossSocketErrors *error = NULL);
	
	// Tools
	// Gets IP address, name or port.
	virtual std::string getClientAddress(CrossSocketErrors *error = NULL);
	virtual int getClientPort(CrossSocketErrors *error = NULL);
	virtual std::string getClientHostName(CrossSocketErrors *error = NULL);
	virtual std::string getServerAddress(CrossSocketErrors *error = NULL);
	virtual int getServerPort(CrossSocketErrors *error = NULL);
	virtual std::string getServerHostName(CrossSocketErrors *error = NULL);
	
protected:	
	virtual void getSocket();
	virtual BaseCrossSocket* create(int socketdescriptor, CrossSocketErrors *error);
};
