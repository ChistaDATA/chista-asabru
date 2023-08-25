#ifndef CH_WIRE_PROTOCOL_HANDLER_H
#define CH_WIRE_PROTOCOL_HANDLER_H

#include "ServerSocket.h"
#include "CProxyHandler.h"

class CHWirePTHandler : public CProxyHandler {

public:
    CHWirePTHandler() {}
    virtual bool HandleUpstreamData(void *Buffer, int len, CLIENT_DATA &clientData);
    virtual bool HandleDownStreamData(void *Buffer, int len, CLIENT_DATA &clientData);
};

#endif