#ifndef PING_HANDLER_DOT_H
#define PING_HANDLER_DOT_H

#include "CProtocolHandler.h"

class PingHandler : public CProtocolHandler {
public:
    PingHandler() {}

    virtual bool Handler(void *Buffer, int len, CLIENT_DATA &clientData);

    virtual ~PingHandler() {}

};

#endif