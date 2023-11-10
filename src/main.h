#pragma once

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

int updateProxyConfig(CProxySocket *socket, RESOLVED_PROXY_CONFIG configValue);

std::string updateProxyServers();

std::string updateConfiguration(std::string content);

int initProtocolServers();

int initProxyServers();

void errorHandler(int sig);
