// CrossSockets
// Copyright Jeroen Saey
// Created 27-01-2013
// CrossSocketErrors.cpp

#include "BaseCrossSocket.h"

using namespace std;

CrossSocketErrors::CrossSocketErrors()
{
    be = ok;
    _error = "";
    failed_class = NULL;
}

CrossSocketErrors::CrossSocketErrors(CrossSocketErrorsEnum e)
{
    be = e;
    _error = "";
    failed_class = NULL;
}

string CrossSocketErrors::getError()
{
    return _error;
}

BaseCrossSocket* CrossSocketErrors::getFailedClass(void)
{
    return failed_class;
}

void CrossSocketErrors::setErrorString(string msg)
{ 
    _error = msg;
}

void CrossSocketErrors::setFailedClass(BaseCrossSocket *pnt)
{
    failed_class = pnt;
}

bool CrossSocketErrors::operator==(CrossSocketErrors e)
{
    return be == e.be;
}
		
bool CrossSocketErrors::operator!=(CrossSocketErrors e)
{
    return be != e.be;
}
