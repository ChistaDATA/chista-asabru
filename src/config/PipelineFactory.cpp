#include "PipelineFactory.h"

PipelineFunction<CProtocolSocket> PipelineFactory::GetProtocolPipeline(std::string pipelineName) {
    return protocolPipelineFunctionMap[pipelineName];
};

PipelineFunction<CProxySocket> PipelineFactory::GetProxyPipeline(std::string pipelineName) {
    return proxyPipelineFunctionMap[pipelineName];
};