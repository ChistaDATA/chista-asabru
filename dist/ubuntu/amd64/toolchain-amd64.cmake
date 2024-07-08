# toolchain-amd64.cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Specify the cross-compiler
set(CMAKE_C_COMPILER /usr/bin/x86_64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER /usr/bin/x86_64-linux-gnu-g++)
set(CMAKE_LINKER /usr/bin/x86_64-linux-gnu-ld)
set(CMAKE_AR /usr/bin/x86_64-linux-gnu-ar)
set(CMAKE_RANLIB /usr/bin/x86_64-linux-gnu-ranlib)
set(CMAKE_STRIP /usr/bin/x86_64-linux-gnu-strip)

# Set the sysroot path if you have a sysroot
set(CMAKE_SYSROOT /sysroot)

# Ensure the linker uses the sysroot
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Set the compiler flags for the target architecture
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
