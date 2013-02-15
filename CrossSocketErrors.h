// CrossSockets
// Copyright Jeroen Saey
// Created 27-01-2013
// CrossSocketErrors.h

#pragma once

#include <string>

enum CrossSocketErrorsEnum
{
    ok,                 // Operation successful
    fatal,              // Unspecified error
    notReady,           // The socket was not ready (blockMode)
    portInUse,          // The specified port is already in use
    notConnected,       // The socket is invalid or not connected
    messageTooLong,     // The messageSize is too big to be send
    terminated,         // Connection terminated (by client)
    noResponse,         // Cannot connect to client
    timeout,            // Read/Write operation timeout occurred (in blockingMode)
    interrupted         // Operation was blocked by signal
};

class BaseCrossSocket;

class CrossSocketErrors
{
public:
    CrossSocketErrors();
    CrossSocketErrors(CrossSocketErrorsEnum e);

    virtual ~CrossSocketErrors(){;}

    virtual std::string getError();
    virtual BaseCrossSocket* getFailedClass(void);

    virtual bool operator == (CrossSocketErrors e);
    virtual bool operator != (CrossSocketErrors e);

    virtual void setErrorString(std::string msg);
    virtual void setFailedClass(BaseCrossSocket *pnt);
protected:
    friend class BaseCrossSocket;

    // The base error type
    CrossSocketErrorsEnum be;

    // Human readable error string
    std::string _error;

    // A pointer to the class causing the error
    BaseCrossSocket *failed_class;
};
