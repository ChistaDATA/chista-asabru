
#include "../handlers/CProtocolSocket.h"
#include "../handlers/CProxySocket.h"
#include "../test/ProxyInfo.h"
#include "ClientSocket.h"
#include "../config/ConfigSingleton.h"
#include "ProtocolHelper.h"
#include "Socket.h"
#include "Pipeline.h"

static ConfigSingleton &configSingleton = ConfigSingleton::getInstance();

namespace postgresql_pipeline
{
    // Callback function for memory allocation when reading data
    void on_alloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
    {
        // Allocate a buffer for incoming data
        buf->base = (char *)malloc(suggested_size);
        buf->len = suggested_size;
    }

    // Callback function for when data is received
    void on_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf)
    {
        ConnectionData *connection_data = (ConnectionData *)stream->data;
        ClientTargetPair *pair = connection_data->pair;

        if (nread < 0)
        {
            // Error or end of stream, close both client and target connections
            uv_close((uv_handle_t *)&pair->client, NULL);
            uv_close((uv_handle_t *)&pair->target, NULL);
            free(buf->base);
            return;
        }

        if (nread > 0)
        {
            // Forward data from source to target
            // uv_write_t write_req;
            // uv_buf_t write_buf = uv_buf_init(buf->base, nread);
            // uv_write(&write_req, (uv_stream_t *)stream->data, &write_buf, 1, NULL);

            // Determine whether the stream is the client or target
            if (stream == (uv_stream_t *)&pair->client)
            {
                cout << "Calling Proxy Upstream Handler.." << endl;
                connection_data->proxy_handler->HandleUpstreamData((void *)buf->base, nread, (uv_stream_t *)&pair->target);
            }
            else
            {
                cout << "Calling Proxy Downstream Handler.." << endl;
                connection_data->proxy_handler->HandleDownStreamData((void *)buf->base, nread, (uv_stream_t *)&pair->client);
            }

            free(buf->base);
        }
    }

    // Callback function when a connection is established to the target server
    void on_target_connected(uv_connect_t *req, int status)
    {
        if (status < 0)
        {
            fprintf(stderr, "Target connection error: %s\n", uv_strerror(status));
            uv_close((uv_handle_t *)&req->handle, NULL);
            free(req);
            return;
        }
        ConnectionData *connection_data = (ConnectionData *)req->data;
        ClientTargetPair *pair = connection_data->pair;

        // Associate the pair with the target socket
        pair->target.data = connection_data;

        // Start reading from the target server
        uv_read_start((uv_stream_t *)&pair->client, postgresql_pipeline::on_alloc, postgresql_pipeline::on_read);
        uv_read_start((uv_stream_t *)&pair->target, postgresql_pipeline::on_alloc, postgresql_pipeline::on_read);
    }
}

void *PostgreSQLPipeline(CProxySocket *ptr, void *lptr)
{
    CLIENT_DATA clientData;
    memcpy(&clientData, lptr, sizeof(CLIENT_DATA));
    ClientTargetPair *pair = clientData.client_target_pair;

    // Check if handler is defined
    CProxyHandler *proxy_handler = ptr->GetHandler();
    if (proxy_handler == 0)
    {
        cout << "The handler is not defined. Exiting!" << endl;
        return 0;
    }

    ConnectionData *connection_data;
    connection_data = (ConnectionData *)malloc(sizeof(ConnectionData));
    connection_data->pair = pair;
    connection_data->proxy_handler = proxy_handler;

    /**
     * Get the configuration data for the target database clusters ( eg. clickhouse )
     * Config given in config.xml
     */
    RESOLVE_ENDPOINT_RESULT result = ptr->GetConfigValues();
    END_POINT *target_endpoint = new END_POINT{result.ipaddress, result.port, result.r_w, result.alias, result.reserved, "  "}; // Resolve("firstcluster", "127.0.0.1" , 9000, pd );
    if (target_endpoint == 0)
    {
        cout << "Failed to retrieve target database configuration. Exiting!" << endl;
        return 0;
    }
    cout << "Resolved (Target) Host: " << target_endpoint->ipaddress << endl
         << "Resolved (Target) Port: " << target_endpoint->port << endl;

    try
    {
        // Connect to the target server
        uv_connect_t *connect_req = (uv_connect_t *)malloc(sizeof(uv_connect_t));

        if (!connect_req)
        {
            fprintf(stderr, "Memory allocation failed\n");
            uv_close((uv_handle_t *)&pair->client, NULL);
            free(pair);
            return 0;
        }

        connect_req->data = connection_data;

        struct sockaddr_in client_addr;
        uv_ip4_addr(target_endpoint->ipaddress.c_str(), target_endpoint->port, &client_addr);
        uv_tcp_connect(connect_req, &pair->target, (const struct sockaddr *)&client_addr, postgresql_pipeline::on_target_connected);
    }
    catch (std::exception &e)
    {
        cout << e.what() << endl;
        cout << "Error when connecting to target socket" << endl;
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