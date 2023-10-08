
#include "CProtocolSocket.h"
#include "CProxySocket.h"
#include "../test/ProxyInfo.h"
#include "CClientSocket.h"
#include "../config/ConfigSingleton.h"
#include "ProtocolHelper.h"
#include "Socket.h"
#include "SocketSelect.h"
#include "Pipeline.h"
#include <utility>
#include "CHttpParser.h"

static ConfigSingleton &configSingleton = ConfigSingleton::getInstance();

void *PostgreSQLPipeline(CProxySocket *ptr, void *lptr)
{
    CLIENT_DATA clientData;
    memcpy(&clientData, lptr, sizeof(CLIENT_DATA));

    // Check if handler is defined
    CProxyHandler *proxy_handler = ptr->GetHandler();
    if (proxy_handler == 0)
    {
        cout << "The handler is not defined. Exiting!" << endl;
        return 0;
    }

    /**
     * Get the configuration data for the target database clusters ( eg. clickhouse )
     * Config given in config.xml
     */
    RESOLVE_ENDPOINT_RESULT result = ptr->GetConfigValues();
    END_POINT *target_endpoint = new END_POINT{result.ipaddress, result.port, result.r_w, result.alias, result.reserved, "  "}; // Resolve("firstcluster", "127.0.0.1" , 9000, pd );
    if (target_endpoint == 0)
    {
        cout << "Failed to retrieve target database configuration. Exiting!" << endl;
        return 0;
    }
    cout << "Resolved (Target) Host: " << target_endpoint->ipaddress << endl
         << "Resolved (Target) Port: " << target_endpoint->port << endl;

    Socket *client_socket = (Socket *)clientData.client_socket;
    CClientSocket *target_socket = new CClientSocket(target_endpoint->ipaddress, target_endpoint->port);

    EXECUTION_CONTEXT exec_context;

    ProtocolHelper::SetReadTimeOut(client_socket->GetSocket(), 1);
    ProtocolHelper::SetReadTimeOut(target_socket->GetSocket(), 1);

    while (1)
    {
        try
        {
            SocketSelect sel(client_socket, target_socket, NonBlockingSocket);
            bool still_connected = true;

            if (sel.Readable(client_socket))
            {
                cout << "client socket is readable, reading bytes : " << endl;
                std::string bytes = client_socket->ReceiveBytes();

                cout << "Calling Proxy Upstream Handler.." << endl;
                std::string response = proxy_handler->HandleUpstreamData((void *)bytes.c_str(), bytes.size(), &exec_context);
                target_socket->SendBytes((char *)response.c_str(), response.size());
                // target_socket.SendBytes((char *) bytes.c_str(), bytes.size());

                if (bytes.empty())
                    still_connected = false;
            }
            if (sel.Readable(target_socket))
            {
                cout << "target socket is readable, reading bytes : " << endl;
                std::string bytes = target_socket->ReceiveBytes();

                cout << "Calling Proxy Downstream Handler.." << endl;
                std::string response = proxy_handler->HandleDownStreamData((void *)bytes.c_str(), bytes.size(), &exec_context);
                client_socket->SendBytes((char *)response.c_str(), response.size());

                if (bytes.empty())
                    still_connected = false;
            }
            if (!still_connected)
            {
                break;
            }
        }
        catch (std::exception &e)
        {
            cout << e.what() << endl;
            break;
        }
    }
#ifdef WINDOWS_OS
    return 0;
#else
    return nullptr;
#endif

    return 0;
}