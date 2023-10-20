#include "PipelineFactory.h"

PipelineFunction<CProtocolSocket> PipelineFactory::GetProtocolPipeline(const std::string &pipelineName) {
    return protocolPipelineFunctionMap[pipelineName];
};

PipelineFunction<CProxySocket> PipelineFactory::GetProxyPipeline(const std::string &pipelineName) {
    return proxyPipelineFunctionMap[pipelineName];

};

PipelineFunction<LibuvProxySocket> PipelineFactory::GetLibuvProxyPipeline(const std::string &pipelineName) {
    return libuvProxyPipelineFunctionMap[pipelineName];
}