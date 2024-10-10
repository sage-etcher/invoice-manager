
cmake_minimum_required(VERSION 3.14)
project(invoice-manager VERSION 0.1)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
            "Default build type: Debug" FORCE)
endif()

add_subdirectory(./src)

