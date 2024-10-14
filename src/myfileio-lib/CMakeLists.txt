
# cmake
cmake_minimum_required(VERSION 3.14)
project(invoice-myfileio VERSION 1.0 LANGUAGES C)

# build library
add_library(invoice-myfileio-lib STATIC myfileio.c)

target_include_directories(invoice-myfileio-lib PRIVATE 
        "${PROJECT_BINARY_DIR}"
        "${CMAKE_SOURCE_DIR}/src"
)

target_link_libraries(invoice-myfileio-lib PRIVATE 
        invoice-logging-lib
)

