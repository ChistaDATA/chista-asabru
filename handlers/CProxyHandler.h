#ifndef PROXY_HANDLER_H
#define PROXY_HANDLER_H

#include "ServerSocket.h"

class CProxyHandler
{
public:
    CProxyHandler() {}

    virtual void * HandleUpstreamData(void *buffer, int buffer_length, uv_stream_t *target) = 0;

    virtual void * HandleDownStreamData(void *buffer, int buffer_length, uv_stream_t *client) = 0;

    virtual ~CProxyHandler() {}
};

#endif