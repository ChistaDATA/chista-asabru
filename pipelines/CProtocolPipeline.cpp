#include "CProtocolSocket.h"
#include "../test/ProxyInfo.h"
#include "../config/ConfigSingleton.h"
#include "Socket.h"
#include "SocketSelect.h"
#include <utility>

static ConfigSingleton &configSingleton = ConfigSingleton::getInstance();

void *ProtocolPipeline(CProtocolSocket *ptr, void *lptr)
{
    std::cout <<"Starting ProtocolPipeline \n";
    CLIENT_DATA clientData;
    memcpy(&clientData, lptr, sizeof(CLIENT_DATA));

    // Check if handler is defined
    CProtocolHandler *protocol_handler = ptr->GetHandler();
    if (protocol_handler == 0)
    {
        cout << "The handler is not defined. Exiting!" << endl;
        return 0;
    }

    Socket *client_socket = (Socket *)clientData.client_socket;
    EXECUTION_CONTEXT exec_context;

    ProtocolHelper::SetReadTimeOut(client_socket->GetSocket(), 1);
    std::string response;

    while (1)
    {
        SocketSelect *sel;
        try
        {
            sel = new SocketSelect(client_socket, nullptr, NonBlockingSocket);
        }
        catch (std::exception &e)
        {
            cout << e.what() << endl;
            cout << "error occurred while creating socket select " << endl;
        }

        bool still_connected = true;
        try
        {
            if (sel->Readable(client_socket))
            {
                cout << "client socket is readable, reading bytes : " << endl;
                std::string bytes = client_socket->ReceiveBytes();

                cout << "Calling Protocol Handler.." << endl;
                response = protocol_handler->HandleData((void *)bytes.c_str(), bytes.size(), &exec_context);
                cout << "Response..\n" << response <<endl;
                client_socket->SendBytes((char *)response.c_str(), response.size());
                if (bytes.empty())
                    still_connected = false;
            }
        }
        catch (std::exception &e)
        {
            cout << "Error while sending to target " << e.what() << endl;
        }

        if (!still_connected)
        {
            break;
        }
    }

#ifdef WINDOWS_OS
    return 0;
#else
    return nullptr;
#endif
}