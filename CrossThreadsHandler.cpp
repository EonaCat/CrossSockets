// CrossThreadsHandler
// Copyright Jeroen Saey
// Created 29-01-2013
// CrossThreadsHandler.cpp

#include "CrossThreadsHandler.h"

#include <sstream>
#include <iostream>
#include <errno.h>

using namespace std;

CrossThreadsHandler::CrossThreadsHandler() 
{
}

unsigned long CrossThreadsHandler::createAndStartThread(threadCallBack callback)
{        
    CrossThreads *thread = new CrossThreads(callback);
    thread->create();
    _threadList.insert(pair<unsigned long, CrossThreads*>(thread->getThreadID(),thread));
    return thread->getThreadID();
}

bool CrossThreadsHandler::joinThread(unsigned long threadID)
{
   map <unsigned long, CrossThreads*>::iterator iterator = _threadList.find(threadID);
    if (iterator != _threadList.end())
    {
        iterator->second->join();
        return true;
    }
    else
    {
        cout << "The client could not be found in the threadList.";
        return false;
    }
}

bool CrossThreadsHandler::removeThread(unsigned long threadID)
{
    map <unsigned long, CrossThreads*>::iterator iterator = _threadList.find(threadID);
    if (iterator != _threadList.end())
    {
        _threadList.erase(iterator);
        return true;
    }
    else
    {
        cout << "The client could not be found in the threadList.";
        return false;
    }
}

CrossThreadsHandler::~CrossThreadsHandler() 
{
}
