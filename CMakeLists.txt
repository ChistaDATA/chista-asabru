cmake_minimum_required(VERSION 3.15)

project(Chista_Asabru VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_COMPILER "/usr/bin/g++")
set(CMAKE_C_COMPILER "/usr/bin/gcc")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
include(CMakePrintHelpers)

enable_testing()

add_subdirectory(src)
add_subdirectory(test)

