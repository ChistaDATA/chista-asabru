#include "ServerSocket.h"
#include "CProxyHandler.h"
#include "CPostgreSQLHandler.h"

void * CPostgreSQLHandler::HandleUpstreamData(void *buffer, int length, SocketClient * target_socket)
{
    // Log the Content and Forward the Data to the EndPoint
    std::cout << "=============CPostgreSQLHandler(UP)=====================" << endl;
    std::cout << "Received a Client packet..................... " << endl;

    std::cout << "Length of Packet is " << length << endl;

    target_socket->SendBytes((char *) buffer, length);
}

void * CPostgreSQLHandler::HandleDownStreamData(void *buffer, int length, Socket * client_socket)
{
    // Log the Content and Forward the Data to the EndPoint
    std::cout << "===============CPostgreSQLHandler(DOWN)===================" << endl;
    std::cout << "Received a Server packet..................... " << endl;
    std::cout << "Length of Packet is " << length << endl;

    client_socket->SendBytes((char *) buffer, length);
}
