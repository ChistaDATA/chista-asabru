#include "../config/ConfigSingleton.h"
#include "CClientSocket.h"
#include "CHttpParser.h"
#include "CMySQLHandler.h"
#include "CProtocolSocket.h"
#include "CProxySocket.h"
#include "Logger.h"
#include "Pipeline.h"
#include "ProtocolHelper.h"
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
 * @param[in]  client_socket  client socket
 * @param[in]  target_socket  server socket
 * @param[in]  exec_context   execution context
 * @return status whether the handshake was successful. (0 = success)
 */
int MySQLHandshake(Socket *client_socket, Socket *target_socket, EXECUTION_CONTEXT *exec_context) {
	std::string packet = "";

	packet = read_packet(target_socket);
	client_socket->SendBytes((char *)packet.c_str(), packet.size());

	packet = read_packet(client_socket);
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
	if (capabilities & CLIENT_SSL) {
		LOG_INFO("SSL enabled. Switching to passthrough mode")
		target_socket->SendBytes((char *)packet.c_str(), packet.size());
		(*exec_context)["ssl_enabled"] = true;
		return 0;
	}
	(*exec_context)["ssl_enabled"] = false;
	(*exec_context)["capabilities"] = capabilities;
	target_socket->SendBytes((char *)packet.c_str(), packet.size());

	packet = read_packet(target_socket);
	client_socket->SendBytes((char *)packet.c_str(), packet.size());

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

	Socket *client_socket = (Socket *)clientData.client_socket;
	CClientSocket *target_socket = new CClientSocket(target_endpoint.ipaddress, target_endpoint.port);

	EXECUTION_CONTEXT exec_context;
	exec_context["target_host"] = target_endpoint.ipaddress + ":" + std::to_string(target_endpoint.port);

	std::string correlation_id;
	bool data_sent = false;

	ProtocolHelper::SetReadTimeOut(client_socket->GetSocket(), 1);
	ProtocolHelper::SetKeepAlive(client_socket->GetSocket(), 1);
	ProtocolHelper::SetReadTimeOut(target_socket->GetSocket(), 1);
	ProtocolHelper::SetKeepAlive(target_socket->GetSocket(), 1);

	bool still_connected = true;
	try {
		// Perform the mysql protocol handshake
		if (MySQLHandshake(client_socket, target_socket, &exec_context) != 0) {
			still_connected = false;
			LOG_ERROR("error occurred during handshake");
		}
	} catch (std::exception &e) {
		still_connected = false;
		LOG_ERROR(e.what());
		LOG_ERROR("error occurred during handshake");
	}

	bool ssl_enabled = std::any_cast<bool>(exec_context["ssl_enabled"]);

	while (still_connected) {
		SocketSelect *sel;
		try {
			sel = new SocketSelect(client_socket, target_socket, NonBlockingSocket);
		} catch (std::exception &e) {
			LOG_ERROR(e.what());
			LOG_ERROR("error occurred while creating socket select ");
		}

		try {
			if (sel->Readable(client_socket)) {
				LOG_INFO("client socket is readable, reading bytes : ");
				std::string bytes = client_socket->ReceiveBytes();

				if (!bytes.empty()) {
					for (const std::string packet : extract_packets(bytes, bytes.length())) {
						LOG_INFO("Calling Proxy Upstream Handler..");
						std::string response = "";
						if (!ssl_enabled) {
							// packets can be read only if ssl is disabled
							response = proxy_handler->HandleUpstreamData(packet, packet.size(), &exec_context);
						} else {
							response = bytes;
						}
						target_socket->SendBytes((char *)response.c_str(), response.size());
					}
				}

				if (bytes.empty())
					still_connected = false;
			}
		} catch (std::exception &e) {
			LOG_ERROR("Error while sending to target " + std::string(e.what()));
			still_connected = false;
		}

		try {
			if (sel->Readable(target_socket)) {
				LOG_INFO("target socket is readable, reading bytes : ");
				std::string bytes = target_socket->ReceiveBytes();

				if (!bytes.empty()) {
					LOG_INFO("Calling Proxy Downstream Handler..");
					std::string response = "";
					if (!ssl_enabled) {
						// packets can be read only if ssl is disabled
						response = proxy_handler->HandleDownStreamData(bytes, bytes.size(), &exec_context);
					} else {
						response = bytes;
					}
					client_socket->SendBytes((char *)response.c_str(), response.size());
				}

				if (bytes.empty())
					still_connected = false;
			}
		} catch (std::exception &e) {
			LOG_ERROR("Error while sending to client " + std::string(e.what()));
			still_connected = false;
		}

		if (!still_connected) {
			// Delete Select from memory
			delete sel;
		}
	}

	// Close the client socket
	client_socket->Close();
	delete client_socket;

	// Close the server socket
	target_socket->Close();
	delete target_socket;

	LOG_INFO("MySQLPipeline::end");
#ifdef WINDOWS_OS
	return 0;
#else
	return nullptr;
#endif

	return 0;
}
