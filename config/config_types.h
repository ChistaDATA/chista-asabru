#pragma once

#include<iostream>
#include<vector>

using namespace std;

typedef struct
{
    int		port;
    string	protocol;
    string name;

}SERVICE;

typedef struct
{
    string node;
    string host;
    vector<SERVICE> services;

}LOCAL_END_POINT;

typedef struct
{
    string clusterName;
    vector<LOCAL_END_POINT> endpoints;

}LOCAL_END_POINTS;

typedef struct
{
    string name;
    LOCAL_END_POINTS localEndpoints;

}WORK_SPACE;

typedef struct
{
    string endPointName;
    bool readWrite;
    string host;
    vector<SERVICE> services;

}END_POINT;

typedef struct
{
    vector<END_POINT> endPoints;
    string clusterName;

}CLUSTER;

typedef struct
{
    vector< WORK_SPACE> workspaces;
    vector<CLUSTER> clusters;

}PROXY_CONFIG;


// Resolve type

typedef struct
{
    string	clusterName;
    string	endPointName;
    string	serviceName;
    int		port;
    string	protocol;

}RESOLVE_CONFIG;
