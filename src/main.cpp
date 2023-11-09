// #include <execinfo.h>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <list>
#include <map>
#include <iostream>
#include <string>
#include <utility>
/*for O_RDONLY*/
#include <fcntl.h>

#include "config/ConfigSingleton.h"
#include "CProtocolSocket.h"
#include "CProxySocket.h"
#include "LibuvServerSocket.h"
#include "protocol-handlers/CHttpProtocolHandler.h"
#include "BaseComputationCommand.h"
#include "CommandDispatcher.h"

using namespace std;

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

std::string updateProxyServers() {
    std::vector<RESOLVED_PROXY_CONFIG> configValues = configSingleton.ResolveProxyServerConfigurations();
    try {
        for (auto value: configValues) {
            updateProxyConfig(configSingleton.proxySocketsMap[value.name], value);
        }
    }
    catch (std::exception &e) {
        cout << e.what() << endl;
        return "Error occurred when updating the configurations : " + std::string(e.what());
    }
    return "Configuration Updated Successfully!\n";
}

std::string updateConfiguration(std::string content) {
    configSingleton.LoadConfigurationsFromString(std::move(content));
    std::string response_content = updateProxyServers();
    return response_content;
}

int initProtocolServers() {
    std::vector<RESOLVED_PROTOCOL_CONFIG> protocolServerConfigValues = configSingleton.ResolveProtocolServerConfigurations();
    for (const auto& value: protocolServerConfigValues) {
        auto httpProtocolHandler = (CHttpProtocolHandler *) value.handler;
        for (const auto& route: value.routes) {
            httpProtocolHandler->RegisterHttpRequestHandler(
                    route.path,
                    HttpMethodEnumMap.find(route.method)->second,
                    [route](const simple_http_server::HttpRequest &request) -> simple_http_server::HttpResponse {
                        std::cout << "Starting request handler lambda :" << std::endl;

                        ComputationContext context;
                        /**
                         * Set necessary shared variables and functions
                         */
                        context.Put("request", &request);
                        context.Put("update_configuration", updateConfiguration);

                        CommandDispatcher::Dispatch(route.request_handler, &context);
                        auto response = std::any_cast<simple_http_server::HttpResponse *>(context.Get("response"));
                        std::cout << (int) (response->status_code()) << std::endl;
                        return *response;
                    }
            );
        }
        configSingleton.protocolSocketsMap[value.protocol_name] = new CProtocolSocket(value.protocol_port);
        int protocolServer = startProtocolServer(
                configSingleton.protocolSocketsMap[value.protocol_name],
                value);
        if (protocolServer < 0)
            return protocolServer;
    }

    return 1;
}

int initProxyServers() {
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
            configSingleton.proxySocketsMap[value.name] = new CProxySocket(value.proxyPort);
            int proxy = startProxyServer(
                    configSingleton.proxySocketsMap[value.name],
                    value);
            if (proxy < 0)
                return proxy;
        }


    }
    return 0;
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
    /* ignore SIGPIPE so that server can continue running when client pipe closes abruptly */
    signal(SIGPIPE, SIG_IGN);

    int returnValue = initProxyServers();
    if (returnValue < 0) {
        cout << "Error occurred during initializing proxy servers!";
        exit(1);
    }

    returnValue = initProtocolServers();
    if (returnValue < 0) {
        cout << "Error occurred during initializing protocol servers!";
        exit(1);
    }

    while (true);
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
