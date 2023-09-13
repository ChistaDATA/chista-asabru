#ifndef CH_MYSQL_PROTOCOL_HANDLER_H
#define CH_MYSQL_PROTOCOL_HANDLER_H

#include "ServerSocket.h"
#include "CProxyHandler.h"

class CMySQLHandler : public CProxyHandler
{

public:
    CMySQLHandler() {}
    virtual void * HandleUpstreamData(void *Buffer, int len, uv_stream_t * target);
    virtual void * HandleDownStreamData(void *Buffer, int len, uv_stream_t *client);
};

#endif