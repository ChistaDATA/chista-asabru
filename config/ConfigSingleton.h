#ifndef CONFIG_SINGLETON_DOT_H
#define CONFIG_SINGLETON_DOT_H

#include <cstdlib>
#include <iostream>
#include "TypeFactory.h"
#include "tinyxml2.h"
#include "config_types.h"
#include "IConfigEngine.h"
#include "CProxyHandler.h"

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
        LoadProxyConfigurations(std::getenv("CONFIG_FILE_PATH"));
    };
    ~ConfigSingleton() = default;
    ConfigSingleton(const ConfigSingleton &) = delete;
    ConfigSingleton &operator=(const ConfigSingleton &) = delete;
    PROXY_CONFIG m_ProxyConfig;
    XMLError LoadProxyConfigurations(std::string filePath);
    TypeFactory *typeFactory = new TypeFactory();

public:
    static bool initialized;
    static ConfigSingleton &getInstance()
    {
        static ConfigSingleton instance;
        return instance;
    }

    RESOLVE_ENDPOINT_RESULT Resolve(RESOLVE_CONFIG config);
    std::vector<RESOLVE_ENDPOINT_RESULT> LoadProxyConfigurations();
    void DownloadConfigFile(std::string url, std::string outputFilePath);
};

#endif