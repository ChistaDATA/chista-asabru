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

std::string updateEndPointService(std::string content) {
    ENDPOINT_SERVICE_CONFIG endpointServiceConfig = configSingleton.LoadEndpointServiceFromString(std::move(content));
    std::string response_content = updateProxyEndPointService(endpointServiceConfig);
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
        LOG_ERROR("Failed to set " + protocolName + " Pipeline ..!");
        return -2;
    }
    auto *protocolHandler = (CProtocolHandler *) configValue.handler;
    if (protocolHandler) {
        if (!(*socket).SetHandler(protocolHandler)) {
            LOG_ERROR("Failed to set " + protocolName + " Handler ..!");
            return -2;
        }
    }

    if (!(*socket).Start()) {
        LOG_ERROR("Failed To Start " + protocolName + " Proxy Server ..!");
        return -3;
    }

    return 0;
}

int initProtocolServers() {
    std::vector<RESOLVED_PROTOCOL_CONFIG> protocolServerConfigValues = configSingleton.ResolveProtocolServerConfigurations();
    for (const auto& value: protocolServerConfigValues) {
        auto protocolHandler = (CHttpProtocolHandler *) value.handler;
        if (protocolHandler) {
            for (const auto& route: value.routes) {
                protocolHandler->RegisterHttpRequestHandler(
                        route.path,
                        HttpMethodEnumMap.find(route.method)->second,
                        [route, value](const simple_http_server::HttpRequest &request) -> simple_http_server::HttpResponse {
                            LOG_INFO("Starting request handler lambda :");

                            ComputationContext context;
                            /**
                             * Set necessary shared variables and functions
                             */
                            context.Put("request", &request);
                            context.Put("update_configuration", updateConfiguration);
                            context.Put("update_endpoint_service",updateEndPointService);
                            context.Put("authentication", value.auth->strategy);

                            if (route.auth.required) {
                                LOG_INFO("Authenticating request :");
                                CommandDispatcher::Dispatch(value.auth->handler, &context);
                                if (!std::any_cast<bool>(context.Get("authenticated")))
                                {
                                    auto *response = new simple_http_server::HttpResponse(simple_http_server::HttpStatusCode::Unauthorized);
                                    response->SetHeader("Content-Type", "application/json");
                                    response->SetContent("Unauthorized");
                                    return *response;
                                }
                            }

                            CommandDispatcher::Dispatch(route.request_handler, &context);
                            auto response = std::any_cast<simple_http_server::HttpResponse *>(context.Get("response"));
                            return *response;
                        }
                );
            }
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