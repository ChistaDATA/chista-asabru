
#include <map>
#include "CHttpHandler.h"
#include "../common/LineGrabber.h"



CHttpHandler::CHttpHandler() {}

bool CHttpHandler::HandleUpstreamData(void *Buffer, int len, CLIENT_DATA &CData) {

    // Log the Content and Forward the Data to the EndPoint
    cout << "=============== CH http(up) ===================" << endl;
    cout << "Received a Client packet..................... " << endl;
    cout << "Length of Packet is " << len << endl;
    cout << "Packet Type = " << (int) *((unsigned char *) Buffer) << endl;
    cout << "======================================" << endl;
    LogResponse((char*)Buffer,len);
    send(CData.forward_port, Buffer, len, 0);
    return true;
}

bool CHttpHandler::HandleDownStreamData(void *Buffer, int len, CLIENT_DATA &CData) {

    // Log the Content and Forward the Data to the EndPoint
    cout << "================= CH http (down) =================" << endl;
    cout << "Received a Server packet..................... " << endl;
    cout << "Length of Packet is " << len << endl;
    cout << "Packet Type = " << (int) *((unsigned char *) Buffer) << endl;
    cout << "======================================" << endl;
    send(CData.Sh, Buffer, len, 0);
    return true;
}


void CHttpHandler::LogResponse(char * buffer, int len)
{
    LineGrabber ln(buffer, len);
    string first_line = ln.getNextLine();
    char first_line2[1024];
    ::strcpy(first_line2, first_line.c_str());
    char method[4096];
    char url[4096];
    char protocol[4096];
    sscanf(first_line2, "%s %s %s", method, url, protocol);
    cout << "================================================================================" << endl;
    cout << "SERVER RESPONSE" << endl;
    cout << "--------------------------------------------------------------------------------" << endl;
    cout << "Http Method: " << method << endl;
    cout << "URL: " << url << endl;
    cout << "Protocol: " << protocol << endl;
    cout << "--------------------------------------------------------------------------------" << endl;
    cout << "Headers" << endl;
    cout << "--------------------------------------------------------------------------------" << endl;
    map<string, string> header_map;
    while (!ln.isEof())
    {
        string test = ln.getNextLine();
        if (strlen(test.c_str()) == 0)
        {
            break;
        }
        pair<string, string> temp = ChopLine(test);
        header_map.insert(temp);
        cout << temp.first << ": " << temp.second << endl;
    }
    cout << "--------------------------------------------------------------------------------" << endl;
    cout << "Body" << endl;
    cout << "--------------------------------------------------------------------------------" << endl;
    string body;
    while (!ln.isEof())
    {
        if (!body.empty())
        {
            body += CRLF;
        }
        body += ln.getNextLine();
    }
    cout << body << endl;
    cout << "================================================================================" << endl;
}

pair<string, string> ChopLine(string str) {
    char buffer[4096];
    ::strcpy(buffer, str.c_str());
    char *ptr = buffer;
    string key;
    string value;
    while (*ptr != 0 && *ptr != ':') {
        ptr++;
    }
    if (*ptr == 0) {
        return pair<string, string>("", "");
    }
    *ptr++ = 0;
    key = string(buffer);
    value = string(ptr);
    return pair<string, string>(key, value);
}

