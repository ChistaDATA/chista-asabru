#pragma once
#include <string>
#include <unordered_map>

#include <set>
#include <dlfcn.h>
#include "socket/Socket.h"
#include "interface/CProxySocket.h"
#include "interface/CProtocolSocket.h"
#include "interface/LibuvProxySocket.h"
#include "../pipelines/Pipeline.h"

typedef std::map<std::string, PipelineFunction<CProtocolSocket>> ProtocolPipelineFunctionMap;
typedef std::map<std::string, PipelineFunction<CProxySocket>> ProxyPipelineFunctionMap;
typedef std::map<std::string, PipelineFunction<LibuvProxySocket>> LibuvProxyPipelineFunctionMap;

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
        proxyPipelineFunctionMap["ClickHouseTLSStartPipeline"] = ClickHouseTLSStartPipeline;
        proxyPipelineFunctionMap["ClickHouseTLSTerminatePipeline"] = ClickHouseTLSTerminatePipeline;
        proxyPipelineFunctionMap["ClickHouseTLSExchangePipeline"] = ClickHouseTLSExchangePipeline;
        proxyPipelineFunctionMap["PostgreSQLPipeline"] = PostgreSQLPipeline;
        proxyPipelineFunctionMap["MySQLPipeline"] = MySQLPipeline;
        proxyPipelineFunctionMap["PassthroughPipeline"] = PassthroughPipeline;

        protocolPipelineFunctionMap["ProtocolPipeline"] = ProtocolPipeline;
        protocolPipelineFunctionMap["CStreamPipeline"] = CStreamPipeline;

        libuvProxyPipelineFunctionMap["ClickHouseLibuvPipeline"] = ClickHouseLibuvPipeline;
    };
    PipelineFunction<CProtocolSocket> GetProtocolPipeline(const std::string& pipelineName);
    PipelineFunction<CProxySocket> GetProxyPipeline(const std::string& pipelineName);
    PipelineFunction<LibuvProxySocket> GetLibuvProxyPipeline(const std::string& pipelineName);
};
