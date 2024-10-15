
# cmake
cmake_minimum_required(VERSION 3.14)
project(invoice-update-database VERSION 0.1 LANGUAGES C)

# configureation file
configure_file(config.h.in config.h)

# build/link exectuable
add_executable(invoice-update-database
        main.c
        cli-interface.c
        parser.c
        settings.c
)

target_link_libraries(invoice-update-database PRIVATE 
        invoice-date-lib
        invoice-mystring-lib
        invoice-myfileio-lib
        invoice-logging-lib
        invoice-database-lib
        hemlock-argparser-lib
        "${PCRE2_LIBRARIES}"
        "${SQLite3_LIBRARIES}"
)

target_include_directories(invoice-update-database PRIVATE 
        "${PROJECT_BINARY_DIR}"
        "${PROJECT_CURRENT_BINARY_DIR}"
        "${CMAKE_SOURCE_DIR}/src"
        "${PCRE2_INCLUDE_DIR}"
        "${SQLite3_INCLUDE_DIRS}"
)

# install
install(PROGRAMS invoice-update-database DESTINATION bin)