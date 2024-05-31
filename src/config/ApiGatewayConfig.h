#pragma once
#include <string>
#include <vector>
#include "interface/CProxySocket.h"

typedef struct {
	std::string path;
	std::string host;
	int port;
} API_GATEWAY_URI;

typedef struct {
	std::string hostname;
	std::vector<API_GATEWAY_URI> uris;
} API_GATEWAY_SERVICE;

typedef struct {
	std::string name;
	int port;
	std::string pipeline;
	PipelineFunction<CProxySocket> pipelineFunc;
	std::vector<API_GATEWAY_SERVICE> services;
} API_GATEWAY_ENDPOINT;

typedef struct {
	std::vector<API_GATEWAY_ENDPOINT> endpoints;
} API_GATEWAY_SERVER_CONFIG;
