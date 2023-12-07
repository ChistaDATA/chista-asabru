#pragma once
#include "BaseComputationCommand.h"
#include "CommandDispatcher.h"
#include "proxy_server.h"
#include "protocol-handlers/CHttpProtocolHandler.h"
#include "CProtocolSocket.h"
#include "config/ConfigSingleton.h"

std::string updateConfiguration(std::string content) {
    configSingleton.LoadConfigurationsFromString(std::move(content));
    std::string response_content = updateProxyServers();
    return response_content;
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

    if (!(*socket).Start()) {
        cout << "Failed To Start " << protocolName << " Proxy Server ..!" << endl;
        return -3;
    }

    return 0;
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