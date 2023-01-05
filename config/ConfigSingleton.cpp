#include "ConfigSingleton.h"

XMLError ConfigSingleton::LoadProxyConfigurations(std::string filePath)
{
    XMLDocument xmlDoc;
    XMLError eResult = xmlDoc.LoadFile(filePath.c_str());
    XMLCheckResult(eResult);

    XMLNode *pRoot = xmlDoc.FirstChildElement("clickhouse-proxy-v2");
    if (pRoot == nullptr) { return XML_ERROR_FILE_READ_ERROR; }

    PROXY_CONFIG proxyConfig;

    // Get 'workspaces' 
    XMLElement *pWorkSpaces = pRoot->FirstChildElement("WORK_SPACES");
    if (NULL != pWorkSpaces)
    {

        XMLElement *pWorkSpace = pWorkSpaces->FirstChildElement("WORK_SPACE");
        while (pWorkSpace)
        {
            WORK_SPACE workSpace;
            workSpace.name = pWorkSpace->Attribute("name");

            XMLElement *pLocalEndpoints = pWorkSpace->FirstChildElement("LOCAL_ENDPOINTS");
            if (pLocalEndpoints == nullptr) { return XML_ERROR_PARSING_ELEMENT; }

            LOCAL_END_POINTS localEndpoints;

            XMLElement *pCluster = pLocalEndpoints->FirstChildElement("CLUSTER");
            if (pCluster == nullptr) { return XML_ERROR_PARSING_ELEMENT; }
            auto clusterName = pCluster->Attribute("name");
            localEndpoints.clusterName = clusterName;

            XMLElement *pLocalEndPoint = pLocalEndpoints->FirstChildElement("LOCAL_END_POINT");
            if (pLocalEndPoint == nullptr) { return XML_ERROR_PARSING_ELEMENT; }


            auto nodeName = pLocalEndPoint->Attribute("name");
            LOCAL_END_POINT localEndPoint;
            localEndPoint.node = nodeName;

            XMLElement *pHost = pLocalEndPoint->FirstChildElement("HOST");
            if (pHost == nullptr) { return XML_ERROR_PARSING_ELEMENT; }
            localEndPoint.host = pHost->GetText();

            XMLElement *pServices = pLocalEndPoint->FirstChildElement("SERVICES");
            if (pServices == nullptr) { return XML_ERROR_PARSING_ELEMENT; }

            if (NULL != pServices)
            {
                XMLElement *pService = pServices->FirstChildElement("SERVICE");
                if (pService == nullptr) { return XML_ERROR_PARSING_ELEMENT; }

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
                    localEndPoint.services.emplace_back(service);
                    // read next sibling element
                    pService = pService->NextSiblingElement("SERVICE");
                }
                localEndpoints.endpoints.emplace_back(localEndPoint);
            }
            workSpace.localEndpoints = localEndpoints;
            proxyConfig.workspaces.emplace_back(workSpace);

            pWorkSpace = pWorkSpace->NextSiblingElement("WORK_SPACE");
        }

    }

    // Get 'clusters'
    XMLElement *pClusters = pRoot->FirstChildElement("CLUSTERS");
    if (NULL != pClusters)
    {
        XMLElement *pCluster = pClusters->FirstChildElement("CLUSTER");
        if (pCluster == nullptr) { return XML_ERROR_PARSING_ELEMENT; }
        while (pCluster)
        {
            CLUSTER cluster;
            auto clusterName = pCluster->Attribute("name");
            cluster.clusterName = clusterName;

            XMLElement *pEndPoints = pCluster->FirstChildElement("END_POINTS");
            if (NULL != pEndPoints)
            {
                XMLElement *pEndPoint = pEndPoints->FirstChildElement("END_POINT");
                REMOTE_END_POINT endPoint;
                auto endPointName = pEndPoint->Attribute("name");
                endPoint.endPointName = endPointName;

                while (pEndPoint)
                {
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
                    if (pServices == nullptr) { return XML_ERROR_PARSING_ELEMENT; }

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
                            endPoint.services.emplace_back(service);
                            // read next sibling element
                            pService = pService->NextSiblingElement("SERVICE");
                        }
                        cluster.endPoints.emplace_back(endPoint);
                        pEndPoint = pEndPoint->NextSiblingElement("END_POINT");
                    }
                }
                proxyConfig.clusters.emplace_back(cluster);
                pCluster = pCluster->NextSiblingElement("CLUSTER");
            }
        }
    }
    m_ProxyConfig = proxyConfig;
    return XML_SUCCESS;
}


SERVICE ConfigSingleton::Resolve(RESOLVE_CONFIG config)
{
    //PROXY_CONFIG tempProxyConfig;
    CLUSTER cluster;
    auto size = m_ProxyConfig.clusters.size();
    //if (size == 0) { cout << "Cluster Name not found" << endl; return ; }

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
    //if (size == 0) { cout << "Remote End Point Name not found" << endl; return; }
    //Get the end point corresponding to the cluster

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
    //if (size == 0) { cout << "Service Name not found" << endl; return; }

    for (int i = 0; i < size; i++)
    {
        if (endpoint.services[i].name == config.serviceName)
        {
            service = endpoint.services[i];
            break;
        }
    }
    return service;
}