cmake_minimum_required(VERSION 3.15)
project(chista_scheduler VERSION 1.0)

# Enables generation of compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add compiler warnings
if (CMAKE_COMPILER_IS_GNUCXX)
	add_compile_options(-Wall)
endif ()

# Set DEBUG preprocessor flag
add_compile_definitions(DEBUG)

# add dependencies
include(cmake/CPM.cmake)

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
			SOURCE_DIR ${ASABRU_COMMONS_LOCAL_DIR}
	)
else()
	CPMAddPackage(
			NAME asabru_commons
			GITHUB_REPOSITORY ChistaDATA/asabru-commons
			VERSION 0.0.2
	)
endif()

add_subdirectory(lib/schedulers)
add_subdirectory(examples)

