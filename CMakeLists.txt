
cmake_minimum_required(VERSION 3.14)
project(invoice-manager)

set(CMAKE_C_STANDARD 11)

set(CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}" "${CMAKE_MODULE_PATH}")
set(CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}" "${CMAKE_PREFIX_PATH}")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

option(BUILD_TESTS "Build tests" ON)

enable_testing()

add_subdirectory(externel)
add_subdirectory(src)
add_subdirectory(apps)
add_subdirectory(scripts)

if (BUILD_TESTS)
    add_subdirectory(tests)
endif ()



# end of file