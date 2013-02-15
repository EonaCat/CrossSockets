// CrossThreads
// Copyright Jeroen Saey
// Created 29-01-2013
// CrossThread.cpp

#include "CrossThreads.h"
#include "errno.h"
#include <iostream>

CrossThreads::CrossThreads(threadCallBack callBack, long timeout) : _callBack(callBack), _threadID(0)
#ifdef __WIN32__ 
, _timeout(timeout) , _thread(NULL) 
#endif
{
}

CrossThreads::~CrossThreads()
{
    stop();	
}

void CrossThreads::stop()
{
    #ifdef __WIN32__
        //waiting for the thread to terminate
        if (_thread) 
        {
            if (WAIT_TIMEOUT == ::WaitForSingleObject (_thread, _timeout))
                    ::TerminateThread (_thread, 1);

            ::CloseHandle (_thread);
        }
    #endif
}

int CrossThreads::create()
{
	
#ifdef __WIN32__
    _thread = ::CreateThread (NULL, 0, 
            (unsigned long (__stdcall *)(void *))_callBack, 
            NULL, 0, &_thread);
    if (NULL == _thread)
    {
        break;	
    }
#endif
            
    pthread_attr_t attribute;

    // Initialize and set thread and set the attribute
    pthread_attr_init(&attribute);
    pthread_attr_setdetachstate(&attribute, PTHREAD_CREATE_DETACHED);				
    
    int result;
    do 
    {
        result = pthread_create(&_threadID, &attribute, _callBack, (void *)this);
    } while (false);
    
    if (result != 0)
    {
        std::cout << "CrossThread: Failed to create the thread\n";
        if (result == EAGAIN)
        {
            std::cout << "The system lacked the necessary resource to create another thread";
        }
        else if (result == EINVAL)
        {
            std::cout << "CrossThread: The value given in the arguments is invalid!";
        }
        else
        {
            std::cout << "CrossThread: The caller does not have the appropriate rights";
        }
    }
    
    // Free the attributes of the thread
    pthread_attr_destroy(&attribute);
    return result;
}

void CrossThreads::join()
{
    if (0 != pthread_join(_threadID, NULL)) 
    {
       fprintf(stderr, "pthread_join error\n");
    }   
}

int CrossThreads::remove()
{
    return pthread_detach(_threadID);
} 

unsigned long CrossThreads::getThreadID()
{
    return printf("%lu\n", (unsigned long) _threadID);
}

void CrossThreads::setThreadCallback(threadCallBack callBack)
{
    _callBack = callBack;
}

bool CrossThreads::isCreated()
{
    return _threadID ? true : false;
}
