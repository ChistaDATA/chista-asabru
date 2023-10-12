
#include "CProtocolSocket.h"
#include "CProxySocket.h"
#include "../test/ProxyInfo.h"
#include "CClientSocket.h"
#include "../config/ConfigSingleton.h"
#include "ProtocolHelper.h"
#include "Socket.h"
#include "Pipeline.h"

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

static ConfigSingleton &configSingleton = ConfigSingleton::getInstance();

/* Helper function to create a BIO connected to the server */
static BIO *create_socket_bio(const char *hostname, const char *port)
{
    int sock = -1;
    BIO_ADDRINFO *res;
    const BIO_ADDRINFO *ai = NULL;
    BIO *bio;

    /*
     * Lookup IP address info for the server.
     */
    if (!BIO_lookup_ex(hostname, port, BIO_LOOKUP_CLIENT, 0, SOCK_STREAM, 0,
                       &res))
        return NULL;

    /*
     * Loop through all the possible addresses for the server and find one
     * we can connect to.
     */
    for (ai = res; ai != NULL; ai = BIO_ADDRINFO_next(ai))
    {
        /*
         * Create a TCP socket. We could equally use non-OpenSSL calls such
         * as "socket" here for this and the subsequent connect and close
         * functions. But for portability reasons and also so that we get
         * errors on the OpenSSL stack in the event of a failure we use
         * OpenSSL's versions of these functions.
         */
        sock = BIO_socket(BIO_ADDRINFO_family(ai), SOCK_STREAM, 0, 0);
        if (sock == -1)
            continue;

        /* Connect the socket to the server's address */
        if (!BIO_connect(sock, BIO_ADDRINFO_address(ai), BIO_SOCK_NODELAY))
        {
            BIO_closesocket(sock);
            sock = -1;
            continue;
        }

        /* Set to nonblocking mode */
        if (!BIO_socket_nbio(sock, 1))
        {
            sock = -1;
            continue;
        }

        /* We have a connected socket so break out of the loop */
        break;
    }

    /* Free the address information resources we allocated earlier */
    BIO_ADDRINFO_free(res);

    /* If sock is -1 then we've been unable to connect to the server */
    if (sock == -1)
        return NULL;

    /* Create a BIO to wrap the socket */
    bio = BIO_new(BIO_s_socket());
    if (bio == NULL)
    {
        BIO_closesocket(sock);
        return NULL;
    }

    /*
     * Associate the newly created BIO with the underlying socket. By
     * passing BIO_CLOSE here the socket will be automatically closed when
     * the BIO is freed. Alternatively you can use BIO_NOCLOSE, in which
     * case you must close the socket explicitly when it is no longer
     * needed.
     */
    BIO_set_fd(bio, sock, BIO_CLOSE);

    return bio;
}

static void wait_for_activity(SSL *ssl, int write)
{
    fd_set fds;
    int width, sock;

    /* Get hold of the underlying file descriptor for the socket */
    sock = SSL_get_fd(ssl);

    FD_ZERO(&fds);
    FD_SET(sock, &fds);
    width = sock + 1;

    /*
     * Wait until the socket is writeable or readable. We use select here
     * for the sake of simplicity and portability, but you could equally use
     * poll/epoll or similar functions
     *
     * NOTE: For the purposes of this demonstration code this effectively
     * makes this demo block until it has something more useful to do. In a
     * real application you probably want to go and do other work here (e.g.
     * update a GUI, or service other connections).
     *
     * Let's say for example that you want to update the progress counter on
     * a GUI every 100ms. One way to do that would be to add a 100ms timeout
     * in the last parameter to "select" below. Then, when select returns,
     * you check if it did so because of activity on the file descriptors or
     * because of the timeout. If it is due to the timeout then update the
     * GUI and then restart the "select".
     */
    if (write)
        select(width, NULL, &fds, NULL, NULL);
    else
        select(width, &fds, NULL, NULL, NULL);
}

static int handle_io_failure(SSL *ssl, int res)
{
    switch (SSL_get_error(ssl, res))
    {
    case SSL_ERROR_WANT_READ:
        /* Temporary failure. Wait until we can read and try again */
        wait_for_activity(ssl, 0);
        return 1;

    case SSL_ERROR_WANT_WRITE:
        /* Temporary failure. Wait until we can write and try again */
        wait_for_activity(ssl, 1);
        return 1;

    case SSL_ERROR_ZERO_RETURN:
        /* EOF */
        return 0;

    case SSL_ERROR_SYSCALL:
        return -1;

    case SSL_ERROR_SSL:
        /*
         * If the failure is due to a verification error we can get more
         * information about it from SSL_get_verify_result().
         */
        if (SSL_get_verify_result(ssl) != X509_V_OK)
            printf("Verify error: %s\n",
                   X509_verify_cert_error_string(SSL_get_verify_result(ssl)));
        return -1;

    default:
        return -1;
    }
}

/**
 * The thread that listens to incoming connections to the socket.
 * It accepts new connections and starts a new client thread.
 */

void handle_client(int client_socket, std::string server_host, int server_port)
{

    SSL_CTX *sslctx = NULL;
    SSL *ssl_target = NULL;
    BIO *bio = NULL;
    int res = EXIT_FAILURE;
    int ret;
    size_t written, readbytes;
    int eof = 0;
    const SSL_METHOD *ssl_method = TLS_client_method();
    int ssl_socket = -1;
    int err;
    std::string request;

    std::string response;

    std::cout << "handle_client connection" << std::endl;

    char buffer[4096];
    int bytes_received;

    // Receive the client's request
    bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received < 0)
    {
        perror("Error receiving data from client");
        close(client_socket);
        goto end;
    }

    // Parse the request to determine the host and port
    request = std::string(buffer, bytes_received);
    std::cout << "=============== Request =================" << std::endl;
    std::cout << request << std::endl;
    std::cout << "=====================================" << std::endl;

    /*
     * Create an SSL_CTX which we can use to create SSL objects from. We
     * want an SSL_CTX for creating clients so we use TLS_client_method()
     * here.
     */

    sslctx = SSL_CTX_new(ssl_method);
    if (sslctx == NULL)
    {
        printf("Failed to create the SSL_CTX\n");
        goto end;
    }

    /*
     * Configure the client to abort the handshake if certificate
     * verification fails. Virtually all clients should do this unless you
     * really know what you are doing.
     */
    SSL_CTX_set_verify(sslctx, SSL_VERIFY_PEER, NULL);

    /* Use the default trusted certificate store */
    if (!SSL_CTX_set_default_verify_paths(sslctx))
    {
        printf("Failed to set the default trusted certificate store\n");
        goto end;
    }

    /*
     * TLSv1.1 or earlier are deprecated by IETF and are generally to be
     * avoided if possible. We require a minimum TLS version of TLSv1.2.
     */
    if (!SSL_CTX_set_min_proto_version(sslctx, TLS1_2_VERSION))
    {
        printf("Failed to set the minimum TLS protocol version\n");
        goto end;
    }

    /* Create an SSL object to represent the TLS connection */
    ssl_target = SSL_new(sslctx);
    if (ssl_target == NULL)
    {
        printf("Failed to create the SSL object\n");
        goto end;
    }

    /*
     * Create the underlying transport socket/BIO and associate it with the
     * connection.
     */
    bio = create_socket_bio(server_host.c_str(), (std::to_string(server_port)).c_str());
    if (bio == NULL)
    {
        printf("Failed to crete the BIO\n");
        goto end;
    }
    SSL_set_bio(ssl_target, bio, bio);

    /*
     * Tell the server during the handshake which hostname we are attempting
     * to connect to in case the server supports multiple hosts.
     */
    if (!SSL_set_tlsext_host_name(ssl_target, server_host.c_str()))
    {
        printf("Failed to set the SNI hostname\n");
        goto end;
    }

    /*
     * Ensure we check during certificate verification that the server has
     * supplied a certificate for the hostname that we were expecting.
     * Virtually all clients should do this unless you really know what you
     * are doing.
     */
    if (!SSL_set1_host(ssl_target, server_host.c_str()))
    {
        printf("Failed to set the certificate verification hostname");
        goto end;
    }

    /* Do the handshake with the server */
    while ((ret = SSL_connect(ssl_target)) != 1)
    {
        if (handle_io_failure(ssl_target, ret) == 1)
            continue; /* Retry */
        printf("Failed to connect to server\n");
        goto end; /* Cannot retry: error */
    }

    printf("SSL connection using %s\n", SSL_get_cipher(ssl_target));

    std::cout << "Sending data to target server" << std::endl;

    /* Write an HTTP GET request to the peer */
    while (!SSL_write_ex(ssl_target, request.c_str(), strlen(request.c_str()), &written))
    {
        if (handle_io_failure(ssl_target, 0) == 1)
            continue; /* Retry */
        printf("Failed to write HTTP request\n");
        goto end; /* Cannot retry: error */
    }

    do
    {
        memset(buffer, 0, sizeof(buffer));
        /*
         * Get up to sizeof(buffer) bytes of the response. We keep reading until
         * the server closes the connection.
         */
        while (!eof && !SSL_read_ex(ssl_target, buffer, sizeof(buffer), &readbytes))
        {
            switch (handle_io_failure(ssl_target, 0))
            {
            case 1:
                continue; /* Retry */
            case 0:
                eof = 1;
                continue;
            case -1:
            default:
                printf("Failed reading remaining data\n");
                goto end; /* Cannot retry: error */
            }
        }
        /*
         * OpenSSL does not guarantee that the returned data is a string or
         * that it is NUL terminated so we use fwrite() to write the exact
         * number of bytes that we read. The data could be non-printable or
         * have NUL characters in the middle of it. For this simple example
         * we're going to print it to stdout anyway.
         */
        if (!eof)
        {
            // fwrite(buffer, 1, readbytes, stdout);
            std::string temp;
            temp.assign(buffer, readbytes);
            response += temp;
        }

    } while (!eof);
    /* In case the response didn't finish with a newline we add one now */
    printf("\n");

    std::cout << "Sending data to client : " << std::endl;
    std::cout << "=============== Response =================" << std::endl;
    std::cout << response << std::endl;
    std::cout << "==========================================" << std::endl;
    send(client_socket, response.c_str(), response.length(), 0);

    /*
     * The peer already shutdown gracefully (we know this because of the
     * SSL_ERROR_ZERO_RETURN (i.e. EOF) above). We should do the same back.
     */
    while ((ret = SSL_shutdown(ssl_target)) != 1)
    {
        if (ret < 0 && handle_io_failure(ssl_target, ret) == 1)
            continue; /* Retry */
        /*
         * ret == 0 is unexpected here because that means "we've sent a
         * close_notify and we're waiting for one back". But we already know
         * we got one from the peer because of the SSL_ERROR_ZERO_RETURN
         * (i.e. EOF) above.
         */
        printf("Error shutting down\n");
        goto end; /* Cannot retry: error */
    }

    /* Success! */
    res = EXIT_SUCCESS;

end:
    // Close both client and server sockets
    close(client_socket);
    // close(server_socket);
    /*
     * If something bad happened then we will dump the contents of the
     * OpenSSL error stack to stderr. There might be some useful diagnostic
     * information there.
     */
    if (res == EXIT_FAILURE)
        ERR_print_errors_fp(stderr);

    /*
     * Free the resources we allocated. We do not free the BIO object here
     * because ownership of it was immediately transferred to the SSL object
     * via SSL_set_bio(). The BIO will be freed when we free the SSL object.
     */
    SSL_free(ssl_target);
    SSL_CTX_free(sslctx);
    // return res;
}

void *ClickHouseSSLPipeline(CProxySocket *ptr, void *lptr)
{
    std::cout << "ClickHouseSSLPipeline::start" << std::endl;
    CLIENT_DATA clientData;
    memcpy(&clientData, lptr, sizeof(CLIENT_DATA));

    /**
     * Get the configuration data for the target database clusters ( eg. clickhouse )
     * Config given in config.xml
     */
    TARGET_ENDPOINT_CONFIG targetEndpointConfig = ptr->GetConfigValues();
    int services_count = targetEndpointConfig.services.size();
    int current_service_index = clientData.current_service_index % services_count;
    RESOLVED_SERVICE currentService = targetEndpointConfig.services[current_service_index];
    END_POINT *target_endpoint = new END_POINT{currentService.ipaddress, currentService.port, currentService.r_w, currentService.alias, currentService.reserved, "  "};
    if (target_endpoint == 0)
    {
        cout << "Failed to retrieve target database configuration. Exiting!" << endl;
        return 0;
    }
    cout << "Resolved (Target) Host: " << target_endpoint->ipaddress << endl
         << "Resolved (Target) Port: " << target_endpoint->port << endl;

    // Check if handler is defined
    CProxyHandler *proxy_handler = ptr->GetHandler();
    if (proxy_handler == 0)
    {
        cout << "The handler is not defined. Exiting!" << endl;
        return 0;
    }

    Socket *client_socket = (Socket *)clientData.client_socket;

    handle_client(client_socket->GetSocket(), target_endpoint->ipaddress, target_endpoint->port);
}
