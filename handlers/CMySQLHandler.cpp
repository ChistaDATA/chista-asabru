#include "ServerSocket.h"
#include "CProxyHandler.h"
#include "CMySQLHandler.h"

void *CMySQLHandler::HandleUpstreamData(void *buffer, int length, uv_stream_t * target)
{
    // Log the Content and Forward the Data to the EndPoint
    std::cout << "=============CMySQLHandler(UP)=====================" << endl;
    std::cout << "Received a Client packet..................... " << endl;
    std::cout << "Length of Packet is " << length << endl;
    // std::cout << "Packet Type = " <<  (int)  *((unsigned char *) Buffer) << endl;
    std::cout << "======================================" << endl;
    
    // Data received from the client, forward it to the target server
    uv_write_t write_req;
    uv_buf_t write_buf = uv_buf_init((char *) buffer, length);
    uv_write(&write_req, (uv_stream_t *) target, &write_buf, 1, NULL);
}

void *CMySQLHandler::HandleDownStreamData(void *buffer, int buffer_length, uv_stream_t *client)
{
    // Log the Content and Forward the Data to the EndPoint
    std::cout << "===============CMySQLHandler(DOWN)===================" << endl;
    std::cout << "Received a Server packet..................... " << endl;
    std::cout << "Length of Packet is " << buffer_length << endl;
    // std::cout << "Packet Type = " <<  (int)  *((unsigned char *) Buffer) << endl;
    std::cout << "======================================" << endl;
        
    // Data received from the target server, forward it to the client
    uv_write_t write_req;
    uv_buf_t write_buf = uv_buf_init((char *) buffer, buffer_length);
    uv_write(&write_req, client, &write_buf, 1, NULL);
}