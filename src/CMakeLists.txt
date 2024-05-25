cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(CMakePrintHelpers)

# Get the value of an environment variable and set it as a preprocessor definition
# Use $ENV{VAR_NAME} to access the value of the environment variable VAR_NAME
# and set it as a preprocessor definition
add_definitions(-DPLUGINS_FOLDER_PATH="$ENV{PLUGINS_FOLDER_PATH}")
add_definitions(-DCONFIG_FILE= "$ENV{CONFIG_FILE}")
add_definitions(-DCONFIG_FILE_URL= "$ENV{CONFIG_FILE_URL}")

# Set DEBUG preprocessor flag
add_compile_definitions(DEBUG)

# add dependencies
include(../cmake/CPM.cmake)

# Find OpenSSL library
set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)
if (OPENSSL_FOUND)
    message(STATUS "OpenSSL found: ${OpenSSL}")
else()
    message(FATAL_ERROR "OpenSSL not found.")
endif()

# Find Python
find_package(Python COMPONENTS Interpreter Development)
if (Python_FOUND)
    message(STATUS "Python found: ${Python_EXECUTABLE}")
else()
    message(FATAL_ERROR "Python not found.")
endif()
include_directories(${Python_INCLUDE_DIRS})

# Find SqlLite3
find_package (SQLite3 REQUIRED)
include_directories(${SQLite3_INCLUDE_DIRS})

#add_subdirectory(../lib/asabru-client/src ../lib/asabru-client)

if (ASABRU_COMMONS_BUILD STREQUAL "GIT_TAG")
    CPMAddPackage(
            NAME asabru_commons
            GITHUB_REPOSITORY ChistaDATA/asabru-commons
            GIT_TAG ${ASABRU_COMMONS_GIT_TAG}
    )
elseif (ASABRU_COMMONS_BUILD STREQUAL "LOCAL_DIR")
    # For development purpose
    CPMAddPackage(
            NAME asabru_commons
            SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../lib/asabru-commons/src
    )
else()
    CPMAddPackage(
            NAME asabru_commons
            GITHUB_REPOSITORY ChistaDATA/asabru-commons
            VERSION 0.0.3
    )
endif()

if (ASABRU_ENGINE_BUILD STREQUAL "GIT_TAG")
    CPMAddPackage(
            NAME asabru_engine
            GITHUB_REPOSITORY ChistaDATA/asabru-engine
            GIT_TAG ${ASABRU_ENGINE_GIT_TAG}
    )
elseif (ASABRU_ENGINE_BUILD STREQUAL "LOCAL_DIR")
    # For development purpose
    CPMAddPackage(
            NAME asabru_engine
            SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../lib/asabru-engine/src
    )
else()
    CPMAddPackage(
            NAME asabru_engine
            GITHUB_REPOSITORY ChistaDATA/asabru-engine
            VERSION 0.0.2
    )
endif()

if (ASABRU_PARSERS_BUILD STREQUAL "GIT_TAG")
    CPMAddPackage(
            NAME asabru_parsers
            GITHUB_REPOSITORY ChistaDATA/asabru-parsers
            GIT_TAG ${ASABRU_PARSERS_GIT_TAG}
    )
elseif (ASABRU_PARSERS_BUILD STREQUAL "LOCAL_DIR")
    # For development purpose
    CPMAddPackage(
            NAME asabru_parsers
            SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../lib/asabru-parsers/src
    )
else()
    CPMAddPackage(
            NAME asabru_parsers
            GITHUB_REPOSITORY ChistaDATA/asabru-parsers
            VERSION 0.0.1
    )
endif()

# Collect all source files
file(GLOB_RECURSE PIPELINE_FILES "./pipelines/*.cpp")
file(COPY ../lib/asabru-ui/build/ DESTINATION ../public)
file(COPY ../config.xml DESTINATION ..)

add_executable(Chista_Asabru
        main.h
        main.cpp

        config/ConfigParser.cpp
        config/ConfigSingleton.cpp
        config/PipelineFactory.cpp
        config/LoadBalancerFactory.cpp

        ${PIPELINE_FILES})

target_link_libraries(
        Chista_Asabru
        OpenSSL::SSL
        ${Python_LIBRARIES}
        ${SQLite3_LIBRARIES}
        asabru_commons
        asabru_engine
        asabru_parsers
        ${CMAKE_DL_LIBS}
)
