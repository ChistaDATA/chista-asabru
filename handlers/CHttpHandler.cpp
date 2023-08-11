
#include <map>
#include "Utils.h"
#include "CHttpHandler.h"
#include "LineGrabber.h"

CHttpHandler::CHttpHandler(CHttpParser *parser)
{
    this->parser = parser;
}

/**
 * Function that handles upstream data
 * @param buffer - the buffer that we receive from upstream ( source dbs )
 * @param length - length of the buffer
 * @param clientData - contains connection information about the client
 */
bool CHttpHandler::HandleUpstreamData(void *buffer, int length, CLIENT_DATA &clientData)
{
    cout << "=============== CH http(up) ===================" << endl;
    cout << "Received a Client packet..................... " << endl;
    cout << "Length of Packet is " << length << endl;
    cout << "Packet Type = " << (int)*((unsigned char *)buffer) << endl;

    // Parse the buffer to a metadata struct
    // This is done so as to analyze the packet easily and filter or apply custom logic
    HttpMetadata metadata = this->parser->construct(buffer, length);

    // Log the Content
    this->parser->logMetadata(&metadata);

    // Reconstruct the buffer from the metadata struct
    void *deconstructedBuffer = this->parser->deconstruct(&metadata);
    
    int deconstructedBufferSize = strlen((char *)deconstructedBuffer);
    cout << "Lengths : " << length << " " << deconstructedBufferSize << endl;

    LogResponse((char *) buffer, length);
    LogResponse((char *) deconstructedBuffer, deconstructedBufferSize);

    // Forward the Data to the EndPoint
    send(clientData.forward_port, deconstructedBuffer, deconstructedBufferSize, 0);

    // free the buffer memory
    free(deconstructedBuffer);
    return true;
}

/**
 * Function that handles downstream data
 * @param buffer - the buffer / response that we receive from downstream ( target dbs )
 * @param length - length of the buffer
 * @param clientData - contains connection information about the client
 */
bool CHttpHandler::HandleDownStreamData(void *buffer, int length, CLIENT_DATA &clientData)
{

    // Log the Content and Forward the Data to the EndPoint
    cout << "================= CH http (down) =================" << endl;
    cout << "Received a Server packet..................... " << endl;
    cout << "Length of Packet is " << length << endl;
    cout << "Packet Type = " << (int)*((unsigned char *)buffer) << endl;
    cout << "======================================" << endl;
    send(clientData.client_port, buffer, length, 0);
    return true;
}

void CHttpHandler::LogResponse(char *buffer, int len)
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
        if (strlen(test.c_str()) == 0) // break if there is an empty line
        {
            break;
        }
        pair<string, string> temp = ChopLine(test);
        header_map.insert(temp);
        cout << temp.first << ": " << temp.second << endl;
    }

    // Rest of the packet will be the body
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

