
#include "CProtocolSocket.h"
#include "CProxySocket.h"
#include "CClientSocket.h"
#include "../config/ConfigSingleton.h"
#include "ProtocolHelper.h"
#include "Socket.h"
#include "SocketSelect.h"
#include "Pipeline.h"
#include <utility>
#include "CHttpParser.h"

void *PostgreSQLPipeline(CProxySocket *ptr, void *lptr)
{
    CLIENT_DATA clientData;
    memcpy(&clientData, lptr, sizeof(CLIENT_DATA));

    // Retrieve the load balancer class
    auto loadBalancer = ptr->loadBalancer;

    // Check if handler is defined
    CProxyHandler *proxy_handler = ptr->GetHandler();
    if (proxy_handler == nullptr)
    {
        std::cout << "The handler is not defined. Exiting!" << std::endl;
        return nullptr;
    }

    RESOLVED_SERVICE currentService = loadBalancer->requestServer();
    END_POINT target_endpoint = END_POINT{currentService.ipaddress, currentService.port, currentService.r_w, currentService.alias, currentService.reserved, "  "};
    std::cout << "Resolved (Target) Host: " << target_endpoint.ipaddress << std::endl
         << "Resolved (Target) Port: " << target_endpoint.port << std::endl;

    Socket *client_socket = (Socket *)clientData.client_socket;
    CClientSocket *target_socket = new CClientSocket(target_endpoint.ipaddress, target_endpoint.port);

    EXECUTION_CONTEXT exec_context;

    ProtocolHelper::SetReadTimeOut(client_socket->GetSocket(), 1);
    ProtocolHelper::SetKeepAlive(client_socket->GetSocket(), 1);
    ProtocolHelper::SetReadTimeOut(target_socket->GetSocket(), 1);
    ProtocolHelper::SetKeepAlive(target_socket->GetSocket(), 1);

    while (1)
    {
        try
        {
            SocketSelect sel(client_socket, target_socket, NonBlockingSocket);
            bool still_connected = true;

            if (sel.Readable(client_socket))
            {
                std::cout << "client socket is readable, reading bytes : " << std::endl;
                std::string bytes = client_socket->ReceiveBytes();

                std::cout << "Calling Proxy Upstream Handler.." << std::endl;
                std::string response = proxy_handler->HandleUpstreamData((void *)bytes.c_str(), bytes.size(), &exec_context);
                target_socket->SendBytes((char *)response.c_str(), response.size());
                // target_socket.SendBytes((char *) bytes.c_str(), bytes.size());

                if (bytes.empty())
                    still_connected = false;
            }
            if (sel.Readable(target_socket))
            {
                std::cout << "target socket is readable, reading bytes : " << std::endl;
                std::string bytes = target_socket->ReceiveBytes();

                std::cout << "Calling Proxy Downstream Handler.." << std::endl;
                std::string response = proxy_handler->HandleDownStreamData((void *)bytes.c_str(), bytes.size(), &exec_context);
                client_socket->SendBytes((char *)response.c_str(), response.size());

                if (bytes.empty())
                    still_connected = false;
            }
            if (!still_connected)
            {
                // Close the client socket
                client_socket->Close();
                break;
            }
        }
        catch (std::exception &e)
        {
            LOG_ERROR(e.what());
            break;
        }
    }

    // Close the server socket
    target_socket->Close();
#ifdef WINDOWS_OS
    return 0;
#else
    return nullptr;
#endif

    return 0;
}