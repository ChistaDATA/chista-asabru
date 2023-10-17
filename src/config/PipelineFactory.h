#pragma once
#include <string>
#include <unordered_map>

#include <set>
#include <dlfcn.h>
#include "Socket.h"
#include "CProxySocket.h"
#include "CProtocolSocket.h"

typedef std::map<string, PipelineFunction<CProtocolSocket>> ProtocolPipelineFunctionMap;
typedef std::map<string, PipelineFunction<CProxySocket>> ProxyPipelineFunctionMap;

void *ClickHousePipeline(CProxySocket *ptr, void *lptr);
void *ClickHouseSSLPipeline(CProxySocket *ptr, void *lptr);
void *PostgreSQLPipeline(CProxySocket *ptr, void *lptr);
void *MySQLPipeline(CProxySocket *ptr, void *lptr);
void *PassthroughPipeLine(CProtocolSocket *ptr, void *lptr);
void *ProtocolPipeline(CProtocolSocket *ptr, void *lptr);

class PipelineFactory
{
private:
    ProtocolPipelineFunctionMap protocolPipelineFunctionMap;
    ProxyPipelineFunctionMap proxyPipelineFunctionMap;
public:
    PipelineFactory()
    {
        // Create PipelineFunction mappings
        proxyPipelineFunctionMap["ClickHousePipeline"] = ClickHousePipeline;
        proxyPipelineFunctionMap["ClickHouseSSLPipeline"] = ClickHouseSSLPipeline;
        proxyPipelineFunctionMap["PostgreSQLPipeline"] = PostgreSQLPipeline;
        proxyPipelineFunctionMap["MySQLPipeline"] = MySQLPipeline;

        protocolPipelineFunctionMap["ProtocolPipeline"] = ProtocolPipeline;
    };
    PipelineFunction<CProtocolSocket> GetProtocolPipeline(std::string pipelineName);
    PipelineFunction<CProxySocket> GetProxyPipeline(std::string pipelineName);
};
