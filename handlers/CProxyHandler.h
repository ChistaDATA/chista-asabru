#ifndef PROXY_HANDLER_H
#define PROXY_HANDLER_H

#include "ServerSocket.h"

class CProxyHandler
{
public:
    CProxyHandler() {}

    virtual void * HandleUpstreamData(void *buffer, int buffer_length, SocketClient * target_socket) = 0;

    virtual void * HandleDownStreamData(void *buffer, int buffer_length, Socket * client_socket) = 0;

    virtual ~CProxyHandler() {}
};

#endif