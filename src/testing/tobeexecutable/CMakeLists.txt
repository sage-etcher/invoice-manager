
# cmake
cmake_minimum_required(VERSION 3.14)
project(sagestesting VERSION 0.1 LANGUAGES C)

# tobeexecutable 
add_executable(tobeexecutable tobeexecutable.c)

target_link_libraries(tobeexecutable PRIVATE 
    "${CMAKE_BINARY_DIR}/src/testing/tobelibrary/${CMAKE_BUILD_TYPE}/tobelibrary.lib"
) 

target_include_directories(tobeexecutable PRIVATE 
    "${CMAKE_SOURCE_DIR}/src/testing/tobelibrary"
)
