#pragma once

#include <vector>
#include <string>
#include "CProxySocket.h"
#include "CProtocolSocket.h"
#include "LoadBalancerStrategy.h"

typedef struct {
    int port;
    int weight;
    std::string name;
    std::string host;
} SERVICE;

typedef struct {
    std::string endPointName;
    int proxyPort;
    bool readWrite;
    std::vector<SERVICE> services;
    std::string ipaddress;
    std::string handler;
    std::string pipeline;
    std::string loadBalancerStrategy;
} REMOTE_END_POINT;

typedef struct {
    std::vector<REMOTE_END_POINT> endPoints;
    std::string clusterName;
} CLUSTER;

typedef struct {
    std::vector<CLUSTER> clusters;
    std::vector<std::string> handlers;
} PROXY_CONFIG;

typedef struct {
    std::string path;
    std::string method;
    std::string request_handler;
} Route;

typedef struct {
    std::string protocol_name;
    int protocol_port;
    std::string pipeline;
    std::string handler;
    std::vector<Route> routes;
} PROTOCOL_SERVER_CONFIG;

typedef struct {
    std::string protocol_name;
    int protocol_port;
    PipelineFunction<CProtocolSocket> pipeline;
    void *handler;
    std::vector<Route> routes;
} RESOLVED_PROTOCOL_CONFIG;

typedef struct {
    std::string clusterName;
    std::string endPointName;
    std::string serviceName;
    std::string handlerName;
    int port;
    std::string protocol;
} RESOLVE_CONFIG;

typedef struct
{
    std::string name; // name of the proxy server
    int proxyPort;    // port at which the proxy listens
    std::string pipelineName;
    PipelineFunction<CProxySocket> pipeline;
    void *handler; // handler for this endpoint
    std::vector<RESOLVED_SERVICE> services;
    LoadBalancerStrategy<RESOLVED_SERVICE>* loadBalancerStrategy;
} RESOLVED_PROXY_CONFIG;


typedef struct {
    std::string name;
    std::string operation;
    SERVICE service;
} ENDPOINT_SERVICE_CONFIG;
