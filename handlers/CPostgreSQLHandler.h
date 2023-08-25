#ifndef POSTGRESQL_PROTOCOL_HANDLER_H
#define POSTGRESQL_PROTOCOL_HANDLER_H

#include "ServerSocket.h"
#include "CProxyHandler.h"

class CPostgreSQLHandler : public CProxyHandler
{

public:
    CPostgreSQLHandler() {}
    virtual bool HandleUpstreamData(void *Buffer, int len, CLIENT_DATA &clientData);
    virtual bool HandleDownStreamData(void *Buffer, int len, CLIENT_DATA &clientData);
};

#endif