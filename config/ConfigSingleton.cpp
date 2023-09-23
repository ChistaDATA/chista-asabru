#include "ConfigSingleton.h"
#include "TypeFactory.h"
#include "Utils.h"

/**
 * Function to load the config.xml file from a URL
 */
void ConfigSingleton::DownloadConfigFile(std::string url, std::string outputFilePath) {
    // giving system command and storing return value
    std::string command = "/opt/bin/curl " + url + " --output " + outputFilePath;
    int returnCode = system(command.c_str());
 
    // checking if the command was executed successfully
    if (returnCode == 0) {
        std::cout << "File downloaded successfully!" << std::endl;
    }
    else {
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
XMLError ConfigSingleton::LoadProxyConfigurations(std::string filePath)
{
    XMLDocument xmlDoc;
    XMLError eResult = xmlDoc.LoadFile(filePath.c_str());
    XMLCheckResult(eResult);

    XMLNode *pRoot = xmlDoc.FirstChildElement("clickhouse-proxy-v2");
    if (pRoot == nullptr)
    {
        return XML_ERROR_FILE_READ_ERROR;
    }

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

                    XMLElement *pHost = pEndPoint->FirstChildElement("HOST");
                    if (NULL != pHost)
                    {
                        auto hostName = "";
                        hostName = pHost->GetText();
                        endPoint.host = hostName;
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

                            XMLElement *pProxyPort = pService->FirstChildElement("PROXY_PORT");
                            if (NULL != pProxyPort)
                            {
                                auto proxyPort = 0;
                                eResult = pProxyPort->QueryIntText(&proxyPort);
                                service.proxyPort = proxyPort;
                                XMLCheckResult(eResult);
                            }

                            XMLElement *pPort = pService->FirstChildElement("PORT");
                            if (NULL != pPort)
                            {
                                auto port = 0;
                                eResult = pPort->QueryIntText(&port);
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

                            XMLElement *handlerElement = pService->FirstChildElement("HANDLER");
                            if (NULL != handlerElement)
                            {
                                auto handler = "";
                                handler = handlerElement->GetText();
                                service.handler = handler;
                            }

                            XMLElement *pipelineElement = pService->FirstChildElement("PIPELINE");
                            if (NULL != pipelineElement)
                            {
                                auto pipeline = "";
                                pipeline = pipelineElement->GetText();
                                service.pipeline = pipeline;
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
    return XML_SUCCESS;
}

std::vector<RESOLVE_ENDPOINT_RESULT> ConfigSingleton::LoadProxyConfigurations()
{
    std::vector<RESOLVE_ENDPOINT_RESULT> results;
    std::vector<CLUSTER> clusters = m_ProxyConfig.clusters;
    for (auto cluster : clusters)
    {
        for (auto endpoint : cluster.endPoints)
        {
            for (auto service : endpoint.services)
            {
                RESOLVE_ENDPOINT_RESULT result;
                result.name = service.name;
                result.ipaddress = endpoint.host;
                result.proxyPort = service.proxyPort;
                result.port = service.port;
                result.r_w = endpoint.readWrite;
                result.alias = "";
                result.reserved = 0;
                memset(result.Buffer, 0, sizeof result.Buffer);

                // Resolve the Pipeline
                result.pipeline = service.pipeline;
                // Resolve the Handler class
                result.handler = typeFactory->GetType(service.handler);

                results.push_back(result);
            }
        }
    }

    return results;
}

RESOLVE_ENDPOINT_RESULT ConfigSingleton::Resolve(RESOLVE_CONFIG config)
{
    RESOLVE_ENDPOINT_RESULT result;
    CLUSTER cluster;
    auto size = m_ProxyConfig.clusters.size();
    // if (size == 0) { cout << "Cluster Name not found" << endl; return ; }

    // Get the cluster
    for (int i = 0; i < size; i++)
    {
        if (m_ProxyConfig.clusters[i].clusterName == config.clusterName)
        {
            cluster = m_ProxyConfig.clusters[i];
            break;
        }
    }

    REMOTE_END_POINT endpoint;
    size = cluster.endPoints.size();
    // if (size == 0) { cout << "Remote End Point Name not found" << endl; return; }
    // Get the end point corresponding to the cluster

    for (int i = 0; i < size; i++)
    {
        if (cluster.endPoints[i].endPointName == config.endPointName)
        {
            endpoint = cluster.endPoints[i];
            break;
        }
    }

    SERVICE service;
    size = endpoint.services.size();
    // if (size == 0) { cout << "Service Name not found" << endl; return; }

    for (int i = 0; i < size; i++)
    {
        if (endpoint.services[i].name == config.serviceName)
        {
            service = endpoint.services[i];
            break;
        }
    }

    result.port = service.port;
    result.ipaddress = endpoint.host;
    result.r_w = endpoint.readWrite;
    result.alias = "";
    result.reserved = 0;
    memset(result.Buffer, 0, sizeof result.Buffer);

    /**
     * Resolve the Handler class
     */
    result.handler = typeFactory->GetType(service.handler);

    return result;
}