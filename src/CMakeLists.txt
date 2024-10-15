
cmake_minimum_required(VERSION 3.14)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
#set(BUILD_MODE "x64" CACHE STRING "Build mode: x64 or Win32")

if(MSVC)
    message("compiling for MSVC")
    add_compile_options(/W4)
else()
    message("compiling for non MSVC")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()


# find global libraries 
set(PCRE2_USE_STATIC_LIBS ON)
find_package(PCRE2 CONFIG COMPONENTS 8BIT REQUIRED)
find_package(SQLite3 REQUIRED)


# add subdirectories
add_subdirectory(date-lib)
add_subdirectory(logging-lib)
add_subdirectory(myfileio-lib)
add_subdirectory(mystring-lib)
add_subdirectory(database-lib)
add_subdirectory(hemlock-argparser-lib)

add_subdirectory(update-database)
add_subdirectory(generate-site)

add_subdirectory(testing)

if(WIN32)
    add_subdirectory(windows-scripts)
else()
    add_subdirectory(gnu-scripts)
endif()








# end of file