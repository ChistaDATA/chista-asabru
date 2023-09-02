#include "ServerSocket.h"
#include "CProxyHandler.h"
#include "CPostgreSQLHandler.h"

void * CPostgreSQLHandler::HandleUpstreamData(void *buffer, int len, SocketClient * target_socket)
{
    // Log the Content and Forward the Data to the EndPoint
    std::cout << "=============CPostgreSQLHandler(UP)=====================" << endl;
    std::cout << "Received a Client packet..................... " << endl;

    std::cout << "Length of Packet is " << len << endl;

    target_socket->SendBytes((char *) buffer);
}

void * CPostgreSQLHandler::HandleDownStreamData(void *buffer, int len)
{
    // Log the Content and Forward the Data to the EndPoint
    std::cout << "===============CPostgreSQLHandler(DOWN)===================" << endl;
    std::cout << "Received a Server packet..................... " << endl;
    std::cout << "Length of Packet is " << len << endl;

    return buffer;
}
