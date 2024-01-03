#include "CProtocolSocket.h"
#include "../config/ConfigSingleton.h"
#include "Socket.h"
#include "SocketSelect.h"
#include <utility>

void *ProtocolPipeline(CProtocolSocket *ptr, void *lptr)
{
    LOG_INFO("Starting ProtocolPipeline");
    CLIENT_DATA clientData;
    memcpy(&clientData, lptr, sizeof(CLIENT_DATA));

    // Check if handler is defined
    CProtocolHandler *protocol_handler = ptr->GetHandler();
    if (protocol_handler == nullptr)
    {
        LOG_ERROR("The handler is not defined. Exiting!");
        return nullptr;
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
            LOG_ERROR(e.what());
            LOG_ERROR("error occurred while creating socket select ");
        }

        bool still_connected = true;
        try
        {
            if (sel->Readable(client_socket))
            {
                LOG_INFO("client socket is readable, reading bytes : ");
                std::string bytes = client_socket->ReceiveBytes();

                if (!bytes.empty()) {
                    LOG_INFO("Calling Protocol Handler..");
                    response = protocol_handler->HandleData((void *)bytes.c_str(), bytes.size(), &exec_context);
                    client_socket->SendBytes((char *)response.c_str(), response.size());
                }
                if (bytes.empty())
                    still_connected = false;
            }
        }
        catch (std::exception &e)
        {
            LOG_ERROR("Error while sending to target :");
            LOG_ERROR(e.what());
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