
cmake_minimum_required(VERSION 3.14)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
#set(BUILD_MODE "x64" CACHE STRING "Build mode: x64 or Win32")

add_subdirectory(logging)
add_subdirectory(extra-fileio)
add_subdirectory(extra-string)
add_subdirectory(database)

add_subdirectory(invoice-update-db)
#add_subdirectory(invoice-gen-site)

#add_subdirectory(testing)

if (WIN32)
    add_subdirectory(windows-scripts)
else()
    add_subdirectory(gnu-scripts)
endif()


# end of file