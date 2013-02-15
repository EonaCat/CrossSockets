// CrossThreadsHandler
// Copyright Jeroen Saey
// Created 29-01-2013
// CrossThreadsHandler.h

#pragma once

#include "CrossThreads.h"

#include <string>
#include <map>

class CrossThreadsHandler 
{
    public:
        CrossThreadsHandler();
        virtual ~CrossThreadsHandler();

        unsigned long         createAndStartThread      (threadCallBack callback);
        bool                  removeThread              (unsigned long threadID);
        bool                  joinThread                (unsigned long threadID);

    private:
        std::map<unsigned long, CrossThreads*>          _threadList;
};
