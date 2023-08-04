
#include <map>
#include "Utils.h"
#include "./CHttpsHandler.h"
#include "LineGrabber.h"
using namespace std;

CHttpsHandler::CHttpsHandler()
{
}

/**
 * Function that handles upstream data
 * @param buffer - the buffer that we receive from upstream ( source dbs )
 * @param length - length of the buffer
 * @param clientData - contains connection information about the client
 */
bool CHttpsHandler::HandleUpstreamData(void *buffer, int length, CLIENT_DATA &clientData)
{
    std::cout << "=============== CH http(up) ===================" << endl;
    std::cout << "Received a Client packet..................... " << endl;
    std::cout << "Length of Packet is " << length << endl;
    std::cout << "Packet Type = " << (int)*((unsigned char *)buffer) << endl;

    // Forward the Data to the EndPoint
    send(clientData.forward_port, buffer, length, 0);

    return true;
}

/**
 * Function that handles downstream data
 * @param buffer - the buffer / response that we receive from downstream ( target dbs )
 * @param length - length of the buffer
 * @param clientData - contains connection information about the client
 */
bool CHttpsHandler::HandleDownStreamData(void *buffer, int length, CLIENT_DATA &clientData)
{

    // Log the Content and Forward the Data to the EndPoint
    std::cout << "================= CH http (down) =================" << endl;
    std::cout << "Received a Server packet..................... " << endl;
    std::cout << "Length of Packet is " << length << endl;
    std::cout << "Packet Type = " << (int)*((unsigned char *)buffer) << endl;
    std::cout << "======================================" << endl;
    send(clientData.Sh, buffer, length, 0);
    return true;
}

void CHttpsHandler::LogResponse(char *buffer, int len)
{
    LineGrabber ln(buffer, len);
    string first_line = ln.getNextLine();
    char first_line2[1024];
    ::strcpy(first_line2, first_line.c_str());
    char method[4096];
    char url[4096];
    char protocol[4096];
    sscanf(first_line2, "%s %s %s", method, url, protocol);
    std::cout << "================================================================================" << endl;
    std::cout << "SERVER RESPONSE" << endl;
    std::cout << "--------------------------------------------------------------------------------" << endl;
    std::cout << "Http Method: " << method << endl;
    std::cout << "URL: " << url << endl;
    std::cout << "Protocol: " << protocol << endl;
    std::cout << "--------------------------------------------------------------------------------" << endl;
    std::cout << "Headers" << endl;
    std::cout << "--------------------------------------------------------------------------------" << endl;
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
        std::cout << temp.first << ": " << temp.second << endl;
    }

    // Rest of the packet will be the body
    std::cout << "--------------------------------------------------------------------------------" << endl;
    std::cout << "Body" << endl;
    std::cout << "--------------------------------------------------------------------------------" << endl;
    string body;
    while (!ln.isEof())
    {
        if (!body.empty())
        {
            body += CRLF;
        }
        body += ln.getNextLine();
    }
    std::cout << body << endl;
    std::cout << "================================================================================" << endl;
}

