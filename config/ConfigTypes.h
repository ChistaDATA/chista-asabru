#pragma once
#include <vector>
#include <string>
#include "CProxySocket.h"
#include "CProtocolSocket.h"

typedef struct
{
    int proxyPort;
    int port;
    std::string protocol;
    std::string name;
    std::string handler;
    std::string pipeline;
} SERVICE;

typedef struct
{
    std::string endPointName;
    bool readWrite;
    std::string host;
    std::vector<SERVICE> services;
    std::string ipaddress;
} REMOTE_END_POINT;

typedef struct
{
    std::vector<REMOTE_END_POINT> endPoints;
    std::string clusterName;
} CLUSTER;

typedef struct
{
    std::vector<CLUSTER> clusters;
    std::vector<std::string> handlers;
} PROXY_CONFIG;

typedef struct
{
    std::string protocol_name;
    int protocol_port;
    std::string pipeline;
    std::string handler;
} PROTOCOL_SERVER_CONFIG;

typedef struct
{
    std::string protocol_name;
    int protocol_port;
    PipelineFunction<CProtocolSocket> pipeline;
    void *handler;
} RESOLVED_PROTOCOL_CONFIG;

typedef struct
{
    std::string clusterName;
    std::string endPointName;
    std::string serviceName;
    std::string handlerName;
    int port;
    std::string protocol;
} RESOLVE_CONFIG;

typedef struct
{
    std::string name;       // name of the proxy server
    std::string ipaddress;  // ip address of the Remote Endpoint
    int proxyPort;          // port at which the proxy listens
    int port;               // target port to which proxy connects
    int r_w;                //  Read Endpoint or Write EndPoint
    std::string alias;      //  unused
    float reserved;         //  unused
    char Buffer[255];       // unused
    void *handler;
    PipelineFunction<CProxySocket> pipeline;
} RESOLVE_ENDPOINT_RESULT;

typedef struct
{
    std::string node;
    std::string host;
    std::vector<SERVICE> services;
} LOCAL_END_POINT;

typedef struct
{
    std::string clusterName;
    std::vector<LOCAL_END_POINT> endpoints;
} LOCAL_END_POINTS;

typedef struct
{
    std::string name;
    LOCAL_END_POINTS localEndpoints;
} WORK_SPACE;
