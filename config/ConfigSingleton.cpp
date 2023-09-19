#include "ConfigSingleton.h"
#include "TypeFactory.h"
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