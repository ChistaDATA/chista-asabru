#pragma  once

#include "CProtocolServer.h"

const string CRLF = "\r\n";
pair<string, string> ChopLine(string str);

class CHttpHandler : public CProxyHandler {

public:
    CHttpHandler();
    virtual bool HandleUpstreamData(void *Buffer, int len, CLIENT_DATA &CData);
    virtual bool HandleDownStreamData(void *Buffer, int len, CLIENT_DATA &CData);

    void LogResponse(char * buffer, int len);
};