
# cmake
cmake_minimum_required(VERSION 3.14)
project(logging VERSION 1.0 LANGUAGES C)

# build library
add_library (logging STATIC logging.c)
