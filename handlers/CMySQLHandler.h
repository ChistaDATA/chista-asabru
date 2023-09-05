#ifndef CH_MYSQL_PROTOCOL_HANDLER_H
#define CH_MYSQL_PROTOCOL_HANDLER_H

#include "ServerSocket.h"
#include "CProxyHandler.h"

class CMySQLHandler : public CProxyHandler
{

public:
    CMySQLHandler() {}
    virtual void * HandleUpstreamData(void *Buffer, int len, SocketClient * target_socket);
    virtual void * HandleDownStreamData(void *Buffer, int len, Socket * client_socket);
};

#endif