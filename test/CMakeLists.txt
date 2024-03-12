# Set the compiler options
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "--coverage -g -O0 -Wall -fprofile-arcs -ftest-coverage")
set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)

include(CMakePrintHelpers)
include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
)
FetchContent_MakeAvailable(googletest)

set(TEST_NAME ${PROJECT_NAME}_tests)
add_executable(${TEST_NAME} hello_test.cpp)
target_link_libraries(${TEST_NAME} gtest_main)

enable_testing()
include(GoogleTest)
gtest_discover_tests(${TEST_NAME})