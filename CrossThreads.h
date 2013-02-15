// CrossThreads
// Copyright Jeroen Saey
// Created 29-01-2013
// CrossThreads.h

#pragma once

#ifndef __WIN32__
        
#include "pthread.h"

typedef pthread_t threadID;
typedef void* (*threadCallBack)(void* parameter);

#else

#include <windows.h>
typedef unsigned long (*threadCallBack)(void* parameter);
typedef DWORD threadID;

#endif

class CrossThreads
{
    public:
        
        CrossThreads(threadCallBack callBack = NULL, long timeout = 3000);
        virtual ~CrossThreads();

        void setThreadCallback(threadCallBack callBack); 
        unsigned long getThreadID();
        int create();
        int remove();
        void join();
        bool isCreated();
    
    private:
        threadID		_threadID;
        threadCallBack		_callBack;

    #ifdef __WIN32__
            long			_timeout;
            HANDLE			_thread;
    #endif

        void stop();
};
