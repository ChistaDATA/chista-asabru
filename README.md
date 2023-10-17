# ChistaData Database Proxy - chista-asabru

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

### Submodules

This `chista-asabru` project consists of the following sub-modules :

1. [asabru-engine](https://github.com/ChistaDATA/asabru-engine) : This is the core engine of project, which provides abstraction over platform depedent networking APIs.
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
```
export CONFIG_FILE_URL=<url-to-config>;
export CONFIG_FILE=<path-to-config>;
export PLUGINS_FOLDER_PATH=<path-to-plugins-folder>;
export PUBLIC_FOLDER_PATH=<path-to-public-folder>;

Eg.
export CONFIG_FILE_URL=https://pastebin.com/raw/RcyrYLMc;
export CONFIG_FILE_PATH=/tmp/config.xml;
export PLUGINS_FOLDER_PATH=/Users/midhundarvin/workplace/chistadata/chista-asabru/lib/asabru-handlers/build;
export PUBLIC_FOLDER_PATH=/Users/midhundarvin/workplace/chistadata/chista-asabru/bin/public;
```

##### Build the asabru app from  the root of the repository

```
mkdir build
cd build
cmake ..
make
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
docker run -it -e CONFIG_FILE_URL=<config-file-url> asabru-proxy
```

### Run with Docker compose

Update the values for the environment variables in the docker-compose.yaml file and run the following command.

```
docker compose up -d --build
```

### Useage

The proxy configurations are given in the `config.xml` file, we need to set the config file path location in ENV variables. Proxy will fetch the 
configurations from that location and start the server.

```
 ./build/Chista_Asabru
```

Run the clickhouse client in SSL mode
```
./clickhouse client --config <your-config-path>/clickhouse-client-ssl.xml --host 127.0.0.1 --port 9120
```
