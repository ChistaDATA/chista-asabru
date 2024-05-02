#include "ConfigParser.h"

XMLError ConfigParser::ParseConfiguration(XMLDocument * xmlDoc, PROXY_CONFIG &m_ProxyConfig, std::vector<PROTOCOL_SERVER_CONFIG> &m_ProtocolConfig) {
    XMLNode *pRoot = xmlDoc->FirstChildElement("clickhouse-proxy-v2");
    if (pRoot == nullptr)
    {
        return XML_ERROR_FILE_READ_ERROR;
    }

    /**
     * Get Protocol Server configurations
     */
    LoadProtocolServerConfigurations(pRoot, m_ProtocolConfig);

    /**
     * Get proxy configurations
     */
    LoadProxyServerConfigurations(pRoot, m_ProxyConfig);

    LOG_INFO("Configuration parsed successfully!");
    return XML_SUCCESS;
}


XMLError ConfigParser::LoadProxyServerConfigurations(XMLNode *pRoot, PROXY_CONFIG &m_ProxyConfig)
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

                    XMLElement *loadBalancerStrategyElement = pEndPoint->FirstChildElement("loadbalancer-strategy");
                    if (NULL != loadBalancerStrategyElement)
                    {
                        auto loadBalancerStrategy = "";
                        loadBalancerStrategy = loadBalancerStrategyElement->GetText();
                        endPoint.loadBalancerStrategy = loadBalancerStrategy;
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
                            auto weight = pService->Attribute("weight");
                            if (weight) {
                                service.weight = std::stoi(weight);
                            }

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


XMLError ConfigParser::LoadProtocolServerConfigurations(XMLNode *root, std::vector<PROTOCOL_SERVER_CONFIG> &m_ProtocolServerConfig)
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

        XMLElement *authElement = protocol_server->FirstChildElement("auth");
        if (authElement)
        {
            config.auth = new AUTH_CONFIG();
            XMLElement *strategyElement = authElement->FirstChildElement("strategy");
            if (NULL != strategyElement)
            {
                config.auth->strategy = strategyElement->GetText();
            }

            XMLElement *handlerElement = authElement->FirstChildElement("handler");
            if (NULL != handlerElement)
            {
                config.auth->handler = handlerElement->GetText();
            }

            XMLElement *authorizationElement = authElement->FirstChildElement("authorization");
            if (authorizationElement)
            {
                config.auth->authorization = new AUTHORIZATION_CONFIG();
                XMLElement *strategyElement = authorizationElement->FirstChildElement("strategy");
                if (NULL != strategyElement)
                {
                    config.auth->authorization->strategy = strategyElement->GetText();
                }

                XMLElement *handlerElement = authorizationElement->FirstChildElement("handler");
                if (NULL != handlerElement)
                {
                    config.auth->authorization->handler = handlerElement->GetText();
                }

                XMLElement *dataElement = authorizationElement->FirstChildElement("data");
                if (dataElement)
                {
                        XMLPrinter printer;
                        dataElement->Accept(&printer);
                        config.auth->authorization->data = printer.CStr();
                }
            }
        } else {
            config.auth = nullptr;
        }

        XMLElement *routesElement = protocol_server->FirstChildElement("routes");
        if (routesElement) {
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

                XMLElement *authConfigElement = routeElement->FirstChildElement("auth");
                r.auth.required = (config.auth != nullptr);
                if (authConfigElement)
                {
                    XMLElement *requiredElement = authConfigElement->FirstChildElement("required");
                    if (requiredElement)
                    {
                        auto requiredStrC = requiredElement->GetText();
                        std::string requiredStr(requiredStrC ? requiredStrC : ""); // Ensure non-null string
                        std::transform(requiredStr.begin(), requiredStr.end(), requiredStr.begin(),
                                       [](unsigned char c)
                                       { return std::tolower(c); });
                        if (requiredStr == "false" || requiredStr == "0" || requiredStr == "no")
                        {
                            r.auth.required = false;
                        }
                        else if (requiredStr == "true" || requiredStr == "1" || requiredStr == "yes")
                        {
                            r.auth.required = true;
                        }
                        else
                        {
                            throw std::runtime_error("Invalid or undefined auth configuration ('" + requiredStr + "') for route: " + r.path);
                        }
                    }

                        XMLElement *authorizationElement = authConfigElement->FirstChildElement("authorization");
                        if (authorizationElement)
                        {
                            XMLPrinter printer;
                            authorizationElement->Accept(&printer);
                            r.auth.authorization = printer.CStr();
                        }
                }

                routes.push_back(r);
                routeElement = routeElement->NextSiblingElement("route");
            }
            config.routes = routes;
        }

        result.push_back(config);
        protocol_server = protocol_server->NextSiblingElement("protocol-server");
    }
    m_ProtocolServerConfig = result;
    return XML_SUCCESS;
}

ENDPOINT_SERVICE_CONFIG ConfigParser::ParseEndPointServiceConfiguration(XMLDocument *xmlDoc) {
    ENDPOINT_SERVICE_CONFIG proxyEndpointServiceConfig;
    XMLElement *pEndPoint = xmlDoc->FirstChildElement("end_point");
    if (pEndPoint == nullptr) {
        throw std::runtime_error("Error Parsing XML Element.");
    }
    auto endPointName = pEndPoint->Attribute("name");
    proxyEndpointServiceConfig.name = endPointName;

    XMLElement *pOperation = pEndPoint->FirstChildElement("operation");
    if (nullptr != pOperation) {
        proxyEndpointServiceConfig.operation = pOperation->GetText();
    }

    XMLElement *pService = pEndPoint->FirstChildElement("service");
    if (nullptr != pService) {
        auto serviceName = pService->Attribute("name");
        proxyEndpointServiceConfig.service.name = serviceName;
    }

    XMLElement *pHost = pService->FirstChildElement("host");
    if (nullptr != pHost) {
        auto hostName = "";
        hostName = pHost->GetText();
        proxyEndpointServiceConfig.service.host = hostName;
    }

    XMLElement *pPort = pService->FirstChildElement("port");
    if (nullptr != pPort) {
        auto port = 0;
        XMLError eResult = pPort->QueryIntText(&port);
        if (eResult != tinyxml2::XML_SUCCESS) {
            throw std::runtime_error("Port Must be an Integer value.");
        }
        proxyEndpointServiceConfig.service.port = port;
    }
    LOG_INFO("Service Configuration parsed successfully!");
    return proxyEndpointServiceConfig;
}