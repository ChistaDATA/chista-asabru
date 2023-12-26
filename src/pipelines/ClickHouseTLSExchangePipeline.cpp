#include "CProtocolSocket.h"
#include "CProxySocket.h"
#include "CClientSSLSocket.h"
#include "Socket.h"
#include "SSLSocket.h"
#include "SocketSelect.h"
#include "CHttpParser.h"
#include <utility>

/**
 * HTTPS/TCP-TLS Exchange ( Forwarding SSL ) Pipeline :
 * Both the client and the target are connected via TLS. This pipeline
 * terminates the TLS connection from the client and then forwards the
 * decrypted packet to the target endpoint via separate TLS connection.
 */
void *ClickHouseTLSExchangePipeline(CProxySocket *ptr, void *lptr)
{
    std::cout << "ClickHouseTLSExchangePipeline::start" << std::endl;
    CLIENT_DATA clientData;
    memcpy(&clientData, lptr, sizeof(CLIENT_DATA));

    // Retrieve the load balancer class
    auto loadBalancer = ptr->loadBalancer;

    // Check if handler is defined
    CProxyHandler *proxy_handler = ptr->GetHandler();
    if (proxy_handler == nullptr)
    {
        cout << "The handler is not defined. Exiting!" << endl;
        return nullptr;
    }

    RESOLVED_SERVICE currentService = loadBalancer->requestServer();
    END_POINT target_endpoint = END_POINT{currentService.ipaddress, currentService.port, currentService.r_w, currentService.alias, currentService.reserved, "  "};

    cout << "Resolved (Target) Host: " << target_endpoint.ipaddress << endl
         << "Resolved (Target) Port: " << target_endpoint.port << endl;

    auto *client_socket= new SSLSocket(((Socket *)clientData.client_socket)->GetSocket());
    auto *target_socket = new CClientSSLSocket(target_endpoint.ipaddress, target_endpoint.port);

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
        }

        if (!still_connected)
        {
            // Close the client socket
            client_socket->Close();
            break;
        }
    }

    // Close the server socket
    target_socket->Close();
    std::cout << "ClickHouseTLSExchangePipeline::end" << std::endl;
#ifdef WINDOWS_OS
    return 0;
#else
    return nullptr;
#endif
}