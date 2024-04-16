#include "../config/ConfigSingleton.h"
#include "CClientSSLSocket.h"
#include "CClientSocket.h"
#include "CHttpParser.h"
#include "CPostgreSQLHandler.h"
#include "CProtocolSocket.h"
#include "CProxySocket.h"
#include "Pipeline.h"
#include "ProtocolHelper.h"
#include "Socket.h"
#include "SocketSelect.h"
#include "uuid/UuidGenerator.h"
#include <utility>

/** @brief Reads specified number of bytes from socket
 *
 * Reads specified number of bytes from socket
 * @param[in]  socket  the socket from which bytes should be read
 * @param[in]  length  number of bytes to read
 * @return data  the data read from socket
 */
static std::string read_bytes(Socket *socket, uint64_t length) {
	char *data_bytes = (char *)malloc(length);
	socket->RecvBlocking(data_bytes, length);
	std::string data = std::string(data_bytes, length);
	free(data_bytes);
	return data;
}

/** @brief Writes bytes to socket
 *
 * Reads specified number of bytes from socket
 * @param[in]  socket  the socket to which bytes should be written
 * @param[in]  bytes   the data to be written
 */
static void write_bytes(Socket *socket, std::string bytes) { socket->SendBytes((char *)bytes.c_str(), bytes.size()); }

/** @brief handles ssl request during postgresql handshake
 *
 * Perform the PostgreSQL protocol handshake
 * @param[in]  conn  a connection object with client and server sockets
 */
static void handle_ssl_request(Connection *conn) {
	char server_resp = 0;
	conn->target_socket->RecvBlocking(&server_resp, 1);
	conn->client_socket->SendBytes(&server_resp, 1);
	if (server_resp == 'S') {
		LOG_INFO("SSL Enabled")
		conn->client_socket = new SSLSocket((conn->client_socket)->GetSocket());
		conn->target_socket = new CClientSSLSocket((conn->target_socket)->GetSocket(), conn->target_address, conn->target_port);
	} else {
		LOG_INFO("SSL Disabled")
	}
}

/** @brief Perform the PostgreSQL protocol handshake
 *
 * Perform the PostgreSQL protocol handshake
 * @param[in]  conn  a connection object with client and server sockets
 * @param[in]  exec_context   execution context
 * @return status whether the handshake was successful. (0 = success)
 */
int PostgreSQLHandshake(Connection *conn, EXECUTION_CONTEXT *exec_context) {
	LOG_INFO("Starting handshake")

	std::string packet_length_bytes = read_bytes(conn->client_socket, 4);
	write_bytes(conn->target_socket, packet_length_bytes);

	uint32_t packet_length = readBEInt32(packet_length_bytes, 0);
	uint32_t packet_body_length = packet_length - packet_length_bytes.length();
	std::string message_body_bytes = "";
	LOG_INFO("packet_length " + std::to_string(packet_length))
	switch (packet_length) {
	case 16: {
		LOG_INFO("Cancel Request")
		// Cancel request
		message_body_bytes = read_bytes(conn->client_socket, packet_body_length);
		write_bytes(conn->target_socket, message_body_bytes);
		return 0;
	}
	case 8: {
		// SSLRequest or GSSENCRequest
		LOG_INFO("SSLRequest / GSSENCRequest")
		message_body_bytes = read_bytes(conn->client_socket, packet_body_length);
		write_bytes(conn->target_socket, message_body_bytes);

		const uint32_t message_code = readBEInt32(message_body_bytes, 0);
		LOG_INFO("message_code " + std::to_string(message_code))
		if (message_code == RequestCodeSSL) {
			LOG_INFO("SSL Request")
			handle_ssl_request(conn);
		} else if (message_code == RequestCodeGSSENC) {
			LOG_INFO("GSSENC Request")
			throw "Not Implemented";
		}

		packet_length_bytes = read_bytes(conn->client_socket, 4);
		packet_length = readBEInt32(packet_length_bytes, 0);
		packet_body_length = packet_length - packet_length_bytes.length();
		message_body_bytes = read_bytes(conn->client_socket, packet_body_length);
		write_bytes(conn->target_socket, packet_length_bytes);
		break;
	}
	}
	// StartupMessage
	write_bytes(conn->target_socket, message_body_bytes);
	LOG_INFO("Handshake complete")
	return 0;
}

void *PostgreSQLPipeline(CProxySocket *ptr, void *lptr) {
	LOG_INFO("PostgreSQLPipeline::start");
	CLIENT_DATA clientData;
	memcpy(&clientData, lptr, sizeof(CLIENT_DATA));

	// Retrieve the load balancer class
	auto loadBalancer = ptr->loadBalancer;

	// Check if handler is defined
	CProxyHandler *proxy_handler = ptr->GetHandler();
	if (proxy_handler == nullptr) {
		LOG_ERROR("The handler is not defined. Exiting!")
		return nullptr;
	}

	RESOLVED_SERVICE currentService = loadBalancer->requestServer();
	END_POINT target_endpoint = END_POINT{currentService.ipaddress, currentService.port,	 currentService.r_w,
										  currentService.alias,		currentService.reserved, "  "};
	LOG_INFO("Resolved (Target) Host: " + target_endpoint.ipaddress);
	LOG_INFO("Resolved (Target) Port: " + std::to_string(target_endpoint.port));

	auto original_client_socket = (Socket *)clientData.client_socket;
	auto original_target_socket = new CClientSocket(target_endpoint.ipaddress, target_endpoint.port);

	Connection conn = {
		.client_socket = original_client_socket,
		.target_socket = original_target_socket,
		.target_address = target_endpoint.ipaddress,
		.target_port = target_endpoint.port,
	};

	EXECUTION_CONTEXT exec_context;
	exec_context["target_host"] = target_endpoint.ipaddress + ":" + std::to_string(target_endpoint.port);

	std::string correlation_id;
	std::chrono::time_point start_time = std::chrono::high_resolution_clock::now();
	bool latency_logged = true;

	ProtocolHelper::SetReadTimeOut(conn.client_socket->GetSocket(), 1);
	ProtocolHelper::SetKeepAlive(conn.client_socket->GetSocket(), 1);
	ProtocolHelper::SetReadTimeOut(conn.target_socket->GetSocket(), 1);
	ProtocolHelper::SetKeepAlive(conn.target_socket->GetSocket(), 1);

	bool has_error = false;
	try {
		// Perform the postgresql protocol handshake
		if (PostgreSQLHandshake(&conn, &exec_context) != 0) {
			has_error = true;
			LOG_ERROR("error occurred during handshake");
		}
	} catch (std::exception &e) {
		has_error = true;
		LOG_ERROR(e.what());
		LOG_ERROR("error occurred during handshake");
	}

	while (!has_error) {
		try {
			SocketSelect sel(conn.client_socket, conn.target_socket, NonBlockingSocket);
			try {
				if (sel.Readable(conn.client_socket)) {
					LOG_INFO("client socket is readable, reading bytes : ");
					std::string bytes = conn.client_socket->ReceiveBytes();

					if (!bytes.empty()) {
						if (latency_logged) {
							correlation_id = UuidGenerator::generateUuid();
							LOG_INFO("Correlation ID : " + correlation_id);
							exec_context["correlation_id"] = correlation_id;
							latency_logged = false;
						}
						start_time = std::chrono::high_resolution_clock::now();
						std::string response = proxy_handler->HandleUpstreamData(bytes, bytes.size(), &exec_context);
						conn.target_socket->SendBytes((char *)response.c_str(), response.size());
					}

					if (bytes.empty())
						has_error = true;
				}
			} catch (std::exception &e) {
				LOG_ERROR("Error while sending to target " + std::string(e.what()));
				has_error = true;
			}

			try {
				if (sel.Readable(conn.target_socket)) {
					LOG_INFO("target socket is readable, reading bytes : ");
					std::string bytes = conn.target_socket->ReceiveBytes();

					if (!bytes.empty()) {
						if (!latency_logged) {
							auto stop_time = std::chrono::high_resolution_clock::now();
							auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop_time - start_time);
							LOG_LATENCY(correlation_id, std::to_string(duration.count()) + "," + target_endpoint.ipaddress + ":" +
															std::to_string(target_endpoint.port));
							latency_logged = true;
						}
						LOG_INFO("Calling Proxy Downstream Handler..");
						std::string response = proxy_handler->HandleDownStreamData(bytes, bytes.size(), &exec_context);
						conn.client_socket->SendBytes((char *)response.c_str(), response.size());
					}

					if (bytes.empty())
						has_error = true;
				}
			} catch (std::exception &e) {
				LOG_ERROR("Error while sending to client " + std::string(e.what()));
				has_error = true;
			}
		} catch (std::exception &e) {
			LOG_ERROR(e.what());
			LOG_ERROR("error occurred while creating socket select ");
		}
	}

	// Close the client socket
	delete conn.client_socket;
	if (original_client_socket != conn.client_socket) {
		// in case of SSL, where the original socket is replaced with SSL one
		delete original_client_socket;
	}

	// Close the server socket
	delete conn.target_socket;
	if (original_target_socket != conn.target_socket) {
		// in case of SSL, where the original socket is replaced with SSL one
		delete original_target_socket;
	}

	LOG_INFO("PostgreSQLPipeline::end");
#ifdef WINDOWS_OS
	return 0;
#else
	return nullptr;
#endif

	return 0;
}
