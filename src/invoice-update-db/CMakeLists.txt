
# cmake
cmake_minimum_required(VERSION 3.14)
project(invoice-update-db VERSION 0.1 LANGUAGES C)

# find libraries 
set(PCRE2_USE_STATIC_LIBS ON)
find_package(PCRE2 CONFIG COMPONENTS 8BIT REQUIRED)
find_package(SQLite3 REQUIRED)


# build/link exectuable
add_executable(invoice-update-db
        main.c
        parser.c
)

target_include_directories(invoice-update-db PRIVATE 
        "${PROJECT_BINARY_DIR}"
        "${CMAKE_SOURCE_DIR}/src/logging"
        "${CMAKE_SOURCE_DIR}/src/database"
        "${CMAKE_SOURCE_DIR}/src/extra-string"
        "${CMAKE_SOURCE_DIR}/src/extra-fileio"
        "${SQLite3_INCLUDE_DIRS}"
        "${PCRE2_INCLUDE_DIR}"
)

target_link_libraries(invoice-update-db PRIVATE 
        "${CMAKE_BINARY_DIR}/src/logging/${CMAKE_BUILD_TYPE}/logging.lib"
        "${CMAKE_BINARY_DIR}/src/database/${CMAKE_BUILD_TYPE}/database.lib"
        "${CMAKE_BINARY_DIR}/src/extra-string/${CMAKE_BUILD_TYPE}/extra-string.lib"
        "${CMAKE_BINARY_DIR}/src/extra-fileio/${CMAKE_BUILD_TYPE}/extra-fileio.lib"
        "${PCRE2_LIBRARIES}"
        "${SQLite3_LIBRARIES}"
)

# install
install(PROGRAMS invoice-update-db DESTINATION bin)