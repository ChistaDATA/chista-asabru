#include "ServerSocket.h"
#include "CProxyHandler.h"
#include "CHWirePTHandler.h"

void *CHWirePTHandler::HandleUpstreamData(void *buffer, int length, SocketClient * target_socket)
{
    // Log the Content and Forward the Data to the EndPoint
    std::cout << "=============== CH Wire protocol (up)===================" << endl;
    std::cout << "Received a Client packet..................... " << endl;
    std::cout << "Length of Packet is " << length << endl;
    std::cout << "Packet Type = " << (int)*((unsigned char *)buffer) << endl;

    target_socket->SendBytes((char *) buffer, length);
    std::cout << "Send bytes successfully!" << endl;
}

void *CHWirePTHandler::HandleDownStreamData(void *buffer, int length, Socket * client_socket)
{
    // Log the Content and Forward the Data to the EndPoint
    std::cout << "================= CH Wire protocol (down)=================" << endl;
    std::cout << "Received a Server packet..................... " << endl;
    std::cout << "Length of Packet is " << length << endl;
    std::cout << "Packet Type = " << (int)*((unsigned char *)buffer) << endl;

    client_socket->SendBytes((char *) buffer, length);
    std::cout << "Send bytes successfully!" << endl;
}