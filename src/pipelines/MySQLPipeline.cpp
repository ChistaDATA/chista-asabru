#include "../config/ConfigSingleton.h"
#include "CClientSSLSocket.h"
#include "CClientSocket.h"
#include "CHttpParser.h"
#include "CMySQLHandler.h"
#include "CProtocolSocket.h"
#include "CProxySocket.h"
#include "Logger.h"
#include "Pipeline.h"
#include "ProtocolHelper.h"
#include "SSLSocket.h"
#include "Socket.h"
#include "SocketSelect.h"
#include "uuid/UuidGenerator.h"
#include <any>
#include <string>

/** @brief Reads a packet from the socket
 *
 * Reads a packet from the socket.
 * @param[in]  socket  the socket to read from
 * @return buffer the buffer with packet bytes
 */
std::string read_packet(Socket *socket) {
	// MySQL packet header is 4 bytes
	char header[4];
	socket->RecvBlocking(header, 4);
	// bytes may contain '\0'. so we need to explicitly set the length when calling std::string(...)
	std::string buffer = std::string(header, 4);
	uint32_t payload_length = readLEInt24(buffer, 0);
	char *payload_bytes = (char *)malloc(payload_length);
	socket->RecvBlocking(payload_bytes, payload_length);
	std::string payload = std::string(payload_bytes, payload_length);
	free(payload_bytes);
	buffer += payload;
	return buffer;
}

void write_packet(Socket *socket, std::string packet) { socket->SendBytes((char *)packet.c_str(), packet.size()); }

/** @brief Extract packets from buffer
 *
 * Sometimes a buffer might contain multiple packets. This function extracts the different packets from the buffer and returns a
 * vector of the packets
 * @param[in]  buffer  the buffer that we receive from upstream ( source dbs )
 * @param[in]  buffer_length  length of the buffer
 * @return packets vector of packets
 */
std::vector<std::string> extract_packets(std::string buffer, ssize_t buffer_length) {
	std::vector<std::string> packets;
	int pos = 0;
	while (pos < buffer_length) {
		uint32_t packet_length = readLEInt24(buffer, pos);
		packets.push_back(buffer.substr(pos, packet_length + 4));
		pos += 4; // length of packet header
		pos += packet_length;
	}
	return packets;
}

/** @brief Perform the MySQL protocol handshake
 *
 * Perform the MySQL protocol handshake
 * @param[in]  conn  a connection object with client and server sockets
 * @param[in]  exec_context   execution context
 * @return status whether the handshake was successful. (0 = success)
 */
// int MySQLHandshake(Socket **client_socket, Socket **target_socket, EXECUTION_CONTEXT *exec_context) {
int MySQLHandshake(Connection *conn, EXECUTION_CONTEXT *exec_context) {
	std::string packet = "";

	std::string ipaddress = std::any_cast<std::string>((*exec_context)["target_ipaddress"]);
	int port = std::any_cast<int>((*exec_context)["target_port"]);

	packet = read_packet(conn->target_socket);
	write_packet(conn->client_socket, packet);

	packet = read_packet(conn->client_socket);
	// extract client capabilities
	int start = 0;
	start += 4; // skipping 4 byte header
	// the first 4 bytes from client side is always client capabilities
	// when CLIENT_PROTOCOL_41 is supported
	uint32_t capabilities = readInt32(packet, start);
	if (!(capabilities & CLIENT_PROTOCOL_41)) {
		// CLIENT_PROTOCOL_41 is not supported
		LOG_ERROR("error during handshake. Unsupported protocol version")
		return -1;
	}
	write_packet(conn->target_socket, packet);
	(*exec_context)["capabilities"] = capabilities;
	if (capabilities & CLIENT_SSL) {
		LOG_INFO("SSL enabled")
		conn->client_socket = new SSLSocket((conn->client_socket)->GetSocket());
		conn->target_socket = new CClientSSLSocket((conn->target_socket)->GetSocket(), ipaddress, port);
		packet = read_packet(conn->client_socket);
		write_packet(conn->target_socket, packet);
	}

	packet = read_packet(conn->target_socket);
	write_packet(conn->client_socket, packet);

	LOG_INFO("Handshake complete")
	return 0;
}

void *MySQLPipeline(CProxySocket *ptr, void *lptr) {
	LOG_INFO("MySQLPipeline::start");
	CLIENT_DATA clientData;
	memcpy(&clientData, lptr, sizeof(CLIENT_DATA));

	// Retrieve the load balancer class
	auto loadBalancer = ptr->loadBalancer;

	// Check if handler is defined
	CProxyHandler *proxy_handler = ptr->GetHandler();
	if (proxy_handler == nullptr) {
		std::cout << "The handler is not defined. Exiting!" << std::endl;
		return nullptr;
	}

	RESOLVED_SERVICE currentService = loadBalancer->requestServer();
	END_POINT target_endpoint = END_POINT{currentService.ipaddress, currentService.port,	 currentService.r_w,
										  currentService.alias,		currentService.reserved, "  "};
	LOG_INFO("Resolved (Target) Host: " + target_endpoint.ipaddress);
	LOG_INFO("Resolved (Target) Port: " + std::to_string(target_endpoint.port));

	Connection conn = {
		.client_socket = (Socket *)clientData.client_socket,
		.target_socket = new CClientSocket(target_endpoint.ipaddress, target_endpoint.port),
	};

	EXECUTION_CONTEXT exec_context;
	exec_context["target_host"] = target_endpoint.ipaddress + ":" + std::to_string(target_endpoint.port);
	exec_context["target_ipaddress"] = target_endpoint.ipaddress;
	exec_context["target_port"] = target_endpoint.port;

	std::string correlation_id;

	ProtocolHelper::SetReadTimeOut(conn.client_socket->GetSocket(), 1);
	ProtocolHelper::SetKeepAlive(conn.client_socket->GetSocket(), 1);
	ProtocolHelper::SetReadTimeOut(conn.target_socket->GetSocket(), 1);
	ProtocolHelper::SetKeepAlive(conn.target_socket->GetSocket(), 1);

	bool has_error = false;
	try {
		// Perform the mysql protocol handshake
		if (MySQLHandshake(&conn, &exec_context) != 0) {
			has_error = true;
			LOG_ERROR("error occurred during handshake");
		}
	} catch (std::exception &e) {
		has_error = true;
		LOG_ERROR(e.what());
		LOG_ERROR("error occurred during handshake");
	}

	while (!has_error) {
		SocketSelect *sel;
		try {
			sel = new SocketSelect(conn.client_socket, conn.target_socket, NonBlockingSocket);
		} catch (std::exception &e) {
			LOG_ERROR(e.what());
			LOG_ERROR("error occurred while creating socket select ");
		}

		try {
			if (sel->Readable(conn.client_socket)) {
				LOG_INFO("client socket is readable, reading bytes : ");
				std::string bytes = conn.client_socket->ReceiveBytes();

				if (!bytes.empty()) {
					for (const std::string packet : extract_packets(bytes, bytes.length())) {
						LOG_INFO("Calling Proxy Upstream Handler..");
						std::string response = proxy_handler->HandleUpstreamData(packet, packet.size(), &exec_context);
						conn.target_socket->SendBytes((char *)response.c_str(), response.size());
					}
				}

				if (bytes.empty())
					has_error = true;
			}
		} catch (std::exception &e) {
			LOG_ERROR("Error while sending to target " + std::string(e.what()));
			has_error = true;
		}

		try {
			if (sel->Readable(conn.target_socket)) {
				LOG_INFO("target socket is readable, reading bytes : ");
				std::string bytes = conn.target_socket->ReceiveBytes();

				if (!bytes.empty()) {
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

		if (has_error) {
			// Delete Select from memory
			delete sel;
		}
	}

	// Close the client socket
	delete conn.client_socket;

	// Close the server socket
	delete conn.target_socket;

	LOG_INFO("MySQLPipeline::end");
#ifdef WINDOWS_OS
	return 0;
#else
	return nullptr;
#endif

	return 0;
}
