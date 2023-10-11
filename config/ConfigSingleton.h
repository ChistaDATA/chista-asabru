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
        LoadConfigurations(std::getenv("CONFIG_FILE_PATH"));
    };
    ~ConfigSingleton() = default;
    ConfigSingleton(const ConfigSingleton &) = delete;
    ConfigSingleton &operator=(const ConfigSingleton &) = delete;
    PROXY_CONFIG m_ProxyConfig;
    std::vector<PROTOCOL_SERVER_CONFIG> m_ProtocolServerConfig;
    XMLError LoadConfigurations(std::string filePath);
    XMLError LoadProtocolServerConfigurations(XMLNode *root);
    XMLError LoadProxyServerConfigurations(XMLNode *pRoot);
public:
    static bool initialized;
    static ConfigSingleton &getInstance()
    {
        static ConfigSingleton instance;
        return instance;
    }

    RESOLVE_ENDPOINT_RESULT Resolve(RESOLVE_CONFIG config);

    std::vector<RESOLVE_ENDPOINT_RESULT> ResolveProxyServerConfigurations();
    std::vector<RESOLVED_PROTOCOL_CONFIG> ResolveProtocolServerConfigurations();

    void DownloadConfigFile(std::string url, std::string outputFilePath);
    TypeFactory *typeFactory = new TypeFactory();
    PipelineFactory * pipelineFactory = new PipelineFactory();
};

#endif