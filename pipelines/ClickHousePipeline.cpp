
#include "../handlers/CProtocolSocket.h"
#include "../handlers/CProxySocket.h"
#include "../test/ProxyInfo.h"
#include "ClientSocket.h"
#include "../config/ConfigSingleton.h"
#include "ProtocolHelper.h"
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
    END_POINT *target_endpoint = new END_POINT{result.ipaddress, result.port, result.r_w, result.alias, result.reserved, "  "}; // Resolve("firstcluster", "127.0.0.1" , 9000, pd );
    if (target_endpoint == 0)
    {
        return 0;
    }
    cout << "Resolved (Target) Host: " << target_endpoint->ipaddress << endl
         << "Resolved (Target) Port: " << target_endpoint->port << endl;

    CClientSocket *target_socket = new CClientSocket((char *)(target_endpoint->ipaddress.c_str()), target_endpoint->port);

    if (target_socket == 0)
    {
        cout << "ClickHousePipeline -> Failed to create target connection" << endl;
        return 0;
    }

    if (!target_socket->Resolve())
    {
        cout << "ClickHousePipeline -> Failed to resolve target" << endl;
        delete target_socket;
        return 0;
    }

    if (!target_socket->Connect())
    {
        cout << "ClickHousePipeline -> Failed To connect to target socket " << endl;
        delete target_socket;
        return 0;
    }

    if (target_socket->GetSocket() == -1)
    {
        cout << "Invalid target socket" << endl;
        delete target_socket;
        return 0;
    }
    clientData.forward_port = target_socket->GetSocket(); // Target Port

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
        cout << "The handler is not defined. Exiting!" << endl;
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