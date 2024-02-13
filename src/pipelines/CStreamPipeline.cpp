#include "CProtocolSocket.h"
#include "../config/ConfigSingleton.h"
#include "SocketSelect.h"
#include <utility>
#include "Utils.h"
#include "file_transfer/Payload.h"

/**
 * Helper function to print the packet type
 */
int GetPacketType(std::string bytes) {
    // Get the packet type
    int packet_type = *((int *) bytes.c_str());
    printf("%d\n", packet_type);
    return packet_type;
}

std::string GetRequest(Socket *client_socket) {
    std::string request;
    while (true) {
        SocketSelect *sel;
        try {
            sel = new SocketSelect(client_socket, nullptr, NonBlockingSocket);
        }
        catch (std::exception &e) {
            LOG_ERROR(e.what());
            LOG_ERROR("error occurred while creating socket select ");
        }

        bool still_connected = true;

        try {
            if (sel->Readable(client_socket)) {
                LOG_INFO("client socket is readable, reading bytes : ");
                std::string bytes = client_socket->ReceiveBytes();

                if (!bytes.empty()) {
                    // LOG_INFO("Calling Protocol Handler..");
                    return bytes;
                }
                if (bytes.empty())
                    still_connected = false;
            }
        }
        catch (std::exception &e) {
            LOG_ERROR("Error while reading data from client :");
            LOG_ERROR(e.what());
        }

        if (!still_connected) {
            break;
        }
    }
    return request;
}

/**
 * Helper function to send acknowledgement
 * packet to a socket
 */
void SendAcknowledgement(Socket *client_socket) {
    T_ACK ack = MakeAck();
    client_socket->SendBytes((char *)&ack, sizeof(ack));
}

/**
 * Send EOF packet to a socket
 */
void SendEOF(Socket *client_socket) {
    T_FILE_EOF eof = MakeEof( );
    client_socket->SendBytes((char *)&eof, sizeof(eof));
}

void *CStreamPipeline(CProtocolSocket *ptr, void *lptr) {
    LOG_INFO("Starting CStreamPipeline");
    CLIENT_DATA clientData;
    memcpy(&clientData, lptr, sizeof(CLIENT_DATA));

//    // Check if handler is defined
//    CProtocolHandler *protocol_handler = ptr->GetHandler();
//    if (protocol_handler == nullptr) {
//        LOG_ERROR("The handler is not defined. Exiting!");
//        return nullptr;
//    }
    auto *client_socket = (Socket *) clientData.client_socket;
    EXECUTION_CONTEXT exec_context;

    ProtocolHelper::SetReadTimeOut(client_socket->GetSocket(), 1);

    // Receive data from the socket
    std::string request = GetRequest(client_socket);

    // Get the packet type
    int packet_type = GetPacketType(request);

    // Check if the first packet is handshake packet, else throw error
    if (packet_type != 1) {
        std::cout << "could not receive the right packet" << std::endl;
        return nullptr;
    }

    // Since we have received the handshake packet, send the acknowledgment packet
    T_ACK ack = MakeAck();
    client_socket->SendBytes((char *)&ack, sizeof(ack));
    std::cout << "Finished Sending the Acknowledgement ... " << std::endl;

    // Read from the client socket connection
    request = GetRequest(client_socket);

    // Get the packet type
    packet_type = GetPacketType(request);

    // Check if the packet type send is of type file metadata
    if (packet_type != 3) {
        std::cout << "Expected Meta Data .... " << std::endl;
        return nullptr;
    }

    // Read file_meta_data from buffer
    T_FILE_META file_meta_data;
    memcpy(&file_meta_data, request.c_str(), sizeof(file_meta_data));

    // Send acknowledgment
    std::cout << " file name = " << file_meta_data.file_name << " size = " << file_meta_data.size << std::endl;
    SendAcknowledgement(client_socket);

    std::cout << "finished .....sending acknowledgement.....\n" << "transferring files" << std::endl;
    std::cout << "Waiting for the content " << std::endl;

    // Open / Create a file in write binary mode
    FILE *fp = fopen("DEST_WIRE.out", "wb");

    // Loop and receive the file data chunks till we receive EOF packet
    int chunk_size = sizeof(T_FILE_CHUNK) + 5000;
    char buffer_chunk[chunk_size];
    int transferred_size = 0;
    while (true) {
        // Read from socket and move the bytes to buffer
        std::cout << "Transferred = " << transferred_size << std::endl;
        memset(buffer_chunk, 0, sizeof(buffer_chunk));
        ProtocolHelper::ReadSocketBuffer((SOCKET) client_socket->GetSocket(), buffer_chunk, sizeof(buffer_chunk));
        std::cout << "Packet Type = " << *(int *) buffer_chunk << std::endl;

        // Check if packet_type is EOF, and close the file pointer and thread if it so.
        if (*(int *) buffer_chunk == 5) {
            std::cout << "End of File Received" << std::endl;
            fclose(fp);

            // Send acknowledment to client
            SendAcknowledgement(client_socket);
            if (transferred_size != file_meta_data.size) {
                std::cout << "Expected Size = " << file_meta_data.size << std::endl;
                std::cout << "Received Size = " << transferred_size << std::endl;
            }
            break;
        }

        // Check if the packet type is of file chunk, terminate if it is not
        if (*(int *) buffer_chunk != 4) {
            std::cout << "I do not know what happens here" << std::endl;
            std::cout << "What kind of packet = " << *(int *) buffer_chunk << std::endl;
            fclose(fp);
            break;
        }

        // Receive the file chunk, copy to temporary memory, and write it to desination file.
        auto *chunk = (T_FILE_CHUNK *) buffer_chunk;
        char temp_mem[32000];

        std::cout << "Packet sequence ...... " << chunk->packet_seq_num << std::endl;
        if (chunk->packet_seq_num > 10) {
            break;
        }
        memcpy(temp_mem, chunk->buffer, chunk->buffer_size);
        fwrite(temp_mem, 1, chunk->buffer_size, fp);
        transferred_size += chunk->buffer_size;
    }

    // Close the client socket
    LOG_INFO("Closing the client socket");
    client_socket->Close();

#ifdef WINDOWS_OS
    return 0;
#else
    return nullptr;
#endif
}
