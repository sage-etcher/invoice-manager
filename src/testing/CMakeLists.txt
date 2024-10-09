
# cmake
cmake_minimum_required(VERSION 3.14)
project(sagestesting VERSION 0.1 LANGUAGES C)

# build/link exectuable
add_executable(testgeneric)
target_sources(testgeneric PRIVATE msvc-generic.c)
set_target_properties(testgeneric PROPERTIES C_STANDARD 11)
set_target_properties(testgeneric PROPERTIES C_STANDARD_REQUIRED ON)
set_target_properties(testgeneric PROPERTIES C_EXTENSIONS OFF)

add_executable(testcast)
target_sources(testcast PRIVATE msvc-generic-cast.c)
set_target_properties(testcast PROPERTIES C_STANDARD 11)
set_target_properties(testcast PROPERTIES C_STANDARD_REQUIRED ON)
set_target_properties(testcast PROPERTIES C_EXTENSIONS OFF)
