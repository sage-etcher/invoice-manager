
# cmake
cmake_minimum_required(VERSION 3.14)
project(invoice-date VERSION 1.0 LANGUAGES C)

# build/link exectuable
add_library(invoice-date-lib STATIC date.c)

