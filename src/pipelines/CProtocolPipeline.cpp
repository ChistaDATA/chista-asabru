#include "CProtocolSocket.h"
#include "../config/ConfigSingleton.h"
#include "Socket.h"
#include "SocketSelect.h"
#include <utility>

void *ProtocolPipeline(CProtocolSocket *ptr, void *lptr)
{
    std::cout << "Starting ProtocolPipeline \n";
    CLIENT_DATA clientData;
    memcpy(&clientData, lptr, sizeof(CLIENT_DATA));

    // Check if handler is defined
    CProtocolHandler *protocol_handler = ptr->GetHandler();
    if (protocol_handler == 0)
    {
        std::cout << "The handler is not defined. Exiting!" << std::endl;
        return 0;
    }

    auto *client_socket = (Socket *)clientData.client_socket;
    EXECUTION_CONTEXT exec_context;

    ProtocolHelper::SetReadTimeOut(client_socket->GetSocket(), 1);
    std::string response;

    while (true)
    {
        SocketSelect *sel;
        try
        {
            sel = new SocketSelect(client_socket, nullptr, NonBlockingSocket);
        }
        catch (std::exception &e)
        {
            std::cout << e.what() << std::endl;
            std::cout << "error occurred while creating socket select " << std::endl;
        }

        bool still_connected = true;
        try
        {
            if (sel->Readable(client_socket))
            {
                std::cout << "client socket is readable, reading bytes : " << std::endl;
                std::string bytes = client_socket->ReceiveBytes();

                if (!bytes.empty()) {
                    std::cout << "Calling Protocol Handler.." << std::endl;
                    response = protocol_handler->HandleData((void *)bytes.c_str(), bytes.size(), &exec_context);
                    client_socket->SendBytes((char *)response.c_str(), response.size());
                }
                if (bytes.empty())
                    still_connected = false;
            }
        }
        catch (std::exception &e)
        {
            std::cout << "Error while sending to target " << e.what() << std::endl;
        }

        if (!still_connected)
        {
            // Close the client socket
            client_socket->Close();
            break;
        }
    }

#ifdef WINDOWS_OS
    return 0;
#else
    return nullptr;
#endif
}