#include "ServerSocket.h"
#include "CProxyHandler.h"
#include "CMySQLHandler.h"

void *CMySQLHandler::HandleUpstreamData(void *buffer, int length, SocketClient * target_socket)
{
    // Log the Content and Forward the Data to the EndPoint
    std::cout << "=============CMySQLHandler(UP)=====================" << endl;
    std::cout << "Received a Client packet..................... " << endl;
    std::cout << "Length of Packet is " << length << endl;
    // std::cout << "Packet Type = " <<  (int)  *((unsigned char *) Buffer) << endl;
    std::cout << "======================================" << endl;
    
    target_socket->SendBytes((char *) buffer, length);
}

void *CMySQLHandler::HandleDownStreamData(void *buffer, int length, Socket * client_socket)
{
    // Log the Content and Forward the Data to the EndPoint
    std::cout << "===============CMySQLHandler(DOWN)===================" << endl;
    std::cout << "Received a Server packet..................... " << endl;
    std::cout << "Length of Packet is " << length << endl;
    // std::cout << "Packet Type = " <<  (int)  *((unsigned char *) Buffer) << endl;
    std::cout << "======================================" << endl;
        
    client_socket->SendBytes((char *) buffer, length);
}