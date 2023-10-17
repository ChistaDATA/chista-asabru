#ifndef CONFIG_SINGLETON_DOT_H
#define CONFIG_SINGLETON_DOT_H

#include <cstdlib>
#include <iostream>
#include "TypeFactory.h"
#include "tinyxml2.h"
#include "CommonTypes.h"
#include "ConfigTypes.h"
#include "CProxyHandler.h"
#include "PipelineFactory.h"

using namespace tinyxml2;

#ifndef XMLCheckResult
#define XMLCheckResult(a_eResult)         \
    if (a_eResult != XML_SUCCESS)         \
    {                                     \
        printf("Error: %i\n", a_eResult); \
        return a_eResult;                 \
    }
#endif

class ConfigSingleton
{
private:
    ConfigSingleton() {
        DownloadConfigFile(std::getenv("CONFIG_FILE_URL"), std::getenv("CONFIG_FILE_PATH"));
        LoadConfigurationsFromFile(std::getenv("CONFIG_FILE_PATH"));
    };
    ~ConfigSingleton() = default;
    ConfigSingleton(const ConfigSingleton &) = delete;
    ConfigSingleton &operator=(const ConfigSingleton &) = delete;
    PROXY_CONFIG m_ProxyConfig;
    std::vector<PROTOCOL_SERVER_CONFIG> m_ProtocolServerConfig;
    XMLError LoadConfigurationsFromFile(std::string filePath);
    XMLError ParseConfiguration(XMLDocument *xmlDoc);
    XMLError LoadProtocolServerConfigurations(XMLNode *root);
    XMLError LoadProxyServerConfigurations(XMLNode *pRoot);
public:
    static bool initialized;
    static ConfigSingleton &getInstance()
    {
        static ConfigSingleton instance;
        return instance;
    }

    XMLError LoadConfigurationsFromString(std::string xml_string);
    std::vector<RESOLVED_PROXY_CONFIG> ResolveProxyServerConfigurations();
    std::vector<RESOLVED_PROTOCOL_CONFIG> ResolveProtocolServerConfigurations();

    void DownloadConfigFile(std::string url, std::string outputFilePath);
    TypeFactory *typeFactory = new TypeFactory();
    PipelineFactory * pipelineFactory = new PipelineFactory();
};

#endif