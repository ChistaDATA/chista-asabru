cmake_minimum_required(VERSION 3.15)

project(Chista_Asabru VERSION 1.0)

include(ExternalProject)

# Enables generation of compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# we cannot analyse results without gcov
find_program(GCOV_PATH gcov)
if(NOT GCOV_PATH)
    message(FATAL_ERROR "Code coverage analysis requires gcov!")
endif()

add_subdirectory(src)

enable_testing()

if(GTEST_ENABLED)
    add_subdirectory(test)
endif()
