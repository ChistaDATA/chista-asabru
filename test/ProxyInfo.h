#ifndef PROXY_INFO_H
#define PROXY_INFO_H

#include <stdio.h>
#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <cstring>
#include "config_types.h"

using namespace std;

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
    string key;   // ip address of the incoming request
    int port;       // port at which incoming request is being listened
    vector<END_POINT> proxies;  // potentially reachable clickhouse masters
} PROXY_ENTRY;

////////////////////////////////
class MasterList {
    vector<END_POINT> m_endpoints;
    string m_ipaddress;
    int m_port;
public:
    MasterList() { }
    bool Add( END_POINT pt ) {
        m_endpoints.push_back(pt);
        return true;
    }
    void Set( string ipaddress, int port ) {
        m_ipaddress = ipaddress;
        m_port = port;
    }
    vector<END_POINT>&  GetEndPoints() { return m_endpoints; }
    string GetIpAddress() { return m_ipaddress; }
    int GetPort( ) { return m_port; }

};
/////////////////////////////////////////
//
//
class CProxyInfo {
    map<string, PROXY_ENTRY>  proxy_map;
    void LoadProxyMap();
public:
    CProxyInfo();
    ~CProxyInfo();
    bool DeleteMaster( string key , string addr );
    bool FindMaster( string key, string addr );
    bool AddProxyList( string key , int port, vector<END_POINT>& entries);
    bool AddProxy(string key , int port, END_POINT entry );
    void DumpProxyMap( );
    const END_POINT* Resolve( string ip , int port , int r_w  );
};
////////////////////////////////////////////
//
//
//
class ClusterInfo {
    map<string, CProxyInfo>  clusters;
public:
    ClusterInfo();
    ~ClusterInfo();
    bool AddCluster(string name);
    bool  AddClusterNode( string cluster_name, string ipaddress , int port, vector<END_POINT>& masters );
    bool SpitCluster(string cluster_name);
    const END_POINT* Resolve( string cluster_name, string ip , int port , int r_w  );
};

#endif
