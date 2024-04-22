
## Development Platform

ChistaDATA Asabru Proxy is Developed using C++ and build system used is CMake from https://cmake.org/

### Prerequisite

Make
`https://www.gnu.org/software/make`

CMake
`https://cmake.org/install/`

Install docker and docker compose
`https://docs.docker.com/engine/install/`
`https://docs.docker.com/compose/install/`

Install curl on your system

https://everything.curl.dev/get

Run the following command to create a symlink

```
ln -s /usr/bin/curl /opt/bin/curl
```

Python ( minimum version 3.11 )

```
brew install python
```

Refer this to upgrade python version on mac : https://stackoverflow.com/questions/42896116/undeclared-identifier-pyunicode-decodefsdefault/77458658#77458658

### Submodules

This `chista-asabru` project consists of the following submodules :

1. [asabru-engine](https://github.com/ChistaDATA/asabru-engine) : This is the core engine of project, which provides abstraction over platform dependent networking APIs.
2. [asabru-commons](https://github.com/ChistaDATA/asabru-commons) : This repo contains the common utility classes for the chista database proxy.
3. [asabru-parsers](https://github.com/ChistaDATA/asabru-parsers) : This repo contains the code for the ChistaDATA database proxy parsers.
4. [asabru-handlers](https://github.com/ChistaDATA/asabru-handlers) : This repo contains the various handlers for the chista database proxy.
### Development setup

Clone the project using the following command. This will clone the submodules of the project and the external dependencies into the `lib` folder.
```
git clone --recurse-submodules
````

To initialize clickhouse cluster
```
docker compose up -d
```

### Build

##### Build the dependency libraries

```
sudo chmod u+x ./build_libs.sh
./build_libs.sh
```
##### Set environment variables
The asabru proxy configuration is based on a xml file. This file can be fetched from remote or from local

Remote configuration
```
export CONFIG_FILE_URL=<url-to-config>;
export CONFIG_FILE_PATH=<path-to-config>;

Eg.
export CONFIG_FILE_URL=https://pastebin.com/raw/RcyrYLMc;
export CONFIG_FILE_PATH=/tmp/config.xml;
```

Local configuration (skip setting the CONFIG_FILE_URL environment variable)
```
export CONFIG_FILE_PATH=<path-to-config>;

Eg.
export CONFIG_FILE_PATH=/Users/midhundarvin/workplace/chistadata/chista-asabru/build/config.xml
```

The plugins folder path is the directory where plugins have been build and the public folder path is the folder where HTTP protocol handler
serves static files.
```
export PLUGINS_FOLDER_PATH=<path-to-plugins-folder>;
export PUBLIC_FOLDER_PATH=<path-to-public-folder>;

Eg.
export PLUGINS_FOLDER_PATH=/Users/midhundarvin/workplace/chistadata/chista-asabru/lib/asabru-handlers/build;
export PUBLIC_FOLDER_PATH=/Users/midhundarvin/workplace/chistadata/chista-asabru/build/public;
```

Running Proxy using SSL clients requires setting the following SSL configuration.
Please see the [**documentation** ](https://tecadmin.net/step-by-step-guide-to-creating-self-signed-ssl-certificates/) on how to create a self-signed certificates for testing

```
export SSL_CERT_FILE_PATH = <path-to-SSL-certificate-directory>
export SSL_KEY_FILE_PATH = <path-to -SSL-key-directory>

If a passphrase is set , give the passphrase also

export SSL_CERT_PASSPHRASE = <passphrase>

Eg.
export SSL_CERT_FILE_PATH = /Users/josephabraham/ChistaV2/ssl_test1/cert.pem
export SSL_KEY_FILE_PATH = /Users/josephabraham/ChistaV2/ssl_test1/key.pem
export SSL_CERT_PASSPHRASE = mypassword
```
If you are connecting to a target server with a self-signed certificate, add this environment
variable
```
export SSL_VERIFY_CERT=false
```

**Logging**

The proxy currently supports logging to these outputs
- text file (FILE)
- sqlite database (DB)
- network tcp channel (NET)

For configuration, set the env variable `LOGGER_TYPE` to any of the above.
```
export LOGGER_TYPE=FILE
```

The text file will be generated in the build folder with the current date appended for the logs.

**Network Logging**

In the case of network logging, the logs are sent terminated by a new line. Set the following variables
to set host and port of the network log processor.

```
export NETWORK_LOGGER_HOST=localhost
export NETWORK_LOGGER_PORT=5170
```

[Fluent Bit](https://fluentbit.io/) is an open source network log processor. It can be started locally via the
`docker compose up` command.

##### Build the asabru app from the root of the repository

```
mkdir build
cd build
cmake ..
make
```

To use local directory files , use the command 
```
cmake -DASABRU_COMMONS_BUILD=LOCAL_DIR -DASABRU_ENGINE_BUILD=LOCAL_DIR -DASABRU_PARSERS_BUILD=LOCAL_DIR ..
```

##### Build in Debug mode

```
mkdir debug
cd debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

### Docker Build

If you want to build the proxy in a docker container. Follow the below steps.

```
docker build --no-cache --progress=plain -t asabru-proxy .
```

### Run Docker Image

```
docker run -it -d -p 9000-9999:9000-9999 -e CONFIG_FILE_URL=<config-file-url> -e CONFIG_FILE_PATH=<config-file-path> --name asabru-proxy asabru-proxy

Eg. docker run -it -d -p 9000-9999:9000-9999 -e CONFIG_FILE_URL=https://pastebin.com/raw/qAPt4KNQ -e CONFIG_FILE_PATH=/tmp/config.xml --name asabru-proxy asabru-proxy
```

### Run with Docker compose

Update the values for the environment variables in the docker-compose.yaml file and run the following command.

```
docker compose up -d --build
```

### Usage

The proxy configurations are given in the `config.xml` file, we need to set the config file path location in ENV variables. Proxy will fetch the
configurations from that location and start the server.

```
 ./build/Chista_Asabru
```

Run the clickhouse client in SSL mode
```
./clickhouse client --config <your-config-path>/clickhouse-client-ssl.xml --host 127.0.0.1 --port 9120

Eg. ./clickhouse client --config /Users/midhundarvin/software/openssl/clickhouse-client-ssl.xml --host 127.0.0.1 --port 9120
```

### ClickHouse Client Support

It supports the different client drivers for ClickHouse proxying

- Python Client
- Java Client (JDBC)
- DotNet Client

Sample Test Programs are available from [github-repo](https://github.com/ChistaDATA/chista-asabru/tree/main/automation_testing/ClickHouse)

