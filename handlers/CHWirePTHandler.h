#ifndef CH_WIRE_PROTOCOL_HANDLER_H
#define CH_WIRE_PROTOCOL_HANDLER_H

#include "ServerSocket.h"
#include "CProxyHandler.h"

class CHWirePTHandler : public CProxyHandler {

public:
    CHWirePTHandler() {}
    virtual void * HandleUpstreamData(void *Buffer, int len, SocketClient * target_socket);
    virtual void * HandleDownStreamData(void *Buffer, int len);
};

#endif