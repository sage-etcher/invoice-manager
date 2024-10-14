
# cmake
cmake_minimum_required(VERSION 3.14)
project(invoice-database VERSION 1.0 LANGUAGES C)

# build/link exectuable
add_library(invoice-database-lib STATIC database.c)

target_include_directories(invoice-database-lib
        PRIVATE
        "${PROJECT_BINARY_DIR}"
        "${CMAKE_SOURCE_DIR}/src"

        PUBLIC
        "${SQLite3_INCLUDE_DIRS}"
)

target_link_libraries(invoice-database-lib
        PRIVATE
        invoice-mystring-lib
        invoice-logging-lib

        PUBLIC
        "${SQLite3_LIBRARIES}"
)

