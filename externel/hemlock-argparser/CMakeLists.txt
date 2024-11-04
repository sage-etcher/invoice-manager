
# cmake
cmake_minimum_required(VERSION 3.14)
project(hemlock-argparser 
        VERSION 0.1.0.0 
        LANGUAGES C 
        HOMEPAGE_URL https://github.com/SoftFauna/HEMLOCK.git)

# build/link exectuable
add_library(hemlock-argparser hemlock-argparser.c)

target_include_directories(hemlock-argparser
        PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
)


# end of file