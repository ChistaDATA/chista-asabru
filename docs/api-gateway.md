# Chista Asabru as API Gateway

Provide the following configuration to HTTP redirect requests based on hostname.

```
<END_POINT name="ch_wire_node1">
    <READ_WRITE>yes</READ_WRITE>
    <PROXY_PORT>80</PROXY_PORT>
    <PIPELINE>HttpRedirectPipeline</PIPELINE>
    <HANDLER>CHWirePTHandler</HANDLER>
    <PROTOCOL>CH_WIRE</PROTOCOL>
    <SERVICES>
        <SERVICE name="ui_service">
            <SOURCE_HOSTNAME>platform.local</SOURCE_HOSTNAME>
            <HOST>localhost</HOST>
            <PORT>8081</PORT>
        </SERVICE>
        <SERVICE name="metrics_service">
            <SOURCE_HOSTNAME>metrics-ui.platform.local</SOURCE_HOSTNAME>
            <HOST>localhost</HOST>
            <PORT>5001</PORT>
        </SERVICE>
        <SERVICE name="metrics_service">
            <SOURCE_HOSTNAME>oltp-olap-ui.platform.local</SOURCE_HOSTNAME>
            <HOST>localhost</HOST>
            <PORT>5002</PORT>
        </SERVICE>
    </SERVICES>
</END_POINT>
```