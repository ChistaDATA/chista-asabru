#include "PingHandler.h"
#include "ServerSocket.h"

bool PingHandler::Handler(void *Buffer, int len, CLIENT_DATA &clientData)
{
    cout << "Pinged from Remote Server ....................." << endl;
    return true;
}