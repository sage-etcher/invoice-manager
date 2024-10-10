
# cmake
cmake_minimum_required(VERSION 3.14)
project(invoice-mystring VERSION 0.1 LANGUAGES C)


# build library
add_library(invoice-mystring-lib STATIC mystring.c)

target_include_directories(invoice-mystring-lib PRIVATE 
        "${PROJECT_BINARY_DIR}"
        "${CMAKE_SOURCE_DIR}/src"
)

target_link_libraries(invoice-mystring-lib PRIVATE 
        invoice-logging-lib
)

