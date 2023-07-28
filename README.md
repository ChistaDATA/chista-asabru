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

build the asabru app from  the root of the repository

```
mkdir build
cd build
cmake ..
make
```

### Useage

```
 ./Chista_Asabru <port>
```