#include "../handlers/CProtocolSocket.h"
#include "../handlers/CProxySocket.h"
#include "../test/ProxyInfo.h"
#include "ClientSocket.h"
#include "../config/config_types.h"
#include "../config/ConfigSingleton.h"
#include "ProtocolHelper.h"

static ConfigSingleton &configSingleton = ConfigSingleton::getInstance();

void *MySQLPipeline(CProxySocket *ptr, void *lptr) {

    CLIENT_DATA clientData;
    memcpy(&clientData, lptr, sizeof(CLIENT_DATA));
    char bfr[32000];
    int RetVal;

    RESOLVE_ENDPOINT_RESULT result = ptr->GetConfigValues();

    END_POINT *ep = new END_POINT{result.ipaddress, result.port, result.r_w, result.alias,
                                  result.reserved, "  "};

    if (ep == 0) {
        return 0;
    }
    CProxyHandler *proxy_handler = ptr->GetHandler();
    if (proxy_handler == 0) {
        return 0;
    }
    cout << "Resolved " << ep->ipaddress << "   " << ep->port << endl;
    CClientSocket *client_sock = new CClientSocket((char *) (ep->ipaddress.c_str()), ep->port);
    if (client_sock == 0) {
        cout << "Failed to Create Client" << endl;
        return 0;
    }
    if (!client_sock->Resolve()) {
        cout << "Failed to Resolve Client" << endl;
        delete client_sock;
        return 0;
    }
    if (!client_sock->Connect()) {
        cout << "Failed To Connect " << endl;
        delete client_sock;
        return 0;
    }
    SOCKET s = client_sock->GetSocket();
    if (s == -1) {
        cout << "Invalid Socket" << endl;
        return 0;
    }
    clientData.forward_port = s;
    ProtocolHelper::SetReadTimeOut(s, 1);
    ProtocolHelper::SetReadTimeOut(clientData.client_port, 1);
    int num_cycles = 0;
    cout << "Entered Nested Loop " << endl;
    while (1) {
        num_cycles++;
        while (1) {
            memset(bfr, 0, 32000);

            RetVal = recv(clientData.client_port, bfr, sizeof(bfr), 0);
            if (RetVal == -1) {
                // cout << "Socket Error...or...Socket Empty " << endl;
                break;
            }
            if (RetVal == 0) {
                num_cycles++;
                break;
            }
            // Call HandleUpStream(bfr,retVal, clientData);
#ifdef INLINE_LOGIC
            send(clientData.forward_port, bfr, RetVal, 0);
#else
            cout << "Calling Proxy Handler.." << endl;
            if (!proxy_handler->HandleUpstreamData(bfr, RetVal, clientData)) {

                return 0;
            }

#endif
            if (RetVal < 32000) {
                break;
            }
            RetVal = 0;
        }

        while (1) {
            memset(bfr, 0, 32000);
            RetVal = recv(clientData.forward_port, bfr, sizeof(bfr), 0);

            if (RetVal == -1) {
                break;
            }
            if (RetVal == 0) {
                num_cycles++;
                break;
            }
            // call HandleDownStream(bfr, RetVal, clientData);
#ifdef INLINE_LOGIC
            send(clientData.Sh, bfr, RetVal, 0);
#else

            cout << "Calling Inline Handler (Downstream).." << endl;
            if (!proxy_handler->HandleDownStreamData(bfr, RetVal, clientData)) {
                return 0;
            }
#endif
            if (RetVal < 32000) {
                break;
            }
            RetVal = 0;
        }

        if (num_cycles > 15) {
            // cout <<"....................." << endl ;
            break;
        }
    }

    return 0;
}
