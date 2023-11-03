#include "CProtocolSocket.h"
#include "CProxySocket.h"
#include "CClientSocket.h"
#include "Socket.h"
#include "SSLSocket.h"
#include "SocketSelect.h"

#include <utility>

/**
 * TLS Termination pipeline :
 * The proxy serves a TLS certificate and connection to the client is
 * via TLS. The TLS connection is terminated here and the decrypted
 * packets are transferred to the target endpoint via TCP/HTTP.
 */
void *ClickHouseTLSTerminatePipeline(CProxySocket *ptr, void *lptr)
{
    std::cout << "ClickHouseTLSTerminatePipeline::start" << std::endl;
    CLIENT_DATA clientData;
    memcpy(&clientData, lptr, sizeof(CLIENT_DATA));

    // Check if handler is defined
    CProxyHandler *proxy_handler = ptr->GetHandler();
    if (proxy_handler == nullptr)
    {
        cout << "The handler is not defined. Exiting!" << endl;
        return nullptr;
    }

    /**
     * Get the configuration data for the target database clusters ( eg. clickhouse )
     * Config given in config.xml
     */
    TARGET_ENDPOINT_CONFIG targetEndpointConfig = ptr->GetConfigValues();
    int services_count = targetEndpointConfig.services.size();
    int current_service_index = clientData.current_service_index % services_count;
    RESOLVED_SERVICE currentService = targetEndpointConfig.services[current_service_index];
    END_POINT *target_endpoint = new END_POINT{currentService.ipaddress, currentService.port, currentService.r_w, currentService.alias, currentService.reserved, "  "};
    if (target_endpoint == 0)
    {
        cout << "Failed to retrieve target database configuration. Exiting!" << endl;
        return 0;
    }
    cout << "Resolved (Target) Host: " << target_endpoint->ipaddress << endl
         << "Resolved (Target) Port: " << target_endpoint->port << endl;

    SSLSocket *client_socket= new SSLSocket(((Socket *)clientData.client_socket)->GetSocket());
    CClientSocket *target_socket = new CClientSocket(target_endpoint->ipaddress, target_endpoint->port);

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
    std::cout << "ClickHouseTLSTerminatePipeline::end" << std::endl;
#ifdef WINDOWS_OS
    return 0;
#else
    return nullptr;
#endif
}