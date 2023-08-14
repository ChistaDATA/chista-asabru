
#include "../handlers/CProtocolServer.h"
#include "../test/ProxyInfo.h"
#include "ClientSocket.h"
#include "../config/ConfigSingleton.h"
// #include <openssl/ssl.h>
// #include <openssl/err.h>

static ConfigSingleton &configSingleton = ConfigSingleton::getInstance();

void *ClickHousePipeline(CProxySocket *ptr, void *lptr)
{
    // Initialize OpenSSL library
    // SSL_library_init();
    // SSL_load_error_strings();
    // OpenSSL_add_all_algorithms();

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
    END_POINT *endpoint = new END_POINT{result.ipaddress, result.port, result.r_w, result.alias, result.reserved, "  "}; // Resolve("firstcluster", "127.0.0.1" , 9000, pd );
    if (endpoint == 0)
    {
        return 0;
    }
    cout << "Resolved (Target) Host: " << endpoint->ipaddress << endl
         << "Resolved (Target) Port: " << endpoint->port << endl;

    CClientSocket *client_socket = new CClientSocket((char *)(endpoint->ipaddress.c_str()), endpoint->port);

    if (client_socket == 0)
    {
        cout << "ClickHousePipeline -> Failed to Create Client" << endl;
        return 0;
    }

    if (!client_socket->Resolve())
    {
        cout << "ClickHousePipeline -> Failed to Resolve Client" << endl;
        delete client_socket;
        return 0;
    }

    if (!client_socket->Connect())
    {
        cout << "ClickHousePipeline -> Failed To Connect " << endl;
        delete client_socket;
        return 0;
    }

    SOCKET s = client_socket->GetSocket();
    if (s == -1)
    {
        cout << "Invalid Socket" << endl;
        return 0;
    }
    clientData.forward_port = s; // Target Port

    ProtocolHelper::SetReadTimeOut(clientData.forward_port, 1);
    ProtocolHelper::SetReadTimeOut(clientData.client_port, 1);

    int num_cycles = 0;
    cout << "Entered Nested Loop :" << endl;

    /**
     * Listen for incoming requests and outgoing requests
     */
    char buffer[32000];
    int returnValue;
    while (1)
    {
        num_cycles++;

        // Incoming requests
        while (1)
        {
            memset(buffer, 0, 32000);

            returnValue = recv(clientData.client_port, buffer, sizeof(buffer), 0);
            cout << "Received value in socket :" << returnValue << endl;
            if (returnValue == -1)
            {
                break;
            }
            if (returnValue == 0)
            {
                num_cycles++;
                break;
            }

#ifdef INLINE_LOGIC
            send(clientData.forward_port, buffer, returnValue, 0);
#else
            cout << "Calling Proxy Handler.." << endl;
            if (!proxy_handler->HandleUpstreamData(buffer, returnValue, clientData))
            {
                return 0;
            }
#endif
            if (returnValue < 32000)
            {
                break;
            }
            returnValue = 0;
        }

        // Outgoing requests
        while (1)
        {
            memset(buffer, 0, 32000);
            returnValue = recv(clientData.forward_port, buffer, sizeof(buffer), 0);

            if (returnValue == -1)
            {
                break;
            }
            if (returnValue == 0)
            {
                num_cycles++;
                break;
            }
            // call HandleDownStream(bfr, RetVal, clientData);
#ifdef INLINE_LOGIC
            send(clientData.Sh, bfr, RetVal, 0);
#else

            cout << "Calling Proxy Handler (Downstream).." << endl;
            if (!proxy_handler->HandleDownStreamData(buffer, returnValue, clientData))
            {
                return 0;
            }
#endif
            if (returnValue < 32000)
            {
                break;
            }
            returnValue = 0;
        }

        if (num_cycles > 15)
        {
            // cout <<"....................." << endl ;
            break;
        }
    }

    return 0;
}