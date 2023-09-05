#include "CProxySocket.h"
#include "ClientSocket.h"
#include "ProtocolHelper.h"
#include "../test/ProxyInfo.h"

void *CProxySocket::ThreadHandler(CProxySocket *ptr, void *lptr)
{

    CLIENT_DATA clientData;
    memcpy(&clientData, lptr, sizeof(CLIENT_DATA));
    char bfr[32000];
    int RetVal;

    END_POINT *ep = new END_POINT{"127.0.0.1", 9440, 1, "", 0,
                                  "  "}; // Resolve("firstcluster", "127.0.0.1" , 9000, pd );
    if (ep == 0)
    {
        return 0;
    }
    CProxyHandler *proxy_handler = ptr->GetHandler();
    if (proxy_handler == 0)
    {
        return 0;
    }
    cout << "Resolved Host: " << ep->ipaddress << ", Port: " << ep->port << endl;
    CClientSocket *client_sock = new CClientSocket((char *)(ep->ipaddress.c_str()), ep->port);
    if (client_sock == 0)
    {
        cout << "Failed to Create Client" << endl;
        return 0;
    }
    if (!client_sock->Resolve())
    {
        cout << "Failed to Resolve Client" << endl;
        delete client_sock;
        return 0;
    }
    if (!client_sock->Connect())
    {
        cout << "Failed To Connect " << endl;
        delete client_sock;
        return 0;
    }
    SOCKET s = client_sock->GetSocket();
    if (s == -1)
    {
        cout << "Invalid Socket" << endl;
        return 0;
    }

    Socket * client_socket;
    SocketClient target_socket(ep->ipaddress, ep->port);
    clientData.forward_port = s;
    ProtocolHelper::SetReadTimeOut(s, 1);
    ProtocolHelper::SetReadTimeOut(clientData.client_port, 1);
    int num_cycles = 0;
    cout << "Entered Nested Loop " << endl;
    while (1)
    {
        num_cycles++;
        while (1)
        {
            memset(bfr, 0, 32000);

            RetVal = recv(clientData.client_port, bfr, sizeof(bfr), 0);
            if (RetVal == -1)
            {
                // cout << "Socket Error...or...Socket Empty " << endl;
                break;
            }
            if (RetVal == 0)
            {
                num_cycles++;
                break;
            }
            // Call HandleUpStream(bfr,retVal, clientData);
#ifdef INLINE_LOGIC
            send(clientData.forward_port, bfr, RetVal, 0);
#else

            cout << "Inside Default handler.." << endl;
             
            if (!proxy_handler->HandleUpstreamData(bfr, RetVal, &target_socket))
            {

                return 0;
            }

#endif
            if (RetVal < 32000)
            {
                break;
            }
            RetVal = 0;
        }

        while (1)
        {
            memset(bfr, 0, 32000);
            RetVal = recv(clientData.forward_port, bfr, sizeof(bfr), 0);

            if (RetVal == -1)
            {
                break;
            }
            if (RetVal == 0)
            {
                num_cycles++;
                break;
            }
            // call HandleDownStream(bfr, RetVal, clientData);
#ifdef INLINE_LOGIC
            send(clientData.Sh, bfr, RetVal, 0);
#else
            cout << "Inside Default handler(Down ).." << endl;
            if (!proxy_handler->HandleDownStreamData(bfr, RetVal, client_socket))
            {
                return 0;
            }
#endif
            if (RetVal < 32000)
            {
                break;
            }
            RetVal = 0;
        }

        if (num_cycles > 15)
        {
            // cout <<"....................." << endl ;
            break;
        }
    }

    return 0;
}