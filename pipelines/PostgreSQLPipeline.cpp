#include "../handlers/CProtocolSocket.h"
#include "../handlers/CProxySocket.h"
#include "../test/ProxyInfo.h"
#include "ClientSocket.h"
#include "../config/ConfigSingleton.h"
#include "ProtocolHelper.h"

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
        return 0;
    }
    cout << "Resolved (Target) Host: " << target_endpoint->ipaddress << endl
         << "Resolved (Target) Port: " << target_endpoint->port << endl;

    Socket *client_socket = (Socket *)clientData.client_socket;
    SocketClient target_socket(target_endpoint->ipaddress, target_endpoint->port);

    while (1)
    {
        SocketSelect sel(client_socket, &target_socket, NonBlockingSocket);

        bool still_connected = true;

        if (sel.Readable(client_socket))
        {
            std::string bytes = client_socket->ReceiveBytes();

            // calling handler

            cout << "Calling Proxy Handler.." << endl;
            proxy_handler->HandleUpstreamData((void *)bytes.c_str(), (int)bytes.size(), &target_socket);
            // std::cout << "Server: " << bytes << std::endl;

            if (bytes.empty())
                still_connected = false;
        }
        if (sel.Readable(&target_socket))
        {
            std::string bytes = target_socket.ReceiveBytes();
            client_socket->SendBytes((char *) bytes.c_str(), bytes.size());
            // std::cout << "Client: " << bytes << std::endl;
            if (bytes.empty())
                still_connected = false;
        }
        if (!still_connected)
        {
            break;
        }
    }

    delete client_socket;
#ifdef WINDOWS_OS
    return 0;
#else
    return nullptr;
#endif
    //     CLIENT_DATA clientData;
    //     memcpy(&clientData, lptr, sizeof(CLIENT_DATA));
    //     char bfr[32000];
    //     int RetVal;

    //     RESOLVE_ENDPOINT_RESULT result = ptr->GetConfigValues();

    //     END_POINT *ep = new END_POINT{result.ipaddress, result.port, result.r_w, result.alias,
    //                                   result.reserved, "  "};

    //     if (ep == 0) {
    //         return 0;
    //     }
    //     CProxyHandler *proxy_handler = ptr->GetHandler();
    //     if (proxy_handler == 0) {
    //         return 0;
    //     }
    //     cout << "Resolved " << ep->ipaddress << "   " << ep->port << endl;
    //     CClientSocket *client_sock = new CClientSocket((char *) (ep->ipaddress.c_str()), ep->port);
    //     if (client_sock == 0) {
    //         cout << "Failed to Create Client" << endl;
    //         return 0;
    //     }
    //     if (!client_sock->Resolve()) {
    //         cout << "Failed to Resolve Client" << endl;
    //         delete client_sock;
    //         return 0;
    //     }
    //     if (!client_sock->Connect()) {
    //         cout << "Failed To Connect " << endl;
    //         delete client_sock;
    //         return 0;
    //     }
    //     SOCKET s = client_sock->GetSocket();
    //     if (s == -1) {
    //         cout << "Invalid Socket" << endl;
    //         return 0;
    //     }
    //     clientData.forward_port = s;
    //     ProtocolHelper::SetReadTimeOut(s, 1);
    //     ProtocolHelper::SetReadTimeOut(clientData.client_port, 1);
    //     int num_cycles = 0;
    //     cout << "Entered Nested Loop " << endl;
    //     while (1) {
    //         num_cycles++;
    //         while (1) {
    //             memset(bfr, 0, 32000);

    //             RetVal = recv(clientData.client_port, bfr, sizeof(bfr), 0);
    //             if (RetVal == -1) {
    //                 // cout << "Socket Error...or...Socket Empty " << endl;
    //                 break;
    //             }
    //             if (RetVal == 0) {
    //                 num_cycles++;
    //                 break;
    //             }
    //             // Call HandleUpStream(bfr,retVal, clientData);
    // #ifdef INLINE_LOGIC
    //             send(clientData.forward_port, bfr, RetVal, 0);
    // #else
    //             cout << "Calling Proxy Handler.." << endl;
    //             if (!proxy_handler->HandleUpstreamData(bfr, RetVal, clientData)) {

    //                 return 0;
    //             }

    // #endif
    //             if (RetVal < 32000) {
    //                 break;
    //             }
    //             RetVal = 0;
    //         }

    //         while (1) {
    //             memset(bfr, 0, 32000);
    //             RetVal = recv(clientData.forward_port, bfr, sizeof(bfr), 0);

    //             if (RetVal == -1) {
    //                 break;
    //             }
    //             if (RetVal == 0) {
    //                 num_cycles++;
    //                 break;
    //             }
    //             // call HandleDownStream(bfr, RetVal, clientData);
    // #ifdef INLINE_LOGIC
    //             send(clientData.Sh, bfr, RetVal, 0);
    // #else

    //             cout << "Calling Inline Handler (Downstream).." << endl;
    //             if (!proxy_handler->HandleDownStreamData(bfr, RetVal, clientData)) {
    //                 return 0;
    //             }
    // #endif
    //             if (RetVal < 32000) {
    //                 break;
    //             }
    //             RetVal = 0;
    //         }

    //         if (num_cycles > 15) {
    //             // cout <<"....................." << endl ;
    //             break;
    //         }
    //     }

    //     return 0;
}