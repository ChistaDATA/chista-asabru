#pragma once
#include <string>
#include <iostream>
#include "config/ConfigSingleton.h"
#include "CProxySocket.h"
#include "LibuvServerSocket.h"

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
        std::cout << e.what() << std::endl;
        return "Error occurred when updating the configurations : " + std::string(e.what());
    }
    return "Configuration Updated Successfully!\n";
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
        std::cout << "Failed to set " << proxyName << " Pipeline ..!" <<
                                                                      std::endl;
        return -2;
    }
    CProxyHandler *proxyHandler = (CProxyHandler *) configValue.handler;
    if (!(*socket).
            SetHandler(proxyHandler)
            ) {
        std::cout << "Failed to set " << proxyName << " Handler ..!" <<
                                                                     std::endl;
        return -2;
    }

    TARGET_ENDPOINT_CONFIG targetEndpointConfig = {
            .name = configValue.name,
            .proxyPort = configValue.proxyPort,
            .services = configValue.services};

    if (!(*socket).
            SetConfigValues(targetEndpointConfig)
            ) {
        std::cout << "Failed to set " << proxyName << " Config values ..!" <<
                                                                           std::endl;
        return -2;
    }

    if (!(*socket).
            Start(proxyName)
            ) {
        std::cout << "Failed To Start " << proxyName << " Proxy Server ..!" <<
                                                                            std::endl;
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
        std::cout << "Failed to set " << proxyName << " Pipeline ..!" << std::endl;
        return -2;
    }
    CProxyHandler *proxyHandler = (CProxyHandler *) configValue.handler;
    if (!(*socket).SetHandler(proxyHandler)) {
        std::cout << "Failed to set " << proxyName << " Handler ..!" << std::endl;
        return -2;
    }

    TARGET_ENDPOINT_CONFIG targetEndpointConfig = {
            .name = configValue.name,
            .proxyPort = configValue.proxyPort,
            .services = configValue.services};

    if (!(*socket).SetConfigValues(targetEndpointConfig)) {
        std::cout << "Failed to set " << proxyName << " Config values ..!" << std::endl;
        return -2;
    }

    if (!(*socket).Start(proxyName)) {
        std::cout << "Failed To Start " << proxyName << " Proxy Server ..!" << std::endl;
        return -3;
    }

    return 0;
}

int initProxyServers() {
    std::vector<RESOLVED_PROXY_CONFIG> configValues = configSingleton.ResolveProxyServerConfigurations();
    PipelineFactory pipelineFactory;
    for (const auto& value: configValues) {
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