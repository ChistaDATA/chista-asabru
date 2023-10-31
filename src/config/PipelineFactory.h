#pragma once
#include <string>
#include <unordered_map>

#include <set>
#include <dlfcn.h>
#include "Socket.h"
#include "CProxySocket.h"
#include "CProtocolSocket.h"
#include "LibuvProxySocket.h"

typedef std::map<string, PipelineFunction<CProtocolSocket>> ProtocolPipelineFunctionMap;
typedef std::map<string, PipelineFunction<CProxySocket>> ProxyPipelineFunctionMap;
typedef std::map<string, PipelineFunction<LibuvProxySocket>> LibuvProxyPipelineFunctionMap;

void *ClickHousePipeline(CProxySocket *ptr, void *lptr);
void *ClickHouseLibuvPipeline(LibuvProxySocket *ptr, void *lptr);
void *ClickHouseSSLPipeline(CProxySocket *ptr, void *lptr);
void *ClickHouseTLSPipeline(CProxySocket *ptr, void *lptr);
void *PostgreSQLPipeline(CProxySocket *ptr, void *lptr);
void *MySQLPipeline(CProxySocket *ptr, void *lptr);
void *PassthroughPipeLine(CProtocolSocket *ptr, void *lptr);
void *ProtocolPipeline(CProtocolSocket *ptr, void *lptr);

class PipelineFactory
{
private:
    ProtocolPipelineFunctionMap protocolPipelineFunctionMap;
    ProxyPipelineFunctionMap proxyPipelineFunctionMap;
    LibuvProxyPipelineFunctionMap libuvProxyPipelineFunctionMap;
public:
    PipelineFactory()
    {
        // Create PipelineFunction mappings
        proxyPipelineFunctionMap["ClickHousePipeline"] = ClickHousePipeline;
        proxyPipelineFunctionMap["ClickHouseSSLPipeline"] = ClickHouseSSLPipeline;
        proxyPipelineFunctionMap["ClickHouseTLSPipeline"] = ClickHouseTLSPipeline;
        proxyPipelineFunctionMap["PostgreSQLPipeline"] = PostgreSQLPipeline;
        proxyPipelineFunctionMap["MySQLPipeline"] = MySQLPipeline;

        protocolPipelineFunctionMap["ProtocolPipeline"] = ProtocolPipeline;

        libuvProxyPipelineFunctionMap["ClickHouseLibuvPipeline"] = ClickHouseLibuvPipeline;
    };
    PipelineFunction<CProtocolSocket> GetProtocolPipeline(const std::string& pipelineName);
    PipelineFunction<CProxySocket> GetProxyPipeline(const std::string& pipelineName);
    PipelineFunction<LibuvProxySocket> GetLibuvProxyPipeline(const std::string& pipelineName);
};
