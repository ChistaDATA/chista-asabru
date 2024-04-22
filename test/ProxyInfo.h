#ifndef PROXY_INFO_H
#define PROXY_INFO_H

#include <stdio.h>
#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <cstring>
#include "config/config_types.h"
#include "../src/config/ConfigTypes.h"

///////////////////////////////////////////////////
// Persistence - SQLite DB calls
// Integrations
// Model - C++ STL and composite entities
// DTO - Data Transfer Objects via C/C++ structs
// API
// Invocation
// Interface
//===================================
// The following struct represents an Endpoint and Proxies to
// which it can map to
typedef struct {
    std::string key;   // ip address of the incoming request
    int port;       // port at which incoming request is being listened
    std::vector<END_POINT> proxies;  // potentially reachable clickhouse masters
} PROXY_ENTRY;

////////////////////////////////
class MasterList {
    std::vector<END_POINT> m_endpoints;
    std::string m_ipaddress;
    int m_port;
public:
    MasterList() { }
    bool Add( END_POINT pt ) {
        m_endpoints.push_back(pt);
        return true;
    }
    void Set( std::string ipaddress, int port ) {
        m_ipaddress = ipaddress;
        m_port = port;
    }
    std::vector<END_POINT>&  GetEndPoints() { return m_endpoints; }
    std::string GetIpAddress() { return m_ipaddress; }
    int GetPort( ) { return m_port; }

};
/////////////////////////////////////////
//
//
class CProxyInfo {
    std::map<std::string, PROXY_ENTRY>  proxy_map;
    void LoadProxyMap();
public:
    CProxyInfo();
    ~CProxyInfo();
    bool DeleteMaster( std::string key , std::string addr );
    bool FindMaster( std::string key, std::string addr );
    bool AddProxyList( std::string key , int port, std::vector<END_POINT>& entries);
    bool AddProxy(std::string key , int port, END_POINT entry );
    void DumpProxyMap( );
    const END_POINT* Resolve( std::string ip , int port , int r_w  );
};
////////////////////////////////////////////
//
//
//
class ClusterInfo {
    std::map<std::string, CProxyInfo>  clusters;
public:
    ClusterInfo();
    ~ClusterInfo();
    bool AddCluster(std::string name);
    bool  AddClusterNode( std::string cluster_name, std::string ipaddress , int port, std::vector<END_POINT>& masters );
    bool SpitCluster(std::string cluster_name);
    const END_POINT* Resolve( std::string cluster_name, std::string ip , int port , int r_w  );
};

#endif
