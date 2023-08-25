#include "../handlers/CProtocolSocket.h"
#include "../handlers/CProxySocket.h"
#include "../test/ProxyInfo.h"
#include "ProtocolHelper.h"

// Basic Passthrough pipeline
void *PassthroughPipeLine(CProtocolSocket *ptr, void *lptr) {

    CLIENT_DATA clientData;
    memcpy(&clientData, lptr, sizeof(CLIENT_DATA));
    CProtocolHandler *proto_handler = ptr->GetHandler();
    if (proto_handler == 0) {
        return 0;
    }

    char bfr[32000];
    while (1) {
        memset(bfr, 0, 32000);
        int num_read = 0;
        if (!ProtocolHelper::ReadSocketBuffer(clientData.client_port, bfr, sizeof(bfr), &num_read)) {
            return nullptr;
        }
        if (!(proto_handler->Handler(bfr, num_read, clientData))) {
            break;
        }
    }
    return 0;
}