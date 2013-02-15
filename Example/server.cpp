/* 
 * File:   Server.cpp
 * Author: SuperSmash
 * 
 * Created on 15 february 2013, 21:10
 */

#include "Server.h"
#include "CrossSockets/CrossThreadsHandler.h"

#include <sstream>

using namespace std;

static Server *self;

Server::Server() 
{       
    self = this;
    
    // Setup the listener into listeningMode and define the serverport
    _server.bind(2013);
    _server.listen();
    _server.setPassword("welcome");
    
    stringstream startMessage;
    LogManagement::getInstance()->write(startMessage << "CrossSocket server started.\n" << "PORT: 2013");
    cout << APPLICATIONNAME << " server started.\n";

    _threadHandler = new CrossThreadsHandler();

    while (_server.isConnected())
    {
        listenForConnections((CrossSocket*)_server.accept());        
    }    
}

void Server::listenForConnections(CrossSocket* socket)
{
    _clientSocket = socket;
    stringstream welcomeMessage;
    welcomeMessage << "Welcome to the CrossSocket server!";
    socket->writeLine(welcomeMessage.str());
    _threadID = _threadHandler->createAndStartThread((threadCallBack) &Server::handleLoopCallback);
}

void* Server::handleLoopCallback(void *functionPointer)
{
    static_cast <Server*>(functionPointer)->handleLoop(self->_clientSocket, self->_threadID);
}

void Server::handleLoop(CrossSocket *socket, unsigned long threadID)
{
    if (self->_server.checkPassword(socket))
    {    
        while (socket->isConnected())
        {
            // Do something here because we got an active connection to the server using CrossSockets ^^
        }
        delete socket;
    }
}

Server::~Server() 
{
}
