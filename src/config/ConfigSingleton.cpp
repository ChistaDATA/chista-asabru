#include "ConfigSingleton.h"
#include "Utils.h"
#include "CommandDispatcher.h"
#include "Logger.h"

/**
 * Function to load the config.xml file from a URL
 */
void ConfigSingleton::DownloadConfigFile(const std::string &url, const std::string &outputFilePath) {
    // giving system command and storing return value
    std::string command = "/opt/bin/curl " + url + " --output " + outputFilePath;
    int returnCode = system(command.c_str());

    // checking if the command was executed successfully
    if (returnCode == 0) {
        LOG_INFO("File downloaded successfully!");
    } else {
        LOG_ERROR("Failed to download file :" + std::to_string(returnCode));
    }
}

/**
 * Function to load the proxy configuration from a file
 * @param filePath the file path to the config.xml file
 */
XMLError ConfigSingleton::LoadConfigurationsFromFile(std::string filePath) {
    XMLDocument xmlDoc;
    XMLError eResult = xmlDoc.LoadFile(filePath.c_str());
    XMLCheckResult(eResult);

    return ConfigParser::ParseConfiguration(&xmlDoc, m_ProxyConfig, m_ProtocolServerConfig);
}

/**
 * Function to load the proxy configuration from a string
 * @param xml_string the xml string that contains the configuration 
 */
XMLError ConfigSingleton::LoadConfigurationsFromString(std::string xml_string) {
    XMLDocument xmlDoc;
    XMLError eResult = xmlDoc.Parse(xml_string.c_str());
    XMLCheckResult(eResult);

    return ConfigParser::ParseConfiguration(&xmlDoc, m_ProxyConfig, m_ProtocolServerConfig);
}

/**
 * Function to load the proxy endpoint services configuration from a string
 * @param xml_string the xml string that contains the endpoint services configuration
 */
ENDPOINT_SERVICE_CONFIG ConfigSingleton::LoadEndpointServiceFromString(const std::string& xml_string) {
    XMLDocument xmlDoc;
    XMLError eResult = xmlDoc.Parse(xml_string.c_str());
    if (eResult != tinyxml2::XML_SUCCESS) {
        throw std::runtime_error("Error parsing xml document.");
    }

    return ConfigParser::ParseEndPointServiceConfiguration(&xmlDoc);
}

std::vector<RESOLVED_PROXY_CONFIG> ConfigSingleton::ResolveProxyServerConfigurations() const {
    std::vector<RESOLVED_PROXY_CONFIG> results;
    std::vector<CLUSTER> clusters = m_ProxyConfig.clusters;
    for (const auto &cluster: clusters) {
        for (const auto &endpoint: cluster.endPoints) {
            RESOLVED_PROXY_CONFIG result;
            result.name = endpoint.endPointName;
            result.proxyPort = endpoint.proxyPort;

            for (const auto &service: endpoint.services) {
                RESOLVED_SERVICE resolvedService;
                resolvedService.ipaddress = service.host;
                resolvedService.port = service.port;
                resolvedService.r_w = endpoint.readWrite;
                resolvedService.alias = "";
                resolvedService.reserved = 0;
                resolvedService.weight = service.weight;
                memset(resolvedService.Buffer, 0, sizeof resolvedService.Buffer);
                result.services.push_back(resolvedService);
            }
            // Resolve load balancer stragegy
            if (!endpoint.loadBalancerStrategy.empty())
                result.loadBalancerStrategy = loadBalancerFactory->GetLoadBalancerStrategy(endpoint.loadBalancerStrategy);

            // Resolve the Pipeline
            result.pipelineName = endpoint.pipeline;
            result.pipeline = pipelineFactory->GetProxyPipeline(endpoint.pipeline);

            // Resolve the Handler class
            CommandDispatcher::RegisterCommand<BaseHandler>(endpoint.handler);
            result.handler = CommandDispatcher::GetCommand<BaseHandler>(endpoint.handler);

            results.push_back(result);
        }
    }

    return results;
}

std::vector<RESOLVED_PROTOCOL_CONFIG> ConfigSingleton::ResolveProtocolServerConfigurations() {
    std::vector<RESOLVED_PROTOCOL_CONFIG> results;
    for (const PROTOCOL_SERVER_CONFIG &protocol_server: m_ProtocolServerConfig) {
        RESOLVED_PROTOCOL_CONFIG result;
        result.protocol_name = protocol_server.protocol_name;
        result.protocol_port = protocol_server.protocol_port;
        // Resolve the Pipeline
        result.pipeline = pipelineFactory->GetProtocolPipeline(protocol_server.pipeline);
        // Resolve the Handler class
        CommandDispatcher::RegisterCommand<BaseHandler>(protocol_server.handler);
        result.handler = CommandDispatcher::GetCommand<BaseHandler>(protocol_server.handler);

        result.routes = protocol_server.routes;
        results.push_back(result);
    }

    return results;
}

