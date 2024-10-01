
project(invoice-update-db VERSION 0.1 LANGUAGES C)

#set(CMAKE_MODULE_PATH "E:\\Libraries\\pcre2-10.44\\build\\out\\cmake")
find_package(pcre2 DIR "E:/Libraries/pcre2-10.44/build/out/cmake")

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