#pragma once
#include "tinyxml2.h"
#include "logger/Logger.h"
#include "CommonTypes.h"
#include "ConfigTypes.h"
#include "ConfigParser.h"
#include "ApiGatewayConfig.h"

using namespace tinyxml2;

class ApiGatewayConfigParser {
  public:
	static XMLError ParseConfiguration(XMLNode *xmlDoc, API_GATEWAY_SERVER_CONFIG &config);
};

