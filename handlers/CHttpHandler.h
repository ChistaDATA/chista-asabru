#pragma once

#include "CProtocolSocket.h"
#include "CHttpParser.h"

pair<string, string> ChopLine(string str);

class CHttpHandler : public CProxyHandler {
    CHttpParser *parser = 0;
public:
    CHttpHandler(CHttpParser *parser);
    virtual void * HandleUpstreamData(void * buffer, int buffer_length, uv_stream_t *target);
    virtual void * HandleDownStreamData(void * buffer, int buffer_length, uv_stream_t *client);

    void LogResponse(char * buffer, int len);
};