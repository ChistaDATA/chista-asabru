#include "ServerSocket.h"
#include "CProxyHandler.h"
#include "CPostgreSQLHandler.h"

bool CPostgreSQLHandler::HandleUpstreamData(void *Buffer, int len, CLIENT_DATA &clientData)
{
    // Log the Content and Forward the Data to the EndPoint
    std::cout << "=============CPostgreSQLHandler(UP)=====================" << endl;
    std::cout << "Received a Client packet..................... " << endl;

    std::cout << "Length of Packet is " << len << endl;
    // std::cout << "Packet Type = " <<  (int)  *((unsigned char *) Buffer) << endl;
    std::cout << "======================================" << endl;
    send(clientData.forward_port, Buffer, len, 0);
    return true;
}

bool CPostgreSQLHandler::HandleDownStreamData(void *Buffer, int len, CLIENT_DATA &clientData)
{
    // Log the Content and Forward the Data to the EndPoint
    std::cout << "===============CPostgreSQLHandler(DOWN)===================" << endl;
    std::cout << "Received a Server packet..................... " << endl;
    std::cout << "Length of Packet is " << len << endl;
    // std::cout << "Packet Type = " <<  (int)  *((unsigned char *) Buffer) << endl;
    std::cout << "======================================" << endl;
    send(clientData.client_port, Buffer, len, 0);
    return true;
}
