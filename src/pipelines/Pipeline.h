
#ifndef PIPELINE_DOT_H
#define PIPELINE_DOT_H

#include "CProtocolSocket.h"
#include "CProxySocket.h"
#include "CClientSocket.h"
#include "ProtocolHelper.h"
#include "Socket.h"
#include "LibuvProxySocket.h"
#include "CServerSocket.h"

void *ClickHousePipeline(CProxySocket *ptr, void *lptr);
void *ClickHouseLibuvPipeline(LibuvProxySocket *ptr, void *lptr);
void *ClickHouseTLSTerminatePipeline(CProxySocket *ptr, void *lptr);
void *ClickHouseTLSStartPipeline(CProxySocket *ptr, void *lptr);
void *ClickHouseTLSExchangePipeline(CProxySocket *ptr, void *lptr);
void *PostgreSQLPipeline(CProxySocket *ptr, void *lptr);
void *MySQLPipeline(CProxySocket *ptr, void *lptr);
void *PassthroughPipeLine(CProtocolSocket *ptr, void *lptr);
void *ProtocolPipeline(CProtocolSocket *ptr, void *lptr);
void *CStreamPipeline(CProtocolSocket *ptr, void *lptr);

typedef struct {
    ClientTargetPair *pair;
    CProxyHandler *proxy_handler;
} ConnectionData;

#endif