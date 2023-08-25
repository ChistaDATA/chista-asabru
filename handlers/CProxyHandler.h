#ifndef PROXY_HANDLER_H
#define PROXY_HANDLER_H

#include "ServerSocket.h"

class CProxyHandler
{
public:
    CProxyHandler() {}

    virtual bool HandleUpstreamData(void *Buffer, int len, CLIENT_DATA &clientData) = 0;

    virtual bool HandleDownStreamData(void *Buffer, int len, CLIENT_DATA &clientData) = 0;

    virtual ~CProxyHandler() {}
};

#endif