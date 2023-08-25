#pragma  once

#include "CProtocolSocket.h"
#include "ServerSocket.h"
#include "../config/config_types.h"

pair<string, string> ChopLine(string str);

class CHttpsHandler : public CProxyHandler {
public:
    CHttpsHandler();
    virtual bool HandleUpstreamData(void *Buffer, int len, CLIENT_DATA &clientData);
    virtual bool HandleDownStreamData(void *Buffer, int len, CLIENT_DATA &clientData);

    void LogResponse(char * buffer, int len);
};
