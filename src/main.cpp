// #include <execinfo.h>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <list>
#include <map>
#include <iostream>
#include <fstream>
#include <string>
/*for getting file size using stat()*/
#include <sys/stat.h>

/*for sendfile()*/
// #include <sys/sendfile.h>

/*for O_RDONLY*/
#include <fcntl.h>

#include "config/ConfigSingleton.h"
#include "CProtocolSocket.h"
#include "CProxySocket.h"
#include "LibuvServerSocket.h"
#include "CHttpProtocolHandler.h"

using namespace std;

/* Proxy sockets map */
typedef std::map<string, CProxySocket *> ProxySocketsMap;
typedef std::map<string, CProtocolSocket *> ProtocolSocketsMap;

/**
 * Struct to contain proxy configurations
 */
typedef struct {
    string proxyName;
    RESOLVE_CONFIG resolveConfig;
} PROXY_INIT_CONFIG;

/**
 * Struct to contain database endpoint
 * and port information
 */
typedef struct {
    char node_end_point[40];
    int port;
} NODE_PING_INFO;

static ConfigSingleton &configSingleton = ConfigSingleton::getInstance();

int startProxyServer(
        CProxySocket *socket,
        RESOLVED_PROXY_CONFIG configValues);

int startLibuvProxyServer(
        LibuvProxySocket *socket,
        PipelineFunction<LibuvProxySocket> pipelineFunction,
        RESOLVED_PROXY_CONFIG configValue
);

int startProtocolServer(
        CProtocolSocket *socket,
        RESOLVED_PROTOCOL_CONFIG configValue);

int updateProxyConfig(CProxySocket *socket, RESOLVED_PROXY_CONFIG configValue) {
    TARGET_ENDPOINT_CONFIG targetEndpointConfig = {
            .name = configValue.name,
            .proxyPort = configValue.proxyPort,
            .services = configValue.services};
    socket->SetConfigValues(targetEndpointConfig);

    return 0;
};

int initProxyServers(ProxySocketsMap *proxySocketsMap) {
    std::vector<RESOLVED_PROXY_CONFIG> configValues = configSingleton.ResolveProxyServerConfigurations();
    PipelineFactory pipelineFactory;
    for (auto value: configValues) {
        if (value.pipelineName == "ClickHouseLibuvPipeline") {
            int proxy = startLibuvProxyServer(
                    new LibuvProxySocket(value.proxyPort),
                    pipelineFactory.GetLibuvProxyPipeline(value.pipelineName),
                    value);
            if (proxy < 0)
                return proxy;
        } else {
            (*proxySocketsMap)[value.name] = new CProxySocket(value.proxyPort);
            int proxy = startProxyServer(
                    (*proxySocketsMap)[value.name],
                    value);
            if (proxy < 0)
                return proxy;
        }


    }
    return 0;
}

std::string updateProxyServers(ProxySocketsMap *proxySocketsMap) {
    std::vector<RESOLVED_PROXY_CONFIG> configValues = configSingleton.ResolveProxyServerConfigurations();
    try {
        for (auto value: configValues) {
            updateProxyConfig((*proxySocketsMap)[value.name], value);
        }
    }
    catch (std::exception &e) {
        cout << e.what() << endl;
        return "Error occurred when updating the configurations : " + std::string(e.what());
    }

    return "Configuration Updated Successfully!\n";
}

/**
 * Error handler
 */
void errorHandler(int sig) {
    void *array[10];
    size_t size;

    // get void*'s for all entries on the stack
    // size = backtrace(array, 10);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);

    pid_t myPid = getpid();
    std::string pstackCommand = "pstack ";
    std::stringstream ss;
    ss << myPid;
    pstackCommand += ss.str();
    system(pstackCommand.c_str());
    // backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}

/**
 * Database Proxy main function
 * @arg port number
 */
int main(int argc, char **argv) {
    // install our error handler
    signal(SIGSEGV, errorHandler);
    signal(SIGPIPE, errorHandler);

    // Create Proxy sockets mapping
    ProxySocketsMap proxySocketsMap;

    int returnValue = initProxyServers(&proxySocketsMap);
    if (returnValue < 0) {
        cout << "Error occurred during initializing proxy servers!";
        exit(1);
    }

    CHttpProtocolHandler *httpProtocolHandler = (CHttpProtocolHandler *) configSingleton.typeFactory->GetType(
            "CHttpProtocolHandler");

    // Register a few endpoints for demo and benchmarking
    auto update_configuration = [&proxySocketsMap](
            const simple_http_server::HttpRequest &request) -> simple_http_server::HttpResponse {
        configSingleton.LoadConfigurationsFromString(request.content());
        std::string response_content = updateProxyServers(&proxySocketsMap);
        simple_http_server::HttpResponse response(simple_http_server::HttpStatusCode::Ok);
        response.SetHeader("Content-Type", "text/plain");
        response.SetContent(response_content);
        return response;
    };
    auto send_html = [](const simple_http_server::HttpRequest &request) -> simple_http_server::HttpResponse {
        simple_http_server::HttpResponse response(simple_http_server::HttpStatusCode::Ok);
        std::string content = "";

        /*
            char header[] =
            "HTTP/1.1 200 Ok\r\n"
            "Content-Type: text/html\r\n\r\n";
        */

        // write(fd, head, strlen(head));
        std::string publicFolderPath = std::getenv("PUBLIC_FOLDER_PATH");
        std::string file_path = publicFolderPath + "/index.html";
        std::ifstream myfile(file_path);

        std::string temp;
        if (myfile.is_open()) { // always check whether the file is open
            while (myfile) // equivalent to myfile.good()
            {
                std::getline(myfile, temp);
                content += temp;
                content += "\n";
            }
        }
        response.SetHeader("Content-Type", "text/html");
        response.SetContent(content);
        return response;
    };

    httpProtocolHandler->RegisterHttpRequestHandler("/configuration", simple_http_server::HttpMethod::POST,
                                                    update_configuration);
    httpProtocolHandler->RegisterHttpRequestHandler("/", simple_http_server::HttpMethod::GET, send_html);

    ProtocolSocketsMap protocolSocketsMap;
    std::vector<RESOLVED_PROTOCOL_CONFIG> protocolServerConfigValues = configSingleton.ResolveProtocolServerConfigurations();
    for (auto value: protocolServerConfigValues) {
        protocolSocketsMap[value.protocol_name] = new CProtocolSocket(value.protocol_port);
        int protocolServer = startProtocolServer(
                protocolSocketsMap[value.protocol_name],
                value);
        if (protocolServer < 0)
            return protocolServer;
    }

    while (1);
    return 0;
}

int startLibuvProxyServer(
        LibuvProxySocket *socket,
        PipelineFunction<LibuvProxySocket> pipelineFunction,
        RESOLVED_PROXY_CONFIG configValue
) {
    // Create proxies
    std::string proxyName = configValue.name;

    if (!(*socket).
            SetPipeline(pipelineFunction)
            ) {
        cout << "Failed to set " << proxyName << " Pipeline ..!" <<
             endl;
        return -2;
    }
    CProxyHandler *proxyHandler = (CProxyHandler *) configValue.handler;
    if (!(*socket).
            SetHandler(proxyHandler)
            ) {
        cout << "Failed to set " << proxyName << " Handler ..!" <<
             endl;
        return -2;
    }

    TARGET_ENDPOINT_CONFIG targetEndpointConfig = {
            .name = configValue.name,
            .proxyPort = configValue.proxyPort,
            .services = configValue.services};

    if (!(*socket).
            SetConfigValues(targetEndpointConfig)
            ) {
        cout << "Failed to set " << proxyName << " Config values ..!" <<
             endl;
        return -2;
    }

    if (!(*socket).
            Start(proxyName)
            ) {
        cout << "Failed To Start " << proxyName << " Proxy Server ..!" <<
             endl;
        return -3;
    }

    return 0;
}

int startProxyServer(
        CProxySocket *socket,
        RESOLVED_PROXY_CONFIG configValue) {
    // Create proxies
    std::string proxyName = configValue.name;

    // Setting up ClickHouse Proxy
    PipelineFunction<CProxySocket> pipelineFunction = configValue.pipeline;
    if (!(*socket).SetPipeline(pipelineFunction)) {
        cout << "Failed to set " << proxyName << " Pipeline ..!" << endl;
        return -2;
    }
    CProxyHandler *proxyHandler = (CProxyHandler *) configValue.handler;
    if (!(*socket).SetHandler(proxyHandler)) {
        cout << "Failed to set " << proxyName << " Handler ..!" << endl;
        return -2;
    }

    TARGET_ENDPOINT_CONFIG targetEndpointConfig = {
            .name = configValue.name,
            .proxyPort = configValue.proxyPort,
            .services = configValue.services};

    if (!(*socket).SetConfigValues(targetEndpointConfig)) {
        cout << "Failed to set " << proxyName << " Config values ..!" << endl;
        return -2;
    }

    if (!(*socket).Start(proxyName)) {
        cout << "Failed To Start " << proxyName << " Proxy Server ..!" << endl;
        return -3;
    }

    return 0;
}

int startProtocolServer(
        CProtocolSocket *socket,
        RESOLVED_PROTOCOL_CONFIG configValue) {
    // Create protocol
    std::string protocolName = configValue.protocol_name;

    // Setting up Protocol Pipeline
    PipelineFunction<CProtocolSocket> pipelineFunction = configValue.pipeline;
    if (!(*socket).SetPipeline(pipelineFunction)) {
        cout << "Failed to set " << protocolName << " Pipeline ..!" << endl;
        return -2;
    }
    CProtocolHandler *protocolHandler = (CProtocolHandler *) configValue.handler;
    if (!(*socket).SetHandler(protocolHandler)) {
        cout << "Failed to set " << protocolName << " Handler ..!" << endl;
        return -2;
    }

    //    if (!(*socket).SetConfigValues(configValue))
    //    {
    //        cout << "Failed to set " << protocolName << " Config values ..!" << endl;
    //        return -2;
    //    }

    if (!(*socket).Start()) {
        cout << "Failed To Start " << protocolName << " Proxy Server ..!" << endl;
        return -3;
    }

    return 0;
}