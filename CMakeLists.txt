cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_COMPILER "/usr/bin/g++")
set(CMAKE_C_COMPILER "/usr/bin/gcc")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

project(Chista_Asabru VERSION 1.0)

# Get the value of an environment variable and set it as a preprocessor definition
# Use $ENV{VAR_NAME} to access the value of the environment variable VAR_NAME
# and set it as a preprocessor definition
add_definitions(-DPLUGINS_FOLDER_PATH="$ENV{PLUGINS_FOLDER_PATH}")
add_definitions(-DCONFIG_FILE="$ENV{CONFIG_FILE}")

# set(OPENSSL_INCLUDE_DIR /opt/homebrew/Cellar/openssl@3/3.1.2/include)

# Find OpenSSL library
# find_package(OpenSSL REQUIRED)

# add_subdirectory(lib)
add_subdirectory(lib/tinyxml2)
add_subdirectory(lib/asabru-commons)
add_subdirectory(lib/asabru-parsers)
add_subdirectory(lib/asabru-engine)
include_directories(lib/tinyxml2)
include_directories(lib/libuv/include)
include_directories(lib/asabru-commons/src)
include_directories(lib/asabru-commons/src/config)
include_directories(lib/asabru-commons/src/util)
# include_directories(lib/asabru-commons/src/ssl)
include_directories(lib/asabru-parsers/src)
include_directories(lib/asabru-engine/src/socket)
include_directories(lib/asabru-engine/src/decoder)
include_directories(lib/asabru-engine/src/interface)
# Add the SSL library include directory
# include_directories(${OPENSSL_INCLUDE_DIR})

#For the shared library:
set ( PROJECT_LINK_LIBS libtinyxml2.a libasabru_commons.a libasabru_parsers.a libasabru_engine.a)

add_executable(Chista_Asabru
        first.cpp

        config/ConfigSingleton.cpp
        config/TypeFactory.cpp

        test/ProxyInfo.h 
        test/ProxyInfo.cpp

        pipelines/ClickHousePipeline.cpp
        pipelines/PostgreSQLPipeline.cpp
        pipelines/MySQLPipeline.cpp
        pipelines/PassthroughPipeline.cpp
)

target_link_libraries(
        Chista_Asabru 
                # OpenSSL::SSL
                ${CMAKE_SOURCE_DIR}/lib/tinyxml2/build/libtinyxml2.a
                ${CMAKE_SOURCE_DIR}/lib/libuv/build/libuv.a
                ${CMAKE_SOURCE_DIR}/lib/asabru-commons/build/lib/libasabru_commons.a
                ${CMAKE_SOURCE_DIR}/lib/asabru-parsers/build/lib/libasabru_parsers.a
                ${CMAKE_SOURCE_DIR}/lib/asabru-engine/build/lib/libasabru_engine.a
                ${CMAKE_DL_LIBS}
)
