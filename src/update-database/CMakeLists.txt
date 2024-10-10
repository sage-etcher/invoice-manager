
# cmake
cmake_minimum_required(VERSION 3.14)
project(invoice-update-database VERSION 0.1 LANGUAGES C)

# find libraries 
set(PCRE2_USE_STATIC_LIBS ON)
find_package(PCRE2 CONFIG COMPONENTS 8BIT REQUIRED)
find_package(SQLite3 REQUIRED)


# build/link exectuable
add_executable(invoice-update-database
        main.c
        parser.c
)

target_link_libraries(invoice-update-database PRIVATE 
        invoice-mystring-lib
        invoice-myfileio-lib
        invoice-logging-lib
        invoice-database-lib
        "${PCRE2_LIBRARIES}"
        "${SQLite3_LIBRARIES}"
)

target_include_directories(invoice-update-database PRIVATE 
        "${PROJECT_BINARY_DIR}"
        "${CMAKE_SOURCE_DIR}/src"
        "${PCRE2_INCLUDE_DIR}"
        "${SQLite3_INCLUDE_DIRS}"
)

# install
install(PROGRAMS invoice-update-database DESTINATION bin)