#pragma once
#include "tinyxml2.h"
#include "logger/Logger.h"
#include "CommonTypes.h"
#include "ConfigTypes.h"
#include "ApiGatewayConfig.h"

#ifndef XMLCheckResult
#define XMLCheckResult(a_eResult)         \
    if (a_eResult != XML_SUCCESS)         \
    {                                     \
        LOG_ERROR("Error parsing XML");   \
        printf("Error: %i\n", a_eResult); \
        return a_eResult;                 \
    }
#endif

using namespace tinyxml2;

class ConfigParser {
public:
    static XMLError ParseConfiguration(
	  XMLDocument *xmlDoc,
	  PROXY_CONFIG &m_ProxyConfig,
	  std::vector<PROTOCOL_SERVER_CONFIG> &m_ProtocolServerConfig,
	  API_GATEWAY_SERVER_CONFIG &m_ApiGatewayConfig
	  );
    static XMLError LoadProtocolServerConfigurations(XMLNode *root, std::vector<PROTOCOL_SERVER_CONFIG> &m_ProtocolServerConfig);
    static XMLError LoadProxyServerConfigurations(XMLNode *pRoot, PROXY_CONFIG &m_ProxyConfig);
    static ENDPOINT_SERVICE_CONFIG ParseEndPointServiceConfiguration(XMLDocument * xmlDoc);
};

