# ChistaData Database Proxy - chista-asabru

### Prerequisite

Make
`https://www.gnu.org/software/make`

CMake
`https://cmake.org/install/`

Install docker and docker compose
`https://docs.docker.com/engine/install/`
`https://docs.docker.com/compose/install/`

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
export CONFIG_FILE=<path-to-config>;
export PLUGINS_FOLDER_PATH=<path-to-plugins-folder>;

Eg.
export CONFIG_FILE=/Users/midhundarvin/workplace/chistadata/chista-asabru/config/config.xml;
export PLUGINS_FOLDER_PATH=/Users/midhundarvin/workplace/chistadata/chista-asabru/lib/asabru-handlers/build;
```

##### Build the asabru app from  the root of the repository

```
mkdir build
cd build
cmake ..
make
```

### Docker Build

For MacOS M1 systems

```
docker build --platform linux/amd64 --no-cache --progress=plain -t asabru-proxy .
```

For Linux systems

```
docker build --no-cache --progress=plain -t asabru-proxy .
```

### Run Docker Image

```
docker run --platform=linux/amd64 -it -e CONFIG_FILE_URL='https://gist.githubusercontent.com/midhunadarvin/0e0b38927571816c73b72adfa92978bb/raw/ed9687cb8a3b10324e747e12f6fddf35e0effc6c/config.xml' asabru-proxy
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

