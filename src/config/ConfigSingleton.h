#pragma once

#include <cstdlib>
#include <iostream>
#include <utility>
#include "TypeFactory.h"
#include "tinyxml2.h"
#include "CommonTypes.h"
#include "ConfigTypes.h"
#include "interface/CProxyHandler.h"
#include "PipelineFactory.h"
#include "ConfigParser.h"
#include "LoadBalancerFactory.h"
#include "authentication/AuthenticationFactory.h"
#include "authorization/AuthorizationFactory.h"

using namespace tinyxml2;

/* Proxy sockets map */
typedef std::map<std::string, CProxySocket *> ProxySocketsMap;
typedef std::map<std::string, CProtocolSocket *> ProtocolSocketsMap;
typedef std::map<std::string, CApiGatewaySocket *> ApiGatewaySocketsMap;



class ConfigSingleton
{
private:
    ConfigSingleton() {
        if(!std::getenv("CONFIG_FILE_PATH")) {
            throw std::runtime_error("CONFIG_FILE_PATH environment variable is missing!");
        }
        if (std::getenv("CONFIG_FILE_URL")) {
            DownloadConfigFile(std::getenv("CONFIG_FILE_URL"), std::getenv("CONFIG_FILE_PATH"));
        }
        LoadConfigurationsFromFile(std::getenv("CONFIG_FILE_PATH"));
    };
    ~ConfigSingleton() = default;
    ConfigSingleton(const ConfigSingleton &) = delete;
    ConfigSingleton &operator=(const ConfigSingleton &) = delete;
    PROXY_CONFIG m_ProxyConfig;
    std::vector<PROTOCOL_SERVER_CONFIG> m_ProtocolServerConfig;
	API_GATEWAY_SERVER_CONFIG m_ApiGatewayServerConfig;

    XMLError LoadConfigurationsFromFile(std::string filePath);
public:
    static bool initialized;
    static ConfigSingleton &getInstance()
    {
        static ConfigSingleton instance;
        return instance;
    }

    PROXY_CONFIG getProxyConfig() { return m_ProxyConfig; }
    void setProxyConfig(PROXY_CONFIG proxyConfig) {
        m_ProxyConfig = std::move(proxyConfig);
    }

	API_GATEWAY_SERVER_CONFIG getApiGatewayConfig() {
		return m_ApiGatewayServerConfig;
	}

    XMLError LoadConfigurationsFromString(std::string xml_string);
    static ENDPOINT_SERVICE_CONFIG LoadEndpointServiceFromString(const std::string& xml_string);
    std::vector<RESOLVED_PROXY_CONFIG> ResolveProxyServerConfigurations() const;
    std::vector<RESOLVED_PROTOCOL_CONFIG> ResolveProtocolServerConfigurations();

    void DownloadConfigFile(const std::string& url, const std::string& outputFilePath);
    PipelineFactory *pipelineFactory = new PipelineFactory();
    LoadBalancerFactory *loadBalancerFactory = new LoadBalancerFactory();
    // Create Proxy sockets mapping
    ProxySocketsMap proxySocketsMap;
    ProtocolSocketsMap protocolSocketsMap;
	ApiGatewaySocketsMap apiGatewaySocketsMap;

    AuthenticationFactory *authenticationFactory = new AuthenticationFactory();
    AuthorizationFactory *authorizationFactory = new AuthorizationFactory();
};

static ConfigSingleton &configSingleton = ConfigSingleton::getInstance();
