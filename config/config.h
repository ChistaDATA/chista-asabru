#pragma once

#include <string>
#include <map>
#include <vector>
#include <list>

#include "../lib/tinyxml2/tinyxml2.h"

#include "config_types.h"
#include "IConfigEngine.h"

using namespace tinyxml2;

#ifndef XMLCheckResult
#define XMLCheckResult(a_eResult) if (a_eResult != XML_SUCCESS) { printf("Error: %i\n", a_eResult); return a_eResult; }
#endif


class Config : public IConfigEngine
{
public:

    Config();

    XMLError LoadProxyConfigurations(std::string filePath);
    SERVICE Resolve(RESOLVE_CONFIG config);

    PROXY_CONFIG GetProxyConfig();
private:

    PROXY_CONFIG m_ProxyConfig;
};
