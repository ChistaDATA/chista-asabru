
#include <map>
#include "./CHttpParser.h"
#include "../common/LineGrabber.h"
using namespace std;

void writeStringToBuffer(char *buffer, const char *str, size_t &position);

CHttpParser::CHttpParser() {}

HttpMetadata CHttpParser::construct(void *buffer, int len)
{
    HttpMetadata result;
    LineGrabber lineGrabber((char *)buffer, len);

    // Grab the first line for method, url and protocol
    std::string first_line = lineGrabber.getNextLine();
    char first_line_copy[1024];
    ::strcpy(first_line_copy, first_line.c_str());
    char method[4096];
    char url[4096];
    char protocol[4096];
    sscanf(first_line_copy, "%s %s %s", method, url, protocol);
    result.url = url;
    result.protocol = protocol;
    result.method = method;

    // Grab the headers
    while (!lineGrabber.isEof())
    {
        std::string test = lineGrabber.getNextLine();
        if (strlen(test.c_str()) == 0)
        {
            break;
        }
        pair<std::string, std::string> temp = ChopLine(test);
        result.headers.insert(temp);
    }

    // Rest of the content will be the body of the packet
    string body;
    while (!lineGrabber.isEof())
    {
        if (!body.empty())
        {
            body += CRLF;
        }
        body += lineGrabber.getNextLine();
    }
    result.body = body;
    return result;
}

void *CHttpParser::deconstruct(HttpMetadata *httpMetadata)
{
    std::cout << "Deconstructing Http metadata (start): " << endl;
    string buffer = "";
    buffer += (*httpMetadata).method + " " + (*httpMetadata).url + " " + (*httpMetadata).protocol + "\r\n";

    map<string, string> *headers = &((*httpMetadata).headers);
    for (auto i : *headers)
    {
        buffer += i.first + ":" + i.second + "\r\n";
    }

    buffer += "\r\n";
    buffer += (*httpMetadata).body;

    size_t buffer_length = strlen(buffer.c_str());
    char *result = (char *)malloc(buffer_length);
    memcpy(result, buffer.c_str(), buffer_length);

    std::cout << "Deconstructing Http metadata (end): " << endl;
    return result;
}

void CHttpParser::logMetadata(HttpMetadata *metadata)
{
    // Log the Content and Forward the Data to the EndPoint
    cout << "=============== CH HTTP ( packet contents ) ===================" << endl;
    cout << "Method: " << metadata->method << endl;
    cout << "URL: " << metadata->url << endl;
    cout << "Protocol: " << metadata->protocol << endl;

    cout << "--------------- Headers Start ---------------------------------:" << endl;
    map<string, string> headers = metadata->headers;
    for (auto i = headers.begin(); i != headers.end(); i++)
        cout << i->first << "	 " << i->second << endl;
    cout << "---------------  Headers End  ---------------------------------:" << endl;

    cout << "--------------- Body Content ---------------------------------:" << endl;
    cout << metadata->body << endl;
}
