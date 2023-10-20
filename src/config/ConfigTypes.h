#pragma once

#include <vector>
#include <string>
#include "CProxySocket.h"
#include "CProtocolSocket.h"

typedef struct {
    int port;
    std::string protocol;
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
    std::string protocol_name;
    int protocol_port;
    std::string pipeline;
    std::string handler;
} PROTOCOL_SERVER_CONFIG;

typedef struct {
    std::string protocol_name;
    int protocol_port;
    PipelineFunction<CProtocolSocket> pipeline;
    void *handler;
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
} RESOLVED_PROXY_CONFIG;

typedef struct {
    std::string node;
    std::string host;
    std::vector<SERVICE> services;
} LOCAL_END_POINT;

typedef struct {
    std::string clusterName;
    std::vector<LOCAL_END_POINT> endpoints;
} LOCAL_END_POINTS;

typedef struct {
    std::string name;
    LOCAL_END_POINTS localEndpoints;
} WORK_SPACE;
