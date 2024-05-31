#pragma once
#include "interface/CApiGatewaySocket.h"

int startGatewayServer(API_GATEWAY_ENDPOINT endpoint, CApiGatewaySocket *socket,
					   PipelineFunction<CApiGatewaySocket> pipelineFunction);

int initApiGatewayServers() {
	API_GATEWAY_SERVER_CONFIG apiGatewayServerConfig = configSingleton.getApiGatewayConfig();

	for (const auto &endpoint : apiGatewayServerConfig.endpoints) {
		configSingleton.pipelineFactory->registerPipeline<CApiGatewaySocket>(endpoint.pipeline);

		configSingleton.apiGatewaySocketsMap[endpoint.name] = new CApiGatewaySocket(endpoint.port);

		int proxy = startGatewayServer(
			endpoint,
			configSingleton.apiGatewaySocketsMap[endpoint.name],
			configSingleton.pipelineFactory->GetPipeline<CApiGatewaySocket>(endpoint.pipeline));
		if (proxy < 0)
			return proxy;
	}
	return 0;
}

int startGatewayServer(API_GATEWAY_ENDPOINT endpoint, CApiGatewaySocket *socket,
					   PipelineFunction<CApiGatewaySocket> pipelineFunction) {

	if (!(*socket).SetPipeline(pipelineFunction)) {
		LOG_ERROR("Failed to set " + endpoint.name + " Pipeline ..!");
		return -2;
	}

	std::map<std::string, std::map<std::string, RESOLVED_SERVICE>> host_map;
	for (auto service: endpoint.services) {
		std::map<std::string, RESOLVED_SERVICE> uri_map;
		for (auto uri: service.uris) {

			RESOLVED_SERVICE resolvedService {
				.ipaddress = uri.host,
				.port = uri.port,
			};
			uri_map.emplace(uri.path, resolvedService);
		}
		host_map[service.hostname] = uri_map;
	}
	(*socket).SetApiGatewayConfig(host_map);

	if (!(*socket).Start(endpoint.name)) {
		LOG_ERROR("Failed To Start " + endpoint.name + " API Gateway Server ..!");
		return -3;
	}

	return 0;
}