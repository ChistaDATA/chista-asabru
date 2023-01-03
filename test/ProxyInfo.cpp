#include <stdio.h>
#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <cstring>

using namespace std;

#include "ProxyInfo.h"

///////////////////////////////////////////////
//  Load Proxy map
//
//
void CProxyInfo::LoadProxyMap(){
    /*
         proxy_map["127.0.0.1"] =  PROXY_ENTRY{ "127.0.0.1" , 9000,
                                                                      vector<END_POINT> {
                                                                                        END_POINT { "127.0.0.2" ,9001 ,1, "",0,"  "},
                                                                                       END_POINT {  "127.0.0.3" ,9003,0,"",0,"   "}
                                                                        }};
         proxy_map["192.168.0.1"] =  PROXY_ENTRY{ "192.168.0.1" , 9000,
                                                                      vector<END_POINT> {
                                                                                        END_POINT { "127.0.0.3" ,9001 , 1,"",0,"  "},
                                                                                       END_POINT {  "127.0.0.4" ,9003,1,"",0,"   "}
                                                                      }};
         return;
      */
}


CProxyInfo::CProxyInfo() {LoadProxyMap();}
CProxyInfo::~CProxyInfo() { proxy_map.clear(); }

bool  CProxyInfo::DeleteMaster( string key , string addr ) {
    auto p = proxy_map.find(key);
    if ( p == proxy_map.end() )  { return true; }
    PROXY_ENTRY& pe = p->second;
    vector<END_POINT>::iterator it = pe.proxies.begin();
    while ( it != pe.proxies.end() ) {
        if ( it->ipaddress== addr )
        {
            pe.proxies.erase(it);
            return true;
        }
        it++;
    }
    return false;
}
//////////////////////////////////////////////////////////
//
//
bool CProxyInfo::FindMaster( string key, string addr ) {
    auto p = proxy_map.find(key);
    if ( p == proxy_map.end() )  { return false; }
    PROXY_ENTRY& pe = p->second;
    vector<END_POINT>::iterator it = pe.proxies.begin();
    while ( it != pe.proxies.end() ) {
        if ( it->ipaddress== addr )
        {

            return true;
        }
        it++;
    }
    return false;
}

bool CProxyInfo::AddProxyList( string key , int port, vector<END_POINT>& entries) {
    for(auto lst : entries ) {
        AddProxy(key, port,lst);
    }
    return true;
}
bool CProxyInfo::AddProxy(string key , int port, END_POINT entry ) {
    auto p  = proxy_map.find(key);
    if ( p  == proxy_map.end())  {
        PROXY_ENTRY  pe_entry;
        pe_entry.key = key;
        pe_entry.port = port;
        pe_entry.proxies.push_back(entry);
        proxy_map[key] = pe_entry;
        return true;
    }
    PROXY_ENTRY& pe = p->second;
    pe.proxies.push_back(entry);
    return true;
}
const END_POINT* CProxyInfo::Resolve(  string ip , int port , int r_w  ) {
    auto p = proxy_map.find(ip);
    if ( p == proxy_map.end() )  { return 0; }
    PROXY_ENTRY& pe = p->second;
    vector<END_POINT>::iterator it = pe.proxies.begin();
    while ( it != pe.proxies.end() ) {
        cout << "  = > " << ip << " port " << port  << "   " << it->ipaddress << " r/w " << r_w << endl;
        if ( it->ipaddress== ip  )
        {
            END_POINT& ep = *it;
            cout << "---------------------" << "<<<>>>" << endl;
            if ( ep.r_w == r_w  )  { return &ep; } else { return 0; }

        }
        it++;
    }
    return 0;

}

void CProxyInfo::DumpProxyMap( ){
    for(auto &e : proxy_map)   {
        cout << e.first << endl;
        for(auto&f : e.second.proxies )
            cout << "       " << f.ipaddress << "   " <<f.port <<  endl;
        cout <<"=======================" << endl;

    }
}



ClusterInfo::ClusterInfo() {}
ClusterInfo::~ClusterInfo() { clusters.clear(); }

bool ClusterInfo::AddCluster(string name) {
    auto it = clusters.find(name);
    if ( it  != clusters.end() ) { return false; }
    clusters[ name ] = CProxyInfo {};
    return true;
}
bool ClusterInfo::AddClusterNode( string cluster_name, string ipaddress , int port, vector<END_POINT>& masters ) {
    auto it = clusters.find(cluster_name);
    if ( it == clusters.end() ) { AddCluster(cluster_name);  }
    CProxyInfo& other = clusters[cluster_name];
    return other.AddProxyList( ipaddress,port, masters );

}

bool ClusterInfo::SpitCluster(string cluster_name) {
    auto it = clusters.find(cluster_name);
    if ( it == clusters.end() ) { cout << "Cluster does not Exists"  << endl; return false; }
    CProxyInfo& other = clusters[cluster_name];
    other.DumpProxyMap();
    return true;

}

const END_POINT* ClusterInfo::Resolve( string cluster_name, string ip , int port , int r_w  )  {
    auto it = clusters.find(cluster_name);
    if ( it == clusters.end() ) { return 0; }
    cout << "Found Cluster " << endl;
    CProxyInfo& other = clusters[cluster_name];
    return other.Resolve(ip,port,r_w);
}

#ifdef MAIN_INCLUDED

int main( int argc, char **argv )
{
          MasterList lst;
          lst.Set("127.0.0.1",9000);
          END_POINT  first{ "127.0.0.1" ,9001 ,1, "",0,"  "};
          lst.Add( first);
          END_POINT  first2{ "127.0.0.1" ,9002 ,0, "",0,"  "};
          lst.Add( first2);
          END_POINT  first3{ "127.0.0.1" ,9003 ,1, "",0,"  "};
          lst.Add( first3);
          ClusterInfo cls;
          cls.AddClusterNode("firstcluster", lst.GetIpAddress(), lst.GetPort() , lst.GetEndPoints());
          cls.SpitCluster( "firstcluster" );



}
#endif


