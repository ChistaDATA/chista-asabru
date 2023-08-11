#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <spawn.h>
#include <sys/wait.h>
#include <string>
#include <iostream>
#include <thread>
#include <list>

#include "handlers/CProtocolServer.h"
#include "handlers/CHttpHandler.h"
#include "handlers/CHttpsHandler.h"
#include "config/config.h"
#include "config/ConfigSingleton.h"
// #include <openssl/ssl.h>
// #include <openssl/err.h>

using namespace std;

/* String constants to identify the proxy types */
static const string CLICKHOUSE_WIRE_LEVEL = "ClickHouseWireLevel";
static const string CLICKHOUSE_HTTP = "ClickHouseHttp";
static const string CLICKHOUSE_WIRE_LEVEL_TLS = "ClickHouseWireLevelTLS";
static const string CLICKHOUSE_HTTP_TLS = "ClickHouseHttpTLS";
static const string POSTGRESQL = "PostgreSQL";
static const string POSTGRESQL_TLS = "PostgreSQLTLS";
static const string MYSQL = "MySQL";
static const string MYSQL_TLS = "MySQLTLS";

/* Proxy pipelines map */
typedef void *(*PipelineFunction)(CProxySocket *ptr, void *lptr);
typedef map<string, PipelineFunction> PipelineFunctionMap;

/* Proxy handlers map */
typedef map<string, CProxyHandler *> ProxyHandlerMap;

/* Proxy sockets map */
typedef map<string, CProxySocket *> ProxySocketsMap;

int startProxyServer(
    string proxyName,
    RESOLVE_CONFIG *resolveConfig,
    PipelineFunctionMap *pipelineFunctionMap,
    ProxyHandlerMap *proxyHandlerMap,
    ProxySocketsMap * proxySocketsMap
);

/**
 * Struct to contain proxy configurations
 */
typedef struct
{
    string proxyName;
    RESOLVE_CONFIG resolveConfig;
} PROXY_INIT_CONFIG;

/**
 * Struct to contain database endpoint
 * and port information
 */
typedef struct
{
    char node_end_point[40];
    int port;
} NODE_PING_INFO;

void PerformLogic() {}

void *ClickHousePipeline(CProxySocket *ptr, void *lptr);
void *PostgreSQLPipeline(CProxySocket *ptr, void *lptr);
void *MySQLPipeline(CProxySocket *ptr, void *lptr);
void *PassthroughPipeLine(CProtocolSocket *ptr, void *lptr);

static ConfigSingleton &configSingleton = ConfigSingleton::getInstance();

//////////////////////////////////////////////////
// A Simple Ping Client to Test Cluster Simulator
//

#if 0
int main(int argc, char **argv )
{

     if ( argc != 2) {
	fprintf(stdout,"Failed to Start the app\n");
	return -1;
    }
    int rc = atoi(argv[1]);
    cout << "Received from Command line " << rc << endl;
    CPing ps("localhost",7086);
    NODE_PING_INFO  pdu{"localhost", rc};
    if ( !ps.Open() ) {
                 fprintf(stdout, "Failed to Open The Client\n" );
                 return 0;
    }
    while (1) {
           sleep(10);
           PerformLogic();
           ps.SendSignal(&pdu,sizeof(pdu));
    }

    ps.Close();
    return 0;
}

#endif

/**
 * Error handler
 */
void errorHandler(int sig)
{
    void *array[10];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 10);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}

/**
 * Database Proxy main function
 * @arg port number
 */
int main(int argc, char **argv)
{
    // install our error handler
    signal(SIGSEGV, errorHandler);

    // Parse arguments
    if (argc != 2)
    {
        fprintf(stdout, "PORT must be passed as argument. Failed to Start the app\n");
        return -1;
    }
    int port = atoi(argv[1]);
    port = (port > 0) ? port : 9100;
    cout << "Received from Command line " << port << endl;

    // Create Parsers
    CHttpParser *httpParser = new CHttpParser();

    // Create PipelineFunction mappings
    PipelineFunctionMap pipelineFunctionMap;
    pipelineFunctionMap[CLICKHOUSE_WIRE_LEVEL] = ClickHousePipeline;
    pipelineFunctionMap[CLICKHOUSE_HTTP] = ClickHousePipeline;
    pipelineFunctionMap[CLICKHOUSE_WIRE_LEVEL_TLS] = ClickHousePipeline;
    pipelineFunctionMap[CLICKHOUSE_HTTP_TLS] = ClickHousePipeline;
    pipelineFunctionMap[POSTGRESQL] = PostgreSQLPipeline;
    pipelineFunctionMap[POSTGRESQL_TLS] = PostgreSQLPipeline;
    pipelineFunctionMap[MYSQL] = MySQLPipeline;
    pipelineFunctionMap[MYSQL_TLS] = MySQLPipeline;

    // Create ProxyHandler mappings
    ProxyHandlerMap proxyHandlerMap;
    proxyHandlerMap[CLICKHOUSE_WIRE_LEVEL] = new CHWirePTHandler();
    proxyHandlerMap[CLICKHOUSE_HTTP] = new CHttpHandler(httpParser);
    proxyHandlerMap[CLICKHOUSE_WIRE_LEVEL_TLS] = new CHWirePTHandler();
    proxyHandlerMap[CLICKHOUSE_HTTP_TLS] = new CHttpsHandler();
    proxyHandlerMap[POSTGRESQL] = new CPostgreSQLHandler();
    proxyHandlerMap[POSTGRESQL_TLS] = new CPostgreSQLHandler();
    proxyHandlerMap[MYSQL] = new CMySQLHandler();
    proxyHandlerMap[MYSQL_TLS] = new CMySQLHandler();

    // Create Proxy sockets mapping
    ProxySocketsMap proxySocketsMap;
    proxySocketsMap[CLICKHOUSE_WIRE_LEVEL] = new CProxySocket(9100);
    proxySocketsMap[CLICKHOUSE_HTTP] = new CProxySocket(9110);
    proxySocketsMap[CLICKHOUSE_WIRE_LEVEL_TLS] = new CProxySocket(9120);
    proxySocketsMap[CLICKHOUSE_HTTP_TLS] = new CProxySocket(9130);
    proxySocketsMap[POSTGRESQL] = new CProxySocket(9140);
    proxySocketsMap[POSTGRESQL_TLS] = new CProxySocket(9150);
    proxySocketsMap[MYSQL] = new CProxySocket(9160);
    proxySocketsMap[MYSQL_TLS] = new CProxySocket(9170);

    // Create Proxy Configs
    PROXY_INIT_CONFIG proxyConfigs[] = {
        {.proxyName = CLICKHOUSE_WIRE_LEVEL, .resolveConfig = {"cluster1", "ch_wire_http_node1", "ch_wire_service1"}},
        {.proxyName = CLICKHOUSE_HTTP, .resolveConfig = {"cluster1", "ch_wire_http_node1", "ch_http_service1"}},
        {.proxyName = CLICKHOUSE_WIRE_LEVEL_TLS, .resolveConfig = {"cluster1", "ch_tls_node1", "ch_tls_wire_service1"}},
        {.proxyName = CLICKHOUSE_HTTP_TLS, .resolveConfig = {"cluster1", "ch_tls_node1", "ch_tls_http_service1"}},
        {.proxyName = POSTGRESQL, .resolveConfig =  {"cluster2", "pg_node1", "pg_wire_service1"}},
        {.proxyName = POSTGRESQL_TLS, .resolveConfig =  {"cluster2", "pg_node1", "pg_tls_service1"}},
        {.proxyName = MYSQL, .resolveConfig =  {"cluster2", "mysql_node1", "mysql_wire_service1"}},
        {.proxyName = MYSQL_TLS, .resolveConfig =  {"cluster2", "mysql_node1", "mysql_tls_service1"}},
    };
    int numberOfProxies = sizeof proxyConfigs / sizeof proxyConfigs[0];

    
    for (int i = 0; i < numberOfProxies; i++)
    {
        int proxy = startProxyServer(
            proxyConfigs[i].proxyName,
            &proxyConfigs[i].resolveConfig,
            &pipelineFunctionMap,
            &proxyHandlerMap,
            &proxySocketsMap
        );
        if (proxy < 0)
            return proxy;
    }

    while (1);
    return 0;
}


int startProxyServer(
    string proxyName,
    RESOLVE_CONFIG *resolveConfig,
    PipelineFunctionMap *pipelineFunctionMap,
    ProxyHandlerMap *proxyHandlerMap,
    ProxySocketsMap * proxySocketsMap
)
{
    // Create proxies
    CProxySocket *socket = (*proxySocketsMap)[proxyName];
    auto configValues = configSingleton.Resolve(*resolveConfig);

    // Setting up ClickHouse Proxy
    if (!(*socket).SetPipeline((*pipelineFunctionMap)[proxyName]))
    {
        cout << "Failed to set " << proxyName << " Pipeline ..!" << endl;
        return -2;
    }

    if (!(*socket).SetHandler((*proxyHandlerMap)[proxyName]))
    {
        cout << "Failed to set " << proxyName << " Handler ..!" << endl;
        return -2;
    }

    if (!(*socket).SetConfigValues(configValues))
    {
        cout << "Failed to set " << proxyName << " Config values ..!" << endl;
        return -2;
    }

    if (!(*socket).Start(proxyName))
    {
        cout << "Failed To Start " << proxyName << " Proxy Server ..!" << endl;
        return -3;
    }

    return 0;
}
