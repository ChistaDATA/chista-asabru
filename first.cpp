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

/* Proxy sockets map */
typedef std::map<string, CProxySocket *> ProxySocketsMap;
typedef std::map<string, CProtocolSocket *> ProtocolSocketsMap;

int startProxyServer(
    CProxySocket *socket,
    RESOLVED_PROXY_CONFIG configValues);

int startProtocolServer(
    CProtocolSocket *socket,
    RESOLVED_PROTOCOL_CONFIG configValue);

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

static ConfigSingleton &configSingleton = ConfigSingleton::getInstance();

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
    std::vector<RESOLVED_PROXY_CONFIG> configValues = configSingleton.ResolveProxyServerConfigurations();

    // Create Proxy sockets mapping
    ProxySocketsMap proxySocketsMap;
    for (auto value : configValues)
    {
        proxySocketsMap[value.name] = new CProxySocket(value.proxyPort);
        int proxy = startProxyServer(
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

    ProtocolSocketsMap protocolSocketsMap;
    std::vector<RESOLVED_PROTOCOL_CONFIG> protocolServerConfigValues = configSingleton.ResolveProtocolServerConfigurations();
    for (auto value : protocolServerConfigValues)
    {
        protocolSocketsMap[value.protocol_name] = new CProtocolSocket(value.protocol_port);
        int protocolServer = startProtocolServer(
            protocolSocketsMap[value.protocol_name],
            value);
        if (protocolServer < 0)
            return protocolServer;
    }

    // RESOLVE_ENDPOINT_RESULT newConfigValue = configValues[1];
    // newConfigValue.ipaddress = "ssl-test.sandbox.chistadata.io";
    // newConfigValue.handler = configSingleton.typeFactory->GetType("CHttpsHandler");
    // updateProxyConfig(proxySocketsMap["ch_http_service1"], newConfigValue);

    while (1)
        ;
    return 0;
}

int startProxyServer(
    CProxySocket *socket,
    RESOLVED_PROXY_CONFIG configValue)
{
    // Create proxies
    std::string proxyName = configValue.name;

    // Setting up ClickHouse Proxy
    PipelineFunction<CProxySocket> pipelineFunction = configValue.pipeline;
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

    TARGET_ENDPOINT_CONFIG targetEndpointConfig = {
        .name = configValue.name,
        .proxyPort = configValue.proxyPort,
        .services = configValue.services
    };

    if (!(*socket).SetConfigValues(targetEndpointConfig))
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
    CProtocolSocket *socket,
    RESOLVED_PROTOCOL_CONFIG configValue)
{
    // Create protocol
    std::string protocolName = configValue.protocol_name;

    // Setting up Protocol Pipeline
    PipelineFunction<CProtocolSocket> pipelineFunction = configValue.pipeline;
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