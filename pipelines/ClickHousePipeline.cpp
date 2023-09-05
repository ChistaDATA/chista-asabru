
#include "../handlers/CProtocolSocket.h"
#include "../handlers/CProxySocket.h"
#include "../test/ProxyInfo.h"
#include "ClientSocket.h"
#include "../config/ConfigSingleton.h"
#include "ProtocolHelper.h"
#include "Socket.h"
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

    Socket *client_socket = (Socket *)clientData.client_socket;
    SocketClient target_socket(target_endpoint->ipaddress, target_endpoint->port);

    ProtocolHelper::SetReadTimeOut(client_socket->GetSocket(), 1);
    ProtocolHelper::SetReadTimeOut(target_socket.GetSocket(), 1);

    while (1)
    {
        try
        {
            SocketSelect sel(client_socket, &target_socket, NonBlockingSocket);
            bool still_connected = true;

            if (sel.Readable(client_socket))
            {
                std::string bytes = client_socket->ReceiveBytes();

                cout << "Calling Proxy Upstream Handler.." << endl;
                proxy_handler->HandleUpstreamData((void *)bytes.c_str(), bytes.size(), &target_socket);

                if (bytes.empty())
                    still_connected = false;
            }
            if (sel.Readable(&target_socket))
            {
                cout << "target socket is readable, reading bytes : " << endl;
                std::string bytes = target_socket.ReceiveBytes();

                cout << "Calling Proxy Downstream Handler.." << endl;
                proxy_handler->HandleDownStreamData((void *)bytes.c_str(), bytes.size(), client_socket);

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

    // int num_cycles = 0;
    // char buffer[32000];
    // int RetVal;
    // while (1)
    // {
    //     num_cycles++;
    //     while (1)
    //     {
    //         memset(buffer, 0, 32000);

    //         // RetVal = recv(client_socket->GetSocket(), buffer, sizeof(buffer), 0);
    //         char buf[32000];
    //         int RetVal = client_socket->ReceiveBytesTemp(buf);
    //         if (RetVal < 0)
    //         {
    //             // cout << "Socket Error...or...Socket Empty " << endl;
    //             break;
    //         }
    //         if (RetVal == 0)
    //         {
    //             num_cycles++;
    //             break;
    //         }

    //         send(target_socket.GetSocket(), buf, RetVal, 0);

    //         if (RetVal < 32000)
    //         {
    //             break;
    //         }
    //         RetVal = 0;
    //     }

    //     while (1)
    //     {
    //         memset(buffer, 0, 32000);
    //         RetVal = recv(target_socket.GetSocket(), buffer, sizeof(buffer), 0);

    //         if (RetVal == -1)
    //         {
    //             break;
    //         }
    //         if (RetVal == 0)
    //         {
    //             num_cycles++;
    //             break;
    //         }
    //         // call HandleDownStream(bfr, RetVal, CData);

    //         send(client_socket->GetSocket(), buffer, RetVal, 0);

    //         if (RetVal < 32000)
    //         {
    //             break;
    //         }
    //         RetVal = 0;
    //     }

    //     if (num_cycles > 15)
    //     {
    //         // cout <<"....................." << endl ;
    //         break;
    //     }
    // }

    delete client_socket;
#ifdef WINDOWS_OS
    return 0;
#else
    return nullptr;
#endif

    return 0;
}