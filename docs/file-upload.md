# File Upload - Configuration

### Basic configuration

##### Set environment variable

Set `UPLOADS_FOLDER` environment variable.
```
export UPLOADS_FOLDER=/Users/midhunadarvin/Downloads
```

##### Add HttpFileUpload Protocol handler in the protocol configuration

```
<protocol-server-config>
    <protocol-server protocol="HTTP">
        <protocol-port>8080</protocol-port>
        <pipeline>ProtocolPipeline2</pipeline>
        <handler>CHttpProtocolHandler</handler>
        <routes>
            <route>
                <path>/upload</path>
                <method>POST</method>
                <request_handler>HttpFileUploadCommand</request_handler>
            </route>
        </routes>
    </protocol-server>
</protocol-server-config>
```

##### Make HTTP multipart form request to the above specified URL

```
curl --location 'http://localhost:8080/upload' \
--form 'image=@"/Users/midhundarvin/Pictures/098A1981.JPG"'
```