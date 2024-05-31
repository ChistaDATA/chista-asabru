#include "ApiGatewayConfigParser.h"

XMLError LoadApiGatewayConfigurations(XMLNode *pRoot, API_GATEWAY_SERVER_CONFIG &config);

XMLError ApiGatewayConfigParser::ParseConfiguration(XMLNode *pRoot, API_GATEWAY_SERVER_CONFIG &config) {
	/**
	 * Get API Gateway Server configurations
	 */
	LoadApiGatewayConfigurations(pRoot, config);

	LOG_INFO("API Gateway configuration parsed successfully!");
	return XML_SUCCESS;
}

XMLError LoadApiGatewayConfigurations(XMLNode *pRoot, API_GATEWAY_SERVER_CONFIG &config) {
	API_GATEWAY_SERVER_CONFIG apiGatewayServerConfig;

	XMLElement *pApiGatewayConfig = pRoot->FirstChildElement("api-gateway-server-config");
	if (pApiGatewayConfig == nullptr) return XML_ERROR_PARSING_ELEMENT;


	XMLElement *pEndPoint = pApiGatewayConfig->FirstChildElement("end-point");
	while (pEndPoint) {
		API_GATEWAY_ENDPOINT endpoint;
		endpoint.name = pEndPoint->Attribute("name");

		XMLElement *pPort = pEndPoint->FirstChildElement("port");
		if (nullptr != pPort) {
			endpoint.port = std::stoi(pPort->GetText());
		}

		XMLElement *pPipeline = pEndPoint->FirstChildElement("pipeline");
		if (nullptr != pPipeline) {
			endpoint.pipeline = pPipeline->GetText();
		}

		XMLElement *pServices = pEndPoint->FirstChildElement("services");
		if (nullptr == pServices) {
			continue;
		}

		XMLElement *pService = pServices->FirstChildElement("service");
		while(pService) {
			API_GATEWAY_SERVICE service;
			service.hostname = pService->Attribute("hostname");

			XMLElement *pUri = pService->FirstChildElement("uri");
			while(pUri) {
				API_GATEWAY_URI uri;
				uri.path = pUri->Attribute("path");

				XMLElement *pHost = pUri->FirstChildElement("host");
				if (nullptr != pHost) {
					uri.host = pHost->GetText();
				}

				XMLElement *pUriPort = pUri->FirstChildElement("port");
				if (nullptr != pUriPort) {
					uri.port = std::stoi(pUriPort->GetText());
				}
				service.uris.emplace_back(uri);
				pUri = pUri->NextSiblingElement("uri");
			}

			endpoint.services.emplace_back(service);
			pService = pService->NextSiblingElement("service");
		}

		apiGatewayServerConfig.endpoints.emplace_back(endpoint);
		pEndPoint = pEndPoint->NextSiblingElement("end-point");
	}
	config = apiGatewayServerConfig;
	return XML_SUCCESS;
}
