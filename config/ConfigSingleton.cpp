#include "ConfigSingleton.h"
#include "TypeFactory.h"
#include "Utils.h"

/**
 * Function to load the config.xml file from a URL
 */
void ConfigSingleton::DownloadConfigFile(std::string url, std::string outputFilePath)
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
        std::cerr << "Failed to download file :" << returnCode << endl;
    }

    // if (downloadFileWithCurl(url, outputFilePath)) {
    //     std::cout << "File downloaded successfully!" << std::endl;
    // } else {
    //     std::cerr << "Failed to download file" << std::endl;
    // }
}

/**
 * Function to load the proxy configuration
 * @param filePath the file path to the config.xml file
 */
XMLError ConfigSingleton::LoadConfigurations(std::string filePath)
{
    XMLDocument xmlDoc;
    XMLError eResult = xmlDoc.LoadFile(filePath.c_str());
    XMLCheckResult(eResult);

    XMLNode *pRoot = xmlDoc.FirstChildElement("clickhouse-proxy-v2");
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
            result.pipeline = pipelineFactory->GetProxyPipeline(endpoint.pipeline);
            // Resolve the Handler class
            result.handler = typeFactory->GetType(endpoint.handler);

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
            config.protocol_port = stoi(protocolPortElement->GetText());
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
        cout << protocol_server.protocol_name << " " << protocol_server.protocol_port << endl;
        RESOLVED_PROTOCOL_CONFIG result;
        result.protocol_name = protocol_server.protocol_name;
        result.protocol_port = protocol_server.protocol_port;
        // Resolve the Pipeline
        result.pipeline = pipelineFactory->GetProtocolPipeline(protocol_server.pipeline);
        // Resolve the Handler class
        result.handler = typeFactory->GetType(protocol_server.handler);

        results.push_back(result);
    }

    return results;
}