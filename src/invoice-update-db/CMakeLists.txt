
# cmake
cmake_minimum_required(VERSION 3.12)
project(invoice-update-db VERSION 0.1 LANGUAGES C)

# find libraries 
set(PCRE2_USE_STATIC_LIBS ON)
find_package(PCRE2 CONFIG COMPONENTS 8BIT)

# build/link exectuable
add_executable(invoice-update-db)
target_sources(invoice-update-db PRIVATE 
        main.c 
        extra-fileio.c
        extra-string.c
        logging.c 
        parser.c
)
target_include_directories(invoice-update-db PRIVATE ${PCRE2_INCLUDE_DIR})
target_link_libraries(invoice-update-db PRIVATE ${PCRE2_LIBRARIES})

set_target_properties(invoice-update-db PROPERTIES C_STANDARD 11)
set_target_properties(invoice-update-db PROPERTIES C_STANDARD_REQUIRED ON)
set_target_properties(invoice-update-db PROPERTIES C_EXTENSIONS OFF)

# install
install(PROGRAMS invoice-update-db DESTINATION bin)