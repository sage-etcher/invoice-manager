
# cmake
cmake_minimum_required(VERSION 3.14)
project(extra-string VERSION 0.1 LANGUAGES C)


# build library
add_library(extra-string STATIC extra-string.c)

target_include_directories(extra-string PRIVATE 
        "${PROJECT_BINARY_DIR}"
        "${CMAKE_SOURCE_DIR}/src/logging"
)

target_link_libraries(extra-string PRIVATE 
        "${CMAKE_BINARY_DIR}/src/logging/${CMAKE_BUILD_TYPE}/logging.lib"
)

