#pragma  once

#include "CProtocolServer.h"
#include "../parsers/CHttpParser.h"

pair<string, string> ChopLine(string str);

class CHttpHandler : public CProxyHandler {
    CHttpParser *parser = 0;
public:
    CHttpHandler(CHttpParser *parser);
    virtual bool HandleUpstreamData(void *Buffer, int len, CLIENT_DATA &CData);
    virtual bool HandleDownStreamData(void *Buffer, int len, CLIENT_DATA &CData);

    void LogResponse(char * buffer, int len);
};