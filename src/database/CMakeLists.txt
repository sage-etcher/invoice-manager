
# cmake
cmake_minimum_required(VERSION 3.14)
project(database VERSION 1.0 LANGUAGES C)

# find libraries 
find_package(SQLite3 REQUIRED)


# build/link exectuable
add_library(database STATIC database.c)

target_include_directories(database PRIVATE 
        "${PROJECT_BINARY_DIR}"
        "${CMAKE_SOURCE_DIR}/src/extra-string"
        "${CMAKE_SOURCE_DIR}/src/logging"
        "${SQLite3_INCLUDE_DIRS}"
)

target_link_libraries(database PRIVATE
        "${CMAKE_BINARY_DIR}/src/extra-string/${CMAKE_BUILD_TYPE}/extra-string.lib"
        "${CMAKE_BINARY_DIR}/src/logging/${CMAKE_BUILD_TYPE}/logging.lib"
        "${SQLite3_LIBRARIES}"
)

