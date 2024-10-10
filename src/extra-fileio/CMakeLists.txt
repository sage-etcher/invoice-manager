
# cmake
cmake_minimum_required(VERSION 3.14)
project(extra-fileio VERSION 1.0 LANGUAGES C)


# build library
add_library(extra-fileio STATIC extra-fileio.c)

target_include_directories(extra-fileio PRIVATE 
        "${PROJECT_BINARY_DIR}"
        "${CMAKE_SOURCE_DIR}/src/logging"
)

target_link_libraries(extra-fileio PRIVATE 
        "${CMAKE_BINARY_DIR}/src/logging/${CMAKE_BUILD_TYPE}/logging.lib"
)

