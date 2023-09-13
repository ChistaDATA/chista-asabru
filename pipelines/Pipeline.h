
#ifndef PIPELINE_DOT_H
#define PIPELINE_DOT_H
#include "../handlers/CProtocolSocket.h"
#include "../handlers/CProxySocket.h"
#include "../test/ProxyInfo.h"
#include "ClientSocket.h"
#include "../config/ConfigSingleton.h"
#include "ProtocolHelper.h"
#include "Socket.h"

typedef struct
{
    ClientTargetPair *pair;
    CProxyHandler *proxy_handler;
} ConnectionData;

#endif