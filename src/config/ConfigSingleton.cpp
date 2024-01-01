#include "ConfigSingleton.h"
#include "Utils.h"
#include "CommandDispatcher.h"

/**
 * Function to load the config.xml file from a URL
 */
void ConfigSingleton::DownloadConfigFile(const std::string& url, const std::string& outputFilePath)
{
    // giving system command and storing return value
    std::string command = "/opt/bin/curl " + url + " --output " + outputFilePath;
    int returnCode = system(command.c_str());

    // checking if the command was executed successfully
    if (returnCode == 0)
    {
        std::cout << "File downloaded successfully!" << std::endl;
    }
    else
    {
        std::cerr << "Failed to download file :" << returnCode << std::endl;
    }

    // if (downloadFileWithCurl(url, outputFilePath)) {
    //     std::cout << "File downloaded successfully!" << std::endl;
    // } else {
    //     std::cerr << "Failed to download file" << std::endl;
    // }
}

/**
 * Function to load the proxy configuration from a file
 * @param filePath the file path to the config.xml file
 */
XMLError ConfigSingleton::LoadConfigurationsFromFile(std::string filePath)
{
    XMLDocument xmlDoc;
    XMLError eResult = xmlDoc.LoadFile(filePath.c_str());
    XMLCheckResult(eResult);

    return ParseConfiguration(&xmlDoc);
}

/**
 * Function to load the proxy configuration from a string
 * @param xml_string the xml string that contains the configuration 
 */
XMLError ConfigSingleton::LoadConfigurationsFromString(std::string xml_string)
{
    XMLDocument xmlDoc;
    XMLError eResult = xmlDoc.Parse(xml_string.c_str());
    XMLCheckResult(eResult);

    return ParseConfiguration(&xmlDoc);
}

/**
 * Function to load the proxy endpoint services configuration from a string
 * @param xml_string the xml string that contains the endpoint services configuration
 */
ENDPOINT_SERVICE_CONFIG ConfigSingleton::LoadEndpointServiceFromString(std::string xml_string)
{
    XMLDocument xmlDoc;
    XMLError eResult = xmlDoc.Parse(xml_string.c_str());
    if (eResult != tinyxml2::XML_SUCCESS)
    {
        throw std::runtime_error("Error parsing xml document.");
    }

    return ParseEndPointServiceConfiguration(&xmlDoc);
}

XMLError ConfigSingleton::ParseConfiguration(XMLDocument * xmlDoc) {
    XMLNode *pRoot = xmlDoc->FirstChildElement("clickhouse-proxy-v2");
    if (pRoot == nullptr)
    {
        return XML_ERROR_FILE_READ_ERROR;
    }

    /**
     * Get Protocol Server configurations
     */
    LoadProtocolServerConfigurations(pRoot);

    /**
     * Get proxy configurations
     */
    LoadProxyServerConfigurations(pRoot);

    std::cout << "Configuration parsed successfully!" << std::endl;
    return XML_SUCCESS;
}

XMLError ConfigSingleton::LoadProxyServerConfigurations(XMLNode *pRoot)
{
    PROXY_CONFIG proxyConfig;
    // Get 'clusters'
    XMLElement *pClusters = pRoot->FirstChildElement("CLUSTERS");
    if (NULL != pClusters)
    {
        XMLElement *pCluster = pClusters->FirstChildElement("CLUSTER");
        if (pCluster == nullptr)
        {
            return XML_ERROR_PARSING_ELEMENT;
        }
        while (pCluster)
        {
            CLUSTER cluster;
            auto clusterName = pCluster->Attribute("name");
            cluster.clusterName = clusterName;

            XMLElement *pEndPoints = pCluster->FirstChildElement("END_POINTS");
            if (NULL != pEndPoints)
            {
                XMLElement *pEndPoint = pEndPoints->FirstChildElement("END_POINT");

                while (pEndPoint)
                {
                    REMOTE_END_POINT endPoint;
                    auto endPointName = pEndPoint->Attribute("name");
                    endPoint.endPointName = endPointName;

                    XMLElement *pReadWrite = pEndPoint->FirstChildElement("READ_WRITE");
                    if (NULL != pReadWrite)
                    {
                        endPoint.readWrite = pEndPoint->GetText();
                    }

                    XMLElement *pProxyPort = pEndPoint->FirstChildElement("PROXY_PORT");
                    if (NULL != pProxyPort)
                    {
                        auto proxyPort = 0;
                        XMLError eResult = pProxyPort->QueryIntText(&proxyPort);
                        endPoint.proxyPort = proxyPort;
                        XMLCheckResult(eResult);
                    }

                    XMLElement *handlerElement = pEndPoint->FirstChildElement("HANDLER");
                    if (NULL != handlerElement)
                    {
                        auto handler = "";
                        handler = handlerElement->GetText();
                        endPoint.handler = handler;
                    }

                    XMLElement *pipelineElement = pEndPoint->FirstChildElement("PIPELINE");
                    if (NULL != pipelineElement)
                    {
                        auto pipeline = "";
                        pipeline = pipelineElement->GetText();
                        endPoint.pipeline = pipeline;
                    }

                    XMLElement *pServices = pEndPoint->FirstChildElement("SERVICES");
                    if (pServices == nullptr)
                    {
                        return XML_ERROR_PARSING_ELEMENT;
                    }

                    if (NULL != pServices)
                    {
                        XMLElement *pService = pServices->FirstChildElement("SERVICE");
                        while (pService)
                        {
                            SERVICE service;
                            auto serviceName = pService->Attribute("name");
                            service.name = serviceName;

                            XMLElement *pHost = pService->FirstChildElement("HOST");
                            if (NULL != pHost)
                            {
                                auto hostName = "";
                                hostName = pHost->GetText();
                                service.host = hostName;
                            }

                            XMLElement *pPort = pService->FirstChildElement("PORT");
                            if (NULL != pPort)
                            {
                                auto port = 0;
                                XMLError eResult = pPort->QueryIntText(&port);
                                service.port = port;
                                XMLCheckResult(eResult);
                            }

                            XMLElement *pHostName = pService->FirstChildElement("PROTOCOL");
                            if (NULL != pHostName)
                            {
                                auto protocol = "";
                                protocol = pHostName->GetText();
                                service.protocol = protocol;
                            }

                            endPoint.services.emplace_back(service);
                            // read next sibling element
                            pService = pService->NextSiblingElement("SERVICE");
                        }
                    }
                    cluster.endPoints.emplace_back(endPoint);
                    pEndPoint = pEndPoint->NextSiblingElement("END_POINT");
                }
                proxyConfig.clusters.emplace_back(cluster);
                pCluster = pCluster->NextSiblingElement("CLUSTER");
            }
        }
    }

    m_ProxyConfig = proxyConfig;
}

std::vector<RESOLVED_PROXY_CONFIG> ConfigSingleton::ResolveProxyServerConfigurations()
{
    std::vector<RESOLVED_PROXY_CONFIG> results;
    std::vector<CLUSTER> clusters = m_ProxyConfig.clusters;
    for (auto cluster : clusters)
    {
        for (auto endpoint : cluster.endPoints)
        {
            RESOLVED_PROXY_CONFIG result;
            result.name = endpoint.endPointName;
            result.proxyPort = endpoint.proxyPort;

            for (auto service : endpoint.services)
            {
                RESOLVED_SERVICE resolvedService;
                resolvedService.ipaddress = service.host;
                resolvedService.port = service.port;
                resolvedService.r_w = endpoint.readWrite;
                resolvedService.alias = "";
                resolvedService.reserved = 0;
                memset(resolvedService.Buffer, 0, sizeof resolvedService.Buffer);
                result.services.push_back(resolvedService);
            }

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

XMLError ConfigSingleton::LoadProtocolServerConfigurations(XMLNode *root)
{
    std::vector<PROTOCOL_SERVER_CONFIG> result;
    XMLElement *protocol_server_config = root->FirstChildElement("protocol-server-config");

    XMLElement *protocol_server = protocol_server_config->FirstChildElement("protocol-server");
    if (protocol_server == nullptr)
    {
        return XML_ERROR_PARSING_ELEMENT;
    }
    while (protocol_server)
    {
        PROTOCOL_SERVER_CONFIG config;
        config.protocol_name = protocol_server->Attribute("protocol");

        XMLElement *protocolPortElement = protocol_server->FirstChildElement("protocol-port");
        if (NULL != protocolPortElement)
        {
            config.protocol_port = std::stoi(protocolPortElement->GetText());
        }

        XMLElement *pipelineElement = protocol_server->FirstChildElement("pipeline");
        if (NULL != pipelineElement)
        {
            config.pipeline = pipelineElement->GetText();
        }

        XMLElement *handlerElement = protocol_server->FirstChildElement("handler");
        if (NULL != handlerElement)
        {
            config.handler = handlerElement->GetText();
        }

        XMLElement *routesElement = protocol_server->FirstChildElement("routes");
        XMLElement *routeElement = routesElement->FirstChildElement("route");
        std::vector<Route> routes;
        while (routeElement) {
            Route r;
            XMLElement *pathElement = routeElement->FirstChildElement("path");
            if (NULL != pathElement)
            {
                r.path = pathElement->GetText();
            }

            XMLElement *methodElement = routeElement->FirstChildElement("method");
            if (NULL != methodElement)
            {
                r.method = methodElement->GetText();
            }

            XMLElement *requestHandlerElement = routeElement->FirstChildElement("request_handler");
            if (NULL != requestHandlerElement)
            {
                r.request_handler = requestHandlerElement->GetText();
            }
            routes.push_back(r);
            routeElement = routeElement->NextSiblingElement("route");
        }
        config.routes = routes;
        result.push_back(config);
        protocol_server = protocol_server->NextSiblingElement("protocol-server");
    }
    m_ProtocolServerConfig = result;
}

std::vector<RESOLVED_PROTOCOL_CONFIG> ConfigSingleton::ResolveProtocolServerConfigurations()
{
    std::vector<RESOLVED_PROTOCOL_CONFIG> results;
    for (PROTOCOL_SERVER_CONFIG protocol_server : m_ProtocolServerConfig)
    {
        std::cout << protocol_server.protocol_name << " " << protocol_server.protocol_port << std::endl;
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

ENDPOINT_SERVICE_CONFIG ConfigSingleton::ParseEndPointServiceConfiguration(XMLDocument *xmlDoc)
{
    ENDPOINT_SERVICE_CONFIG proxyEndpointServiceConfig;
    XMLElement *pEndPoint = xmlDoc->FirstChildElement("end_point");
    if (pEndPoint == nullptr)
    {
        throw std::runtime_error("Error Parsing XML Element.");
    }
    auto endPointName = pEndPoint->Attribute("name");
    proxyEndpointServiceConfig.name = endPointName;

    XMLElement *pOperation = pEndPoint->FirstChildElement("operation");
    if (nullptr != pOperation)
    {
        proxyEndpointServiceConfig.operation = pOperation->GetText();
    }

    XMLElement *pService = pEndPoint->FirstChildElement("service");
    if (nullptr != pService)
    {
        auto serviceName = pService->Attribute("name");
        proxyEndpointServiceConfig.service.name = serviceName;
    }

  XMLElement *pHost = pService->FirstChildElement("host");
  if (nullptr != pHost)
  {
      auto hostName = "";
      hostName = pHost->GetText();
      proxyEndpointServiceConfig.service.host = hostName;
  }

  XMLElement *pPort = pService->FirstChildElement("port");
  if (nullptr != pPort)
  {
      auto port = 0;
      XMLError eResult = pPort->QueryIntText(&port);
      if (eResult != tinyxml2::XML_SUCCESS)
      {
          throw std::runtime_error("Port Must be an Integer value.");
      }
      proxyEndpointServiceConfig.service.port = port;
  }
  std::cout << "Service Configuration parsed successfully!" << std::endl;
  return proxyEndpointServiceConfig;
}