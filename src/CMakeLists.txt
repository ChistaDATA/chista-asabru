cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
include(CMakePrintHelpers)

# Get the value of an environment variable and set it as a preprocessor definition
# Use $ENV{VAR_NAME} to access the value of the environment variable VAR_NAME
# and set it as a preprocessor definition
add_definitions(-DPLUGINS_FOLDER_PATH="$ENV{PLUGINS_FOLDER_PATH}")
add_definitions(-DCONFIG_FILE= "$ENV{CONFIG_FILE}")
add_definitions(-DCONFIG_FILE_URL= "$ENV{CONFIG_FILE_URL}")

# Set DEBUG preprocessor flag
add_compile_definitions(DEBUG)

# Find OpenSSL library
find_package(OpenSSL REQUIRED)

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

add_subdirectory(../lib/asabru-client/src ../lib/asabru-client)

include_directories(../lib/tinyxml2)
include_directories(../lib/libuv/include)
include_directories(../lib/asabru-parsers/src)
include_directories(../lib/asabru-handlers/src)

MACRO(HEADER_DIRECTORIES search_path return_list)
    FILE(GLOB_RECURSE new_list ${search_path})
    SET(dir_list "")
    FOREACH(file_path ${new_list})
        GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
        SET(dir_list ${dir_list} ${dir_path})
    ENDFOREACH()
    LIST(REMOVE_DUPLICATES dir_list)
    SET(${return_list} ${dir_list})
ENDMACRO()

HEADER_DIRECTORIES("../lib/asabru-commons/*.h" header_dir_list)
include_directories(${header_dir_list})
HEADER_DIRECTORIES("../lib/asabru-engine/*.h" header_dir_list)
include_directories(${header_dir_list})

#For the shared library:
set(PROJECT_LINK_LIBS
        libtinyxml2.a
        libasabru_commons.a
        libasabru_parsers.a
        libasabru_engine.a
        )

# Collect all source files
file(GLOB_RECURSE PIPELINE_FILES "./pipelines/*.cpp")
file(COPY public DESTINATION ..)
file(COPY config/config.xml DESTINATION ..)

add_executable(Chista_Asabru
        main.h
        main.cpp

        config/ConfigSingleton.cpp
        config/PipelineFactory.cpp

        ${PIPELINE_FILES})

target_link_libraries(
        Chista_Asabru
        OpenSSL::SSL
        ${Python_LIBRARIES}
        ${SQLite3_LIBRARIES}
        ${CMAKE_SOURCE_DIR}/lib/tinyxml2/build/libtinyxml2.a
        ${CMAKE_SOURCE_DIR}/lib/libuv/build/libuv.a
        ${CMAKE_SOURCE_DIR}/lib/asabru-commons/build/lib/libasabru_commons.a
        ${CMAKE_SOURCE_DIR}/lib/asabru-parsers/build/lib/libasabru_parsers.a
        ${CMAKE_SOURCE_DIR}/lib/asabru-engine/build/lib/libasabru_engine.a
        ${CMAKE_DL_LIBS}
)