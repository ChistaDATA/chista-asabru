#pragma  once

#include "CProtocolSocket.h"
#include "ServerSocket.h"
#include "../config/config_types.h"

pair<string, string> ChopLine(string str);

class CHttpsHandler : public CProxyHandler {
public:
    CHttpsHandler();
    virtual void * HandleUpstreamData(void * buffer, int buffer_length, uv_stream_t * target);
    virtual void * HandleDownStreamData(void * buffer, int buffer_length, uv_stream_t *client);

    void LogResponse(char * buffer, int len);
};
