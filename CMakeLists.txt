cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 17)
set( CMAKE_CXX_COMPILER "/usr/bin/g++" )
set( CMAKE_C_COMPILER "/usr/bin/gcc" )

project(Chista_Asabru)
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
# include_directories(lib/asabru-commons/src/ssl)
include_directories(lib/asabru-parsers/src)
include_directories(lib/asabru-engine/src/socket)
include_directories(lib/asabru-engine/src/decoder)
# Add the SSL library include directory
# include_directories(${OPENSSL_INCLUDE_DIR})

#For the shared library:
set ( PROJECT_LINK_LIBS libtinyxml2.a libasabru_commons.a libasabru_parsers.a libasabru_engine.a)
link_directories( lib/asabru-commons/lib )
link_directories( lib/asabru-parsers/lib )
link_directories( lib/asabru-engine/lib )

add_executable(Chista_Asabru
        first.cpp
        config/config.h
        config/config.cpp
        config/config_types.h
        config/IConfigEngine.h

        # engine/socket/ServerSocket.h
        # engine/socket/ServerSocket.cpp
        # engine/socket/ClientSocket.h
        # engine/socket/ClientSocket.cpp
        # engine/decoder/ClickHouse_Wire.h
        # engine/decoder/ClickHouse_Wire.cpp

        # common/Utils.cpp common/Utils.h
        # common/Utils.cpp
        # common/LineGrabber.h

        handlers/CProtocolHandler.h
        
        handlers/CProxySocket.h 
        handlers/CProxySocket.cpp
        handlers/CProtocolSocket.h
        handlers/CProtocolSocket.cpp

        handlers/CHWirePTHandler.h
        handlers/CHWirePTHandler.cpp
        handlers/CMySQLHandler.h
        handlers/CMySQLHandler.cpp 
        handlers/CPostgreSQLHandler.h
        handlers/CPostgreSQLHandler.cpp
        handlers/PingHandler.h
        handlers/PingHandler.cpp


        test/ProxyInfo.h test/ProxyInfo.cpp

        # parsers/CHttpParser.h
        # parsers/ChttpParser.cpp

        pipelines/Pipeline.h
        pipelines/ClickHousePipeline.cpp
        pipelines/PostgreSQLPipeline.cpp
        pipelines/MySQLPipeline.cpp
        pipelines/PassthroughPipeline.cpp

        handlers/CHttpHandler.h
        handlers/CHttpHandler.cpp
        handlers/CHttpsHandler.h
        handlers/CHttpsHandler.cpp

        
        config/ConfigSingleton.h
        config/ConfigSingleton.cpp)

target_link_libraries(
        Chista_Asabru 
                # OpenSSL::SSL
                ${CMAKE_SOURCE_DIR}/lib/tinyxml2/build/libtinyxml2.a
                ${CMAKE_SOURCE_DIR}/lib/libuv/build/libuv.a
                ${CMAKE_SOURCE_DIR}/lib/asabru-commons/build/lib/libasabru_commons.a
                ${CMAKE_SOURCE_DIR}/lib/asabru-parsers/build/lib/libasabru_parsers.a
                ${CMAKE_SOURCE_DIR}/lib/asabru-engine/build/lib/libasabru_engine.a
)
