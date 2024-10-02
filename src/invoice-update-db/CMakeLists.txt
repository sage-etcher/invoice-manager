
cmake_minimum_required(VERSION 3.12)
project(invoice-update-db VERSION 0.1 LANGUAGES C)

#set(PCRE2_DIR)
#set(PCRE2_ROOT "E:/Libraries/pcre2-10.44/build/out")
#set(PCRE2_INCLUDE_DIR "E:/Libraries/pcre2-10.44/build/out/include")
set(PCRE2_USE_STATIC_LIBS ON)
find_package(PCRE2 CONFIG COMPONENTS 8BIT)

add_executable(invoice-update-db)
target_sources(invoice-update-db PRIVATE 
        main.c 
        logging.c 
        extra-fileio.c)
target_link_libraries(invoice-update-db PRIVATE -lpcre2-8.a)

set_target_properties(invoice-update-db PROPERTIES C_STANDARD 11)
set_target_properties(invoice-update-db PROPERTIES C_STANDARD_REQUIRED ON)
set_target_properties(invoice-update-db PROPERTIES C_EXTENSIONS OFF)

install(PROGRAMS invoice-update-db DESTINATION bin)