
cmake_minimum_required(VERSION 3.14)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
#set(BUILD_MODE "x64" CACHE STRING "Build mode: x64 or Win32")

add_subdirectory(logging-lib)
add_subdirectory(myfileio-lib)
add_subdirectory(mystring-lib)
add_subdirectory(database-lib)

add_subdirectory(update-database)

#add_subdirectory(invoice-gen-site)

#add_subdirectory(testing)

if (WIN32)
    add_subdirectory(windows-scripts)
else()
    add_subdirectory(gnu-scripts)
endif()


# end of file