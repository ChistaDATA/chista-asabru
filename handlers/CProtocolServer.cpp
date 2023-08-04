////////////////////////////
// A Simple Protocol to test Protocol Sequencing
// https://github.com/eminfedar/async-sockets-cpp

#include "Utils.h"
#include "ClientSocket.h"
#include "../handlers/CProtocolServer.h"
#include "../test/ProxyInfo.h"

bool PingHandler::Handler(void *Buffer, int len, CLIENT_DATA &CData)
{

    cout << "Pinged from Remote Server ....................." << endl;
    return true;
}

void *CProtocolSocket::ThreadHandler(CProtocolSocket *ptr, void *lptr)
{

    CLIENT_DATA CData;
    memcpy(&CData, lptr, sizeof(CLIENT_DATA));
    CProtocolHandler *proto_handler = ptr->GetHandler();
    if (proto_handler == 0)
    {
        return 0;
    }

    char bfr[32000];
    while (1)
    {
        memset(bfr, 0, 32000);
        int num_read = 0;
        if (!ProtocolHelper::ReadSocketBuffer(CData.Sh, bfr, sizeof(bfr), &num_read))
        {
            return nullptr;
        }
        if (!(proto_handler->Handler(bfr, num_read, CData)))
        {
            break;
        }
    }
    return 0;
}

void *CProxySocket::ThreadHandler(CProxySocket *ptr, void *lptr)
{

    CLIENT_DATA CData;
    memcpy(&CData, lptr, sizeof(CLIENT_DATA));
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
    CData.forward_port = s;
    ProtocolHelper::SetReadTimeOut(s, 1);
    ProtocolHelper::SetReadTimeOut(CData.Sh, 1);
    int num_cycles = 0;
    cout << "Entered Nested Loop " << endl;
    while (1)
    {
        num_cycles++;
        while (1)
        {
            memset(bfr, 0, 32000);

            RetVal = recv(CData.Sh, bfr, sizeof(bfr), 0);
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
            // Call HandleUpStream(bfr,retVal, CData);
#ifdef INLINE_LOGIC
            send(CData.forward_port, bfr, RetVal, 0);
#else

            cout << "Inside Default handler.." << endl;
            if (!proxy_handler->HandleUpstreamData(bfr, RetVal, CData))
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
            RetVal = recv(CData.forward_port, bfr, sizeof(bfr), 0);

            if (RetVal == -1)
            {
                break;
            }
            if (RetVal == 0)
            {
                num_cycles++;
                break;
            }
            // call HandleDownStream(bfr, RetVal, CData);
#ifdef INLINE_LOGIC
            send(CData.Sh, bfr, RetVal, 0);
#else
            cout << "Inside Default handler(Down ).." << endl;
            if (!proxy_handler->HandleDownStreamData(bfr, RetVal, CData))
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

#if 0
int main(int argc, char* argv[])
{


    if ( argc != 2 ){
              fprintf(stdout,"Usage: Server <portid> \n");
              return 1;
    }
    if ( !StartSocket() )
    {
              fprintf(stdout,"Failed To Initialize Socket Library\n");
          return 1;
    }
                  int port = atoi(argv[1]);
        CServerSocket sock{ port};
                  std::function<void * (void *) > myfunc =  [] (void *ptr ) -> void *{
                                return ThreadHandler(ptr);
                     }  ;
                  if ( ! sock.Open("localhost:master 1:5000:READ",myfunc) )
                  {
                                      cout << "Could not Open Socket Properly" << endl;
                                      return 0;
                   }
    CServerSocket sock2{ port+1};
                  if (!sock2.Open("localhost:master 2:5001:WRITE", myfunc ))
                  {
                                      cout << "Could not Open Socket Properly" << endl;
                                      return 0;
                   }
                 while (1) ;
       Cleanup();
    return 0;
}

#endif
