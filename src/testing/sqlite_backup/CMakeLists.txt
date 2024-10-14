
# cmake
cmake_minimum_required(VERSION 3.14)
project(sagestesting VERSION 0.1 LANGUAGES C)

find_package(SQlite3 REQUIRED)
add_executable(testbackup backup.c)

target_include_directories(testbackup PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
    "${SQLite3_INCLUDE_DIRS}"
)

target_link_libraries(testbackup PRIVATE
    invoice-logging-lib
    invoice-database-lib
    "${SQLite3_LIBRARIES}"
)

