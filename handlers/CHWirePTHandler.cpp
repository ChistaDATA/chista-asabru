#include "ServerSocket.h"
#include "CProxyHandler.h"
#include "CHWirePTHandler.h"

void *CHWirePTHandler::HandleUpstreamData(void *buffer, int len, SocketClient * target_socket)
{
    // Log the Content and Forward the Data to the EndPoint
    std::cout << "===============CH(up)===================" << endl;
    std::cout << "Received a Client packet..................... " << endl;
    std::cout << "Length of Packet is " << len << endl;
    std::cout << "Packet Type = " << (int)*((unsigned char *)buffer) << endl;

    target_socket->SendBytes((char *) buffer);
}

void *CHWirePTHandler::HandleDownStreamData(void *buffer, int len)
{
    // Log the Content and Forward the Data to the EndPoint
    std::cout << "=================CH (down)=================" << endl;
    std::cout << "Received a Server packet..................... " << endl;
    std::cout << "Length of Packet is " << len << endl;
    std::cout << "Packet Type = " << (int)*((unsigned char *)buffer) << endl;
    return buffer;
}