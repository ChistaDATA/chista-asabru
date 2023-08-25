#ifndef CH_MYSQL_PROTOCOL_HANDLER_H
#define CH_MYSQL_PROTOCOL_HANDLER_H

#include "ServerSocket.h"
#include "CProxyHandler.h"

class CMySQLHandler : public CProxyHandler
{

public:
    CMySQLHandler() {}
    virtual bool HandleUpstreamData(void *Buffer, int len, CLIENT_DATA &clientData);
    virtual bool HandleDownStreamData(void *Buffer, int len, CLIENT_DATA &clientData);
};

#endif