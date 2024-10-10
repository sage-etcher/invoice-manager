
# cmake
cmake_minimum_required(VERSION 3.14)
project(invoice-logging VERSION 1.0 LANGUAGES C)

# build library
add_library (invoice-logging-lib STATIC logging.c)
