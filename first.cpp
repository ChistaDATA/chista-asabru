#include <stdio.h>
// #include <execinfo.h>
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
#include <map>

#include "config/ConfigSingleton.h"
#include "CProtocolSocket.h"
#include "CProxySocket.h"

using namespace std;

/* Proxy pipelines map */
typedef void *(*PipelineFunction)(CProxySocket *ptr, void *lptr);
typedef std::map<string, PipelineFunction> PipelineFunctionMap;

/* Proxy sockets map */
typedef std::map<string, CProxySocket *> ProxySocketsMap;

int startProxyServer(
    PipelineFunction pipelineFunction,
    CProxySocket *socket,
    RESOLVE_ENDPOINT_RESULT configValues
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
void *ClickHouseSSLPipeline(CProxySocket *ptr, void *lptr);
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
    // size = backtrace(array, 10);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    // backtrace_symbols_fd(array, size, STDERR_FILENO);
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
    signal(SIGPIPE, errorHandler);
    std::vector<RESOLVE_ENDPOINT_RESULT> configValues = configSingleton.LoadProxyConfigurations();

    // Create Parsers
    // CHttpParser *httpParser = new CHttpParser();

    // Create PipelineFunction mappings
    PipelineFunctionMap pipelineFunctionMap;
    pipelineFunctionMap["ClickHousePipeline"] = ClickHousePipeline;
    pipelineFunctionMap["ClickHouseSSLPipeline"] = ClickHouseSSLPipeline;
    pipelineFunctionMap["PostgreSQLPipeline"] = PostgreSQLPipeline;
    pipelineFunctionMap["MySQLPipeline"] = MySQLPipeline;


    // Create Proxy sockets mapping
    ProxySocketsMap proxySocketsMap;
    for (auto value: configValues) {
            proxySocketsMap[value.name] = new CProxySocket(value.proxyPort);
            pipelineFunctionMap[value.name] = pipelineFunctionMap[value.pipeline];
        int proxy = startProxyServer(
            pipelineFunctionMap[value.name],
            proxySocketsMap[value.name],
            value
        );
        if (proxy < 0)
            return proxy;
    }

    while (1);
    return 0;
}


int startProxyServer(
    PipelineFunction pipelineFunction,
    CProxySocket *socket,
    RESOLVE_ENDPOINT_RESULT configValue
)
{
    // Create proxies
    std::string proxyName = configValue.name;

    // Setting up ClickHouse Proxy
    if (!(*socket).SetPipeline(pipelineFunction))
    {
        cout << "Failed to set " << proxyName << " Pipeline ..!" << endl;
        return -2;
    }
    CProxyHandler *proxyHandler = (CProxyHandler *) configValue.handler;
    if (!(*socket).SetHandler(proxyHandler))
    {
        cout << "Failed to set " << proxyName << " Handler ..!" << endl;
        return -2;
    }

    if (!(*socket).SetConfigValues(configValue))
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
