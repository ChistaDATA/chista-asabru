#include "CProtocolSocket.h"
#include "CProxySocket.h"
#include "CClientSSLSocket.h"
#include "SocketSelect.h"
#include "CHttpParser.h"
#include "Logger.h"
#include <utility>
#include "uuid/UuidGenerator.h"

/**
 * HTTPS/TCP-TLS Exchange ( Forwarding SSL ) Pipeline :
 * Both the client and the target are connected via TLS. This pipeline
 * terminates the TLS connection from the client and then forwards the
 * decrypted packet to the target endpoint via separate TLS connection.
 */
void *ClickHouseTLSExchangePipeline(CProxySocket *ptr, void *lptr) {
    std::string correlation_id = UuidGenerator::generateUuid();
    LOG_INFO("Correlation ID : " + correlation_id);
    LOG_INFO("ClickHouseTLSExchangePipeline::start");
    CLIENT_DATA clientData;
    memcpy(&clientData, lptr, sizeof(CLIENT_DATA));

    // Retrieve the load balancer class
    auto loadBalancer = ptr->loadBalancer;

    // Check if handler is defined
    CProxyHandler *proxy_handler = ptr->GetHandler();
    if (proxy_handler == nullptr) {
        LOG_ERROR("The handler is not defined. Exiting!");
        return nullptr;
    }

    RESOLVED_SERVICE currentService = loadBalancer->requestServer();
    END_POINT target_endpoint = END_POINT{currentService.ipaddress, currentService.port, currentService.r_w,
                                          currentService.alias, currentService.reserved, "  "};

    LOG_INFO("Resolved (Target) Host: " + std::string(target_endpoint.ipaddress));
    LOG_INFO("Resolved (Target) Port: " + std::to_string(target_endpoint.port));

    auto *client_socket = new SSLSocket(((Socket *) clientData.client_socket)->GetSocket());
    auto *target_socket = new CClientSSLSocket(target_endpoint.ipaddress, target_endpoint.port);

    EXECUTION_CONTEXT exec_context;
    exec_context["correlation_id"] = (void *) correlation_id.c_str();

    ProtocolHelper::SetReadTimeOut(client_socket->GetSocket(), 1);
    ProtocolHelper::SetKeepAlive(client_socket->GetSocket(), 1);
    ProtocolHelper::SetReadTimeOut(target_socket->GetSocket(), 1);
    ProtocolHelper::SetKeepAlive(target_socket->GetSocket(), 1);

    auto start = std::chrono::high_resolution_clock::now();
    while (true) {
        SocketSelect *sel;

        try {
            sel = new SocketSelect(client_socket, target_socket, NonBlockingSocket);
        }
        catch (std::exception &e) {
            LOG_ERROR(e.what());
            LOG_ERROR("error occurred while creating socket select ");
        }

        bool still_connected = true;
        try {
            if (sel->Readable(client_socket)) {
                LOG_INFO("client socket is readable, reading bytes : ");
                std::string bytes = client_socket->ReceiveBytes();
                if (!bytes.empty()) {
                    LOG_INFO("Calling Proxy Upstream Handler..");
                    std::string response = proxy_handler->HandleUpstreamData(bytes, bytes.size(),
                                                                             &exec_context);
                    target_socket->SendBytes((char *) response.c_str(), response.size());
                }

                if (bytes.empty())
                    still_connected = false;
            }
        }
        catch (std::exception &e) {
            LOG_ERROR("Error while receiving data from client " + std::string(e.what()));
        }

        try {
            if (sel->Readable(target_socket)) {
                LOG_INFO("target socket is readable, reading bytes : ");
                std::string bytes = target_socket->ReceiveBytes();

                if (!bytes.empty()) {
                    LOG_INFO("Calling Proxy Downstream Handler..");
                    std::string response = proxy_handler->HandleDownStreamData(bytes, bytes.size(),
                                                                               &exec_context);
                    client_socket->SendBytes((char *) response.c_str(), response.size());
                }

                if (bytes.empty())
                    still_connected = false;
            }
        }
        catch (std::exception &e) {
            LOG_ERROR("Error while sending to target " + std::string(e.what()));
        }

        if (!still_connected) {
            // Close the client socket
            client_socket->Close();
            break;
        }
    }

    // Close the server socket
    target_socket->Close();
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    LOG_LATENCY(correlation_id, std::to_string(duration.count()));
    LOG_INFO("ClickHouseTLSExchangePipeline::end");
#ifdef WINDOWS_OS
    return 0;
#else
    return nullptr;
#endif
}