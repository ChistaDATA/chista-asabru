#ifndef POSTGRESQL_PROTOCOL_HANDLER_H
#define POSTGRESQL_PROTOCOL_HANDLER_H

#include "ServerSocket.h"
#include "CProxyHandler.h"

class CPostgreSQLHandler : public CProxyHandler
{

public:
    CPostgreSQLHandler() {}
    virtual void * HandleUpstreamData(void *Buffer, int len, SocketClient * target_socket);
    virtual void * HandleDownStreamData(void *Buffer, int len);
};

#endif