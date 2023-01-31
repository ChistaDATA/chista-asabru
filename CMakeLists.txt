cmake_minimum_required(VERSION 3.21)
project(Chista_Asabru)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(lib)
include_directories(lib/tinyxml2)

add_executable(Chista_Asabru
        first.cpp
        config/config.h
        config/config.cpp
        config/config_types.h
        config/IConfigEngine.h

        engine/socket/ServerSocket.h
        engine/socket/ServerSocket.cpp
        engine/socket/ClientSocket.h
        engine/socket/ClientSocket.cpp

        engine/decoder/ClickHouse_Wire.h
        engine/decoder/ClickHouse_Wire.cpp

        common/Utils.cpp common/Utils.h
        common/Utils.cpp

        handlers/CProtocolServer.h
        handlers/CProtocolServer.cpp

        test/ProxyInfo.h test/ProxyInfo.cpp

        pipelines/ClickHousePipeline.cpp
        pipelines/PostgreSQLPipeline.cpp
        pipelines/MySQLPipeline.cpp
        pipelines/PassthroughPipeline.cpp
        handlers/CHttpHandler.h
        handlers/CHttpHandler.cpp
        common/LineGrabber.h
        config/ConfigSingleton.h
        config/ConfigSingleton.cpp)

target_link_libraries(Chista_Asabru tinyxml2)
