# ChistaData Database Proxy - chista-asabru

### Prerequisite

Make
`https://www.gnu.org/software/make`

CMake
`https://cmake.org/install/`

Install docker and docker compose
`https://docs.docker.com/engine/install/`
`https://docs.docker.com/compose/install/`
### Development setup
clone using 
```
git clone --recurse-submodules
````

To initialize clickhouse cluster
```
docker compose up -d
```

### Build

Build the static libraries 

```
sudo chmod u+x ./build_libs.sh
./build_libs.sh
```

Build the asabru app from  the root of the repository

```
mkdir build
cd build
cmake ..
make
```

```
./build_libs.sh; cd build; cmake ..; make; cd ..;
```

### Useage

```
 ./build/Chista_Asabru <port>
```

Run the clickhouse client in SSL mode
```
./clickhouse client --config <your-config-path>/clickhouse-client-ssl.xml --host 127.0.0.1 --port 9120
```