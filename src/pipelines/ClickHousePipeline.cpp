
#include "CProtocolSocket.h"
#include "CProxySocket.h"
#include "CClientSocket.h"
#include "../config/ConfigSingleton.h"
#include "Socket.h"
#include "SocketSelect.h"
#include "Pipeline.h"
#include "CHttpParser.h"
#include "Logger.h"
#include <utility>

void *ClickHousePipeline(CProxySocket *ptr, void *lptr)
{
    Logger *logger = Logger::getInstance();
    CLIENT_DATA clientData;
    memcpy(&clientData, lptr, sizeof(CLIENT_DATA));

    // Retrieve the load balancer class
    auto loadBalancer = ptr->loadBalancer;

    // Check if handler is defined
    CProxyHandler *proxy_handler = ptr->GetHandler();
    if (proxy_handler == nullptr)
    {
        logger->Log("ClickHousePipeline", "ERROR", "The handler is not defined. Exiting!");
        return nullptr;
    }

    RESOLVED_SERVICE currentService = loadBalancer->requestServer();
    END_POINT target_endpoint {
        currentService.ipaddress, currentService.port, currentService.r_w, currentService.alias, currentService.reserved, "  "};

    logger->Log("ClickHousePipeline", "INFO", "Resolved (Target) Host: " + target_endpoint.ipaddress);
    logger->Log("ClickHousePipeline", "INFO", "Resolved (Target) Port: " + std::to_string(target_endpoint.port));

    auto *client_socket = (Socket *)clientData.client_socket;
    CClientSocket *target_socket;
    try {
        target_socket = new CClientSocket(target_endpoint.ipaddress, target_endpoint.port);
    } catch (std::exception &e) {
        cout << e.what() << endl;
        logger->Log("ClickHousePipeline", "ERROR", e.what());
        client_socket->Close();
        delete client_socket;
        return nullptr;
    }

    EXECUTION_CONTEXT exec_context;

    ProtocolHelper::SetReadTimeOut(client_socket->GetSocket(), 1);
    ProtocolHelper::SetKeepAlive(client_socket->GetSocket(), 1);
    ProtocolHelper::SetReadTimeOut(target_socket->GetSocket(), 1);
    ProtocolHelper::SetKeepAlive(target_socket->GetSocket(), 1);

    while (true)
    {
        SocketSelect *sel;
        try
        {
            sel = new SocketSelect(client_socket, target_socket, NonBlockingSocket);
        }
        catch (std::exception &e)
        {
            cout << e.what() << endl;
            cout << "error occurred while creating socket select " << endl;
        }

        bool still_connected = true;
        try
        {
            if (sel->Readable(client_socket))
            {
                cout << "client socket is readable, reading bytes : " << endl;
                std::string bytes = client_socket->ReceiveBytes();

                cout << "Calling Proxy Upstream Handler.." << endl;
                std::string response = proxy_handler->HandleUpstreamData((void *)bytes.c_str(), bytes.size(), &exec_context);
                target_socket->SendBytes((char *)response.c_str(), response.size());

                if (bytes.empty())
                    still_connected = false;
            }
        }
        catch (std::exception &e)
        {
            cout << "Error while sending to target " << e.what() << endl;
            still_connected = false;
        }

        try
        {
            if (sel->Readable(target_socket))
            {
                cout << "target socket is readable, reading bytes : " << endl;
                std::string bytes = target_socket->ReceiveBytes();

                cout << "Calling Proxy Downstream Handler.." << endl;
                std::string response = proxy_handler->HandleDownStreamData((void *)bytes.c_str(), bytes.size(), &exec_context);
                client_socket->SendBytes((char *)response.c_str(), response.size());

                if (bytes.empty())
                    still_connected = false;
            }
        }
        catch (std::exception &e)
        {
            cout << "Error while sending to client " << e.what() << endl;
            still_connected = false;
        }

        if (!still_connected)
        {
            // Delete Select from memory
            delete sel;

            // Close the client socket
            client_socket->Close();
            delete client_socket;
            break;
        }
    }

    // Close the server socket
    target_socket->Close();
    delete target_socket;
#ifdef WINDOWS_OS
    return 0;
#else
    return nullptr;
#endif
}