#ifndef PIPELINE_DOT_H
#define PIPELINE_DOT_H

#include "CClientSocket.h"
#include "CProtocolSocket.h"
#include "CProxySocket.h"
#include "CServerSocket.h"
#include "LibuvProxySocket.h"
#include "ProtocolHelper.h"
#include "Socket.h"

void *ClickHousePipeline(CProxySocket *ptr, void *lptr);
void *ClickHouseLibuvPipeline(LibuvProxySocket *ptr, void *lptr);
void *ClickHouseTLSTerminatePipeline(CProxySocket *ptr, void *lptr);
void *ClickHouseTLSStartPipeline(CProxySocket *ptr, void *lptr);
void *ClickHouseTLSExchangePipeline(CProxySocket *ptr, void *lptr);
void *PostgreSQLPipeline(CProxySocket *ptr, void *lptr);
void *MySQLPipeline(CProxySocket *ptr, void *lptr);
void *PassthroughPipeline(CProxySocket *ptr, void *lptr);
void *ProtocolPipeline(CProtocolSocket *ptr, void *lptr);
void *CStreamPipeline(CProtocolSocket *ptr, void *lptr);

typedef struct {
	ClientTargetPair *pair;
	CProxyHandler *proxy_handler;
} ConnectionData;

typedef struct {
	Socket *client_socket;
	Socket *target_socket;
	std::string target_address;
	int target_port;
} Connection;

#endif
