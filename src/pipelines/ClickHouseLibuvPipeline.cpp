//
// Created by Midhun Darvin on 20/10/23.
//

#include "CProtocolSocket.h"
#include "Pipeline.h"

namespace clickhouse_pipeline {
    // Callback function for memory allocation when reading data
    void on_alloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
        std::cout << "on_alloc" << std::endl;
        // Allocate a buffer for incoming data
        std::cout << "suggested_size : " << suggested_size << std::endl;
        buf->base = (char *) malloc(suggested_size);
        buf->len = suggested_size;
    }

    // Callback function for when data is received
    void on_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
        auto *connection_data = (ConnectionData *) stream->data;
        ClientTargetPair *pair = connection_data->pair;
        EXECUTION_CONTEXT exec_context;

        if (nread < 0) {
            // Error or end of stream, close both client and target connections
            uv_close((uv_handle_t *) &pair->client, nullptr);
            uv_close((uv_handle_t *) &pair->target, nullptr);
            free(buf->base);
            return;
        }

        if (nread > 0) {
            // Determine whether the stream is the client or target
            if (stream == (uv_stream_t *) &pair->client) {
                std::cout << "Calling Proxy Upstream Handler.." << std::endl;

                std::cout << "Buffer length " << std::string((char *) buf->base).size() << std::endl;
                std::string response = connection_data->proxy_handler->HandleUpstreamData((void *)buf->base, nread, &exec_context);
                // Data received from the client, forward it to the target server
                auto *write_req = new uv_write_t;
                uv_buf_t write_buf = uv_buf_init((char *) response.c_str(), response.size());
                uv_write(write_req, (uv_stream_t *) &pair->target, &write_buf, 1, nullptr);
            } else {
                std::cout << "Calling Proxy Downstream Handler.." << std::endl;
                std::string response = connection_data->proxy_handler->HandleDownStreamData(buf->base, nread, &exec_context);
                // Data received from the target server, forward it to the client
                auto *write_req = new uv_write_t;
                uv_buf_t write_buf = uv_buf_init((char *) response.c_str(), response.size());
                uv_write(write_req, (uv_stream_t *) &pair->client, &write_buf, 1, nullptr);
            }

            free(buf->base);
        }
    }

    // Callback function when a connection is established to the target server
    void on_target_connected(uv_connect_t *req, int status) {
        auto *connection_data = (ConnectionData *) req->data;
        ClientTargetPair *pair = connection_data->pair;

        if (status < 0) {
            fprintf(stderr, "Target connection error: %s\n", uv_strerror(status));
            uv_close((uv_handle_t *) &pair->client, nullptr);
            free(req);
            return;
        }

        // Associate the pair with the target socket
        pair->target.data = connection_data;

        // Start reading from the target server
        uv_read_start((uv_stream_t *) &pair->client, clickhouse_pipeline::on_alloc, clickhouse_pipeline::on_read);
        uv_read_start((uv_stream_t *) &pair->target, clickhouse_pipeline::on_alloc, clickhouse_pipeline::on_read);
    }
}

void *ClickHouseLibuvPipeline(LibuvProxySocket *ptr, void *lptr) {
    std::cout << "ClickHouseLibuvPipeline : " << std::endl;
    CLIENT_DATA clientData;
    memcpy(&clientData, lptr, sizeof(CLIENT_DATA));
    ClientTargetPair *pair = clientData.client_target_pair;

    // Retrieve the load balancer class
    auto loadBalancer = ptr->loadBalancer;

    // Check if handler is defined
    CProxyHandler *proxy_handler = ptr->GetHandler();
    if (proxy_handler == nullptr) {
        std::cout << "The handler is not defined. Exiting!" << std::endl;
        return nullptr;
    }

    ConnectionData *connection_data;
    connection_data = (ConnectionData *) malloc(sizeof(ConnectionData));
    connection_data->pair = pair;
    connection_data->proxy_handler = proxy_handler;

    RESOLVED_SERVICE currentService = loadBalancer->requestServer();
    END_POINT target_endpoint {
        currentService.ipaddress,
        currentService.port,
        currentService.r_w,
        currentService.alias,
        currentService.reserved,
        "  "
    };
    std::cout << "Resolved (Target) Host: " << target_endpoint.ipaddress << std::endl
         << "Resolved (Target) Port: " << target_endpoint.port << std::endl;

    // Connect to target database
    std::string error;
    hostent *he;
    try
    {
        if ((he = gethostbyname(target_endpoint.ipaddress.c_str())) == nullptr)
        {
            std::cout << "Unable to get host endpoint by name " << std::endl;
            error = strerror(errno);
            throw std::runtime_error(error);
        }
    }
    catch (std::exception &e)
    {
        std::cout << e.what() << std::endl;
        std::cout << "Unable to get host endpoint by name " << std::endl;
    }
    char * ip_address = inet_ntoa(*(struct in_addr *)he->h_addr);
    // Step 4
    std::cout << "IP address of " << he->h_name << " is: " << ip_address << std::endl;
    std::cout << ip_address << ":" << target_endpoint.port << std::endl;

    try {
        // Connect to the target server
        auto *connect_req = (uv_connect_t *) malloc(sizeof(uv_connect_t));

        if (!connect_req) {
            fprintf(stderr, "Memory allocation failed\n");
            uv_close((uv_handle_t *) &pair->client, nullptr);
            free(pair);
            return 0;
        }

        connect_req->data = connection_data;

        struct sockaddr_in client_addr;
        uv_ip4_addr(ip_address, target_endpoint.port, &client_addr);
        uv_tcp_connect(connect_req, &pair->target, (const struct sockaddr *) &client_addr,
                       clickhouse_pipeline::on_target_connected);
    }
    catch (std::exception &e) {
        std::cout << e.what() << std::endl;
        std::cout << "Error when connecting to target socket" << std::endl;
    }

    // Associate the pair with the client socket
    pair->client.data = connection_data;

#ifdef WINDOWS_OS
    return 0;
#else
    return nullptr;
#endif

    return 0;
}