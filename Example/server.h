/* 
 * File:   Server.h
 * Author: SuperSmash
 *
 * Created on 15 february 2013, 21:10
 */

#pragma once

#include "CrossSockets/CrossSockets.h"
#include "CrossSockets/CrossThreads.h"

class CrossThreadsHandler;

class Server 
{
    public:
        Server();
        virtual ~Server();
        
        void listenForConnections(CrossSocket* socket);
        void deleteClientSocketWithForce(int signalnumber);
        
    private:
        static void* handleLoopCallback(void *functionPointer);
        void handleLoop(CrossSocket *socket, unsigned long threadID);
        
        CrossSocket             _server;
        CrossThreadsHandler     *_threadHandler;    
        
        CrossSocket *_clientSocket;
        unsigned long _threadID;       
};
