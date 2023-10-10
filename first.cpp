// #include <execinfo.h>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <list>
#include <map>

#include "config/ConfigSingleton.h"
#include "CProtocolSocket.h"
#include "CProxySocket.h"
#include "CHttpProtocolHandler.h"

// #include <chrono>

using namespace std;

/* Proxy pipelines map */
typedef void *(*PipelineFunction)(CProxySocket *ptr, void *lptr);
typedef std::map<string, PipelineFunction> PipelineFunctionMap;

typedef void *(*ProtocolPipelineFunction)(CProtocolSocket *ptr, void *lptr);
typedef std::map<string, ProtocolPipelineFunction> ProtocolPipelineFunctionMap;

/* Proxy sockets map */
typedef std::map<string, CProxySocket *> ProxySocketsMap;

int startProxyServer(
    PipelineFunction pipelineFunction,
    CProxySocket *socket,
    RESOLVE_ENDPOINT_RESULT configValues);

int startProtocolServer(
    ProtocolPipelineFunction pipelineFunction,
    CProtocolSocket *socket,
    RESOLVE_ENDPOINT_RESULT configValue);

// int updateProxyConfig(CProxySocket *socket, RESOLVE_ENDPOINT_RESULT configValue)
// {
//     using namespace std::this_thread; // sleep_for, sleep_until
//     using namespace std::chrono;      // nanoseconds, system_clock, seconds

//     sleep_until(system_clock::now() + seconds(10));

//     socket->SetConfigValues(configValue);
// };

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

void *ProtocolPipeline(CProtocolSocket *ptr, void *lptr);

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
    for (auto value : configValues)
    {
        proxySocketsMap[value.name] = new CProxySocket(value.proxyPort);
        pipelineFunctionMap[value.name] = pipelineFunctionMap[value.pipeline];
        int proxy = startProxyServer(
            pipelineFunctionMap[value.name],
            proxySocketsMap[value.name],
            value);
        if (proxy < 0)
            return proxy;
    }

    CHttpProtocolHandler *httpProtocolHandler = (CHttpProtocolHandler *)configSingleton.typeFactory->GetType("CHttpProtocolHandler");

    // Register a few endpoints for demo and benchmarking
    auto say_hello = [](const simple_http_server::HttpRequest &request) -> simple_http_server::HttpResponse
    {
        simple_http_server::HttpResponse response(simple_http_server::HttpStatusCode::Ok);
        response.SetHeader("Content-Type", "text/plain");
        response.SetContent("Hello, world\n");
        return response;
    };
    auto send_html = [](const simple_http_server::HttpRequest &request) -> simple_http_server::HttpResponse
    {
        simple_http_server::HttpResponse response(simple_http_server::HttpStatusCode::Ok);
        std::string content;
        content += "<!doctype html>\n";
        content += "<html>\n<body>\n\n";
        content += "<h1>Hello, world in an Html page</h1>\n";
        content += "<p>A Paragraph</p>\n\n";
        content += "</body>\n</html>\n";

        response.SetHeader("Content-Type", "text/html");
        response.SetContent(content);
        return response;
    };

    httpProtocolHandler->RegisterHttpRequestHandler("/", simple_http_server::HttpMethod::HEAD, say_hello);
    httpProtocolHandler->RegisterHttpRequestHandler("/", simple_http_server::HttpMethod::GET, say_hello);
    httpProtocolHandler->RegisterHttpRequestHandler("/hello.html", simple_http_server::HttpMethod::HEAD, send_html);
    httpProtocolHandler->RegisterHttpRequestHandler("/hello.html", simple_http_server::HttpMethod::GET, send_html);

    ProtocolPipelineFunctionMap protocolPipelineFunctionMap;
    protocolPipelineFunctionMap["ProtocolPipeline"] = ProtocolPipeline;
    RESOLVE_ENDPOINT_RESULT protocolConfigValue = {
        .name = "Test",
        .ipaddress = "0.0.0.0",
        .handler = httpProtocolHandler};

    int protocolServer = startProtocolServer(protocolPipelineFunctionMap["ProtocolPipeline"],
                                             new CProtocolSocket(8080),
                                             protocolConfigValue);

    if (protocolServer < 0)
        return protocolServer;

    // RESOLVE_ENDPOINT_RESULT newConfigValue = configValues[1];
    // newConfigValue.ipaddress = "ssl-test.sandbox.chistadata.io";
    // newConfigValue.handler = configSingleton.typeFactory->GetType("CHttpsHandler");
    // updateProxyConfig(proxySocketsMap["ch_http_service1"], newConfigValue);

    while (1);
    return 0;
}

int startProxyServer(
    PipelineFunction pipelineFunction,
    CProxySocket *socket,
    RESOLVE_ENDPOINT_RESULT configValue)
{
    // Create proxies
    std::string proxyName = configValue.name;

    // Setting up ClickHouse Proxy
    if (!(*socket).SetPipeline(pipelineFunction))
    {
        cout << "Failed to set " << proxyName << " Pipeline ..!" << endl;
        return -2;
    }
    CProxyHandler *proxyHandler = (CProxyHandler *)configValue.handler;
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

int startProtocolServer(
    ProtocolPipelineFunction pipelineFunction,
    CProtocolSocket *socket,
    RESOLVE_ENDPOINT_RESULT configValue)
{
    // Create protocol
    std::string protocolName = configValue.name;

    // Setting up Protocol Pipeline
    if (!(*socket).SetPipeline(pipelineFunction))
    {
        cout << "Failed to set " << protocolName << " Pipeline ..!" << endl;
        return -2;
    }
    CProtocolHandler *protocolHandler = (CProtocolHandler *)configValue.handler;
    if (!(*socket).SetHandler(protocolHandler))
    {
        cout << "Failed to set " << protocolName << " Handler ..!" << endl;
        return -2;
    }

    //    if (!(*socket).SetConfigValues(configValue))
    //    {
    //        cout << "Failed to set " << protocolName << " Config values ..!" << endl;
    //        return -2;
    //    }

    if (!(*socket).Start())
    {
        cout << "Failed To Start " << protocolName << " Proxy Server ..!" << endl;
        return -3;
    }

    return 0;
}