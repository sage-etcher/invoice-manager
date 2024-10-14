
cmake_minimum_required(VERSION 3.14)
project(invoice-manager VERSION 0.1)

# Default Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
            "Default build type: Debug" FORCE)
endif()

# subdirectories
add_subdirectory(./src)

