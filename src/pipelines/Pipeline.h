
#ifndef PIPELINE_DOT_H
#define PIPELINE_DOT_H

#include "CProtocolSocket.h"
#include "CProxySocket.h"
#include "CClientSocket.h"
#include "../config/ConfigSingleton.h"
#include "ProtocolHelper.h"
#include "Socket.h"
#include "CServerSocket.h"

typedef struct {
    ClientTargetPair *pair;
    CProxyHandler *proxy_handler;
} ConnectionData;

#endif