# Chista Asabru - Configuration

There are two types of servers that we can configure

1. Proxy Server - These accept connections and forward the requests to destination based on configuration.
2. Protocol Server - These accept connections and process requests. This is used to control the proxy.

There are several components that form the chista-asabru proxy servers.

1. Pipelines - These control or decide the transport mechanism of packets through the proxy.
2. Handlers - These are stateless lamda functions that decide what to do with the packet.

### Basic configuration

```
<clickhouse-proxy-v2>
	<protocol-server-config>
		<protocol-server protocol="HTTP">
			<protocol-port>8080</protocol-port>
			<pipeline>ProtocolPipeline</pipeline>
			<handler>CHttpProtocolHandler</handler>
		</protocol-server>
	</protocol-server-config>

	<CLUSTERS>
		<CLUSTER name ="cluster1">
			<END_POINTS>
			    ...
			    ... ENDPOINT goes here
			    ...
			</END_POINTS>
		</CLUSTER>
	<CLUSTERS>
<clickhouse-proxy-v2>
```

The `chista-asabru` proxy currently supports the following operation modes.

### HTTP/HTTPS/TCP-PlainText/TCP-TLS Passthrough

This configuration implies that the proxy will listen on port `9110` and it will forward
requests to target server `localhost:8123`. It will act as passthrough proxy and 
will support HTTP, HTTPS, TCP and TCP-TLS.
```
<END_POINT name="ch_http_node1">
    <READ_WRITE>yes</READ_WRITE>
    <PROXY_PORT>9110</PROXY_PORT>
    <PROTOCOL>CH_HTTP</PROTOCOL>
    <PIPELINE>ClickHousePipeline</PIPELINE>
    <HANDLER>CHttpHandler</HANDLER>
    <SERVICES>
        <SERVICE name="ch_http_service1">
            <HOST>localhost</HOST>
            <PORT>8123</PORT>
        </SERVICE>
    </SERVICES>
</END_POINT>
```

### HTTPS/TCP-TLS Termination

The proxy supports TLS Termination. It will serve a TLS certificate and connect via TLS to the 
client user. The TLS connection will be terminated at the proxy and 
then forward to the target HTTP/TCP endpoint.

This configuration implies that the proxy will listen on port `9130` and it will forward
requests to target server `localhost:8123`. It will act as HTTPS/TCP-TLS Termination proxy.

```
<END_POINT name="ch_http_node1">
    <READ_WRITE>yes</READ_WRITE>
    <PROXY_PORT>9130</PROXY_PORT>
    <PROTOCOL>CH_HTTP</PROTOCOL>
    <PIPELINE>ClickHouseTLSTerminatePipeline</PIPELINE>
    <HANDLER>CHttpHandler</HANDLER>
    <SERVICES>
        <SERVICE name="ch_http_service1">
            <HOST>localhost</HOST>
            <PORT>8123</PORT>
        </SERVICE>
    </SERVICES>
</END_POINT>
```

### HTTPS/TCP-TLS Exchange ( Forwarding SSL )

In this mode, the proxy will connect via TLS to the client and also to the target endpoint. The proxy
will be acting as man in the middle. It will serve a TLS certificate and connect via TLS to the
client user. It will connect to the target endpoint with a separate TLS connection. The TLS connection will be terminated at the proxy and
then forwarded to the target HTTP/TCP endpoint via the separate TLS connection.

This configuration implies that the proxy will listen on port `9130` and it will forward
requests to target server `localhost:8443`. 

```
<END_POINT name="ch_tls_http_node1">
    <READ_WRITE>no</READ_WRITE>
    <PROXY_PORT>9130</PROXY_PORT>
    <PROTOCOL>CH_TLS_HTTP</PROTOCOL>
    <PIPELINE>ClickHouseTLSExchangePipeline</PIPELINE>
    <HANDLER>CHttpsHandler</HANDLER>
    <SERVICES>
        <SERVICE name="ch_tls_http_service1">
            <HOST>localhost</HOST>
            <PORT>8443</PORT>
        </SERVICE>
    </SERVICES>
</END_POINT>
```