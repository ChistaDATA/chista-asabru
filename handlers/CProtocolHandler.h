#ifndef PROTOCOL_HANDLER_H
#define PROTOCOL_HANDLER_H

#include "ServerSocket.h"

class CProtocolHandler
{
public:
    CProtocolHandler() {}

    virtual bool Handler(void *Buffer, int len, CLIENT_DATA &clientData) = 0;

    virtual ~CProtocolHandler() {}
};

#endif