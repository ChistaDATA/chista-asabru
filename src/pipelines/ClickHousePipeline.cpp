
#include "interface/CProtocolSocket.h"
#include "interface/CProxySocket.h"
#include "../config/ConfigSingleton.h"
#include "socket/SocketSelect.h"
#include "Pipeline.h"
#include <utility>
#include "uuid/UuidGenerator.h"

void *ClickHousePipeline(CProxySocket *ptr, void *lptr)
{
    CLIENT_DATA clientData;
    memcpy(&clientData, lptr, sizeof(CLIENT_DATA));

    // Retrieve the load balancer class
    auto loadBalancer = ptr->loadBalancer;

    // Check if handler is defined
    CProxyHandler *proxy_handler = ptr->GetHandler();
    if (proxy_handler == nullptr)
    {
        LOG_ERROR("The handler is not defined. Exiting!");
        return nullptr;
    }

    RESOLVED_SERVICE currentService = loadBalancer->requestServer();
    END_POINT target_endpoint {
        currentService.ipaddress, currentService.port, currentService.r_w, currentService.alias, currentService.reserved, "  "};

    LOG_INFO("Resolved (Target) Host: " + target_endpoint.ipaddress);
    LOG_INFO("Resolved (Target) Port: " + std::to_string(target_endpoint.port));

    auto *client_socket = (Socket *)clientData.client_socket;
    CClientSocket *target_socket;
    try {
        target_socket = new CClientSocket(target_endpoint.ipaddress, target_endpoint.port);
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
        LOG_ERROR(std::string("Error connecting to target : ") + e.what());
        client_socket->Close();
        delete client_socket;
        return nullptr;
    }

    EXECUTION_CONTEXT exec_context;
    std::string correlation_id;
    bool data_sent = false;

    ProtocolHelper::SetReadTimeOut(client_socket->GetSocket(), 1);
    ProtocolHelper::SetKeepAlive(client_socket->GetSocket(), 1);
    ProtocolHelper::SetReadTimeOut(target_socket->GetSocket(), 1);
    ProtocolHelper::SetKeepAlive(target_socket->GetSocket(), 1);

    std::chrono::time_point start = std::chrono::high_resolution_clock::now();
    while (true)
    {
        SocketSelect *sel;
        try
        {
            sel = new SocketSelect(client_socket, target_socket, NonBlockingSocket);
        }
        catch (std::exception &e)
        {
            LOG_ERROR(e.what());
            LOG_ERROR("error occurred while creating socket select ");
        }

        bool still_connected = true;
        try
        {
            if (sel->Readable(client_socket))
            {
                LOG_INFO("client socket is readable, reading bytes : ");
                std::string bytes = client_socket->ReceiveBytes();

                if (!bytes.empty()) {
                    LOG_INFO("Calling Proxy Upstream Handler..");

                    if (!data_sent) {
                        correlation_id = UuidGenerator::generateUuid();
                        LOG_INFO("Correlation ID : " + correlation_id);
                        exec_context["correlation_id"] = correlation_id;
                    }

                    start = std::chrono::high_resolution_clock::now();
                    std::string response = proxy_handler->HandleUpstreamData(bytes, bytes.size(), &exec_context);
                    target_socket->SendBytes((char *)response.c_str(), response.size());
                    data_sent = true;
                }

                if (bytes.empty())
                    still_connected = false;
            }
        }
        catch (std::exception &e)
        {
            LOG_ERROR("Error while sending to target " + std::string(e.what()));
            still_connected = false;
        }

        try
        {
            if (sel->Readable(target_socket))
            {
                LOG_INFO("target socket is readable, reading bytes : ");
                std::string bytes = target_socket->ReceiveBytes();

                if (!bytes.empty()) {
                    auto stop = std::chrono::high_resolution_clock::now();
                    if (data_sent) {
                        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                        LOG_LATENCY(
                                correlation_id,
                                std::to_string(duration.count())+ "," +target_endpoint.ipaddress+":"+std::to_string(target_endpoint.port));
                        data_sent = false;
                    }
                    exec_context["request_stop_time"] = stop;
                    exec_context["target_host"] = target_endpoint.ipaddress+":"+std::to_string(target_endpoint.port);
                    LOG_INFO("Calling Proxy Downstream Handler..");
                    std::string response = proxy_handler->HandleDownStreamData(bytes, bytes.size(), &exec_context);
                    client_socket->SendBytes((char *) response.c_str(), response.size());
                }

                if (bytes.empty())
                    still_connected = false;
            }
        }
        catch (std::exception &e)
        {
            LOG_ERROR("Error while sending to client " + std::string(e.what()));
            still_connected = false;
        }

        if (!still_connected)
        {
            // Delete Select from memory
            delete sel;

            // Close the client socket
            LOG_INFO("Closing the client socket");
            client_socket->Close();
            delete client_socket;
            break;
        }
    }
    // Close the server socket
    LOG_INFO("Closing the target socket");
    target_socket->Close();
    delete target_socket;
#ifdef WINDOWS_OS
    return 0;
#else
    return nullptr;
#endif
}