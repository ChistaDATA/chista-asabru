#pragma  once

#include "CProtocolServer.h"
#include "../engine/socket/ServerSocket.h"
#include "../config/config_types.h"

pair<string, string> ChopLine(string str);

class CHttpsHandler : public CProxyHandler {
public:
    CHttpsHandler();
    virtual bool HandleUpstreamData(void *Buffer, int len, CLIENT_DATA &CData);
    virtual bool HandleDownStreamData(void *Buffer, int len, CLIENT_DATA &CData);

    void LogResponse(char * buffer, int len);
};
