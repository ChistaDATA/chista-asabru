#include <iostream>

#include "../lib/tinyxml2/tinyxml2.h"
#include "config_types.h"
#include "IConfigEngine.h"

using namespace tinyxml2;

#ifndef XMLCheckResult
#define XMLCheckResult(a_eResult) if (a_eResult != XML_SUCCESS) { printf("Error: %i\n", a_eResult); return a_eResult; }
#endif



class ConfigSingleton
{
public:
    static ConfigSingleton& getInstance(){
        static ConfigSingleton instance;
        instance.LoadProxyConfigurations("../config/config2.xml");
        // volatile int dummy{};
        return instance;
    }

    SERVICE Resolve(RESOLVE_CONFIG config);
private:
    ConfigSingleton()= default;
    ~ConfigSingleton()= default;
    ConfigSingleton(const ConfigSingleton&)= delete;
    ConfigSingleton& operator=(const ConfigSingleton&)= delete;

    PROXY_CONFIG m_ProxyConfig;
    XMLError LoadProxyConfigurations(std::string filePath);
};