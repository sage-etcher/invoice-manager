
project(invoice-gen-site VERSION 0.1 LANGUAGES C)

add_executable(invoice-gen-site)
target_sources(invoice-gen-site PRIVATE main.c)
target_link_libraries(invoice-gen-site PRIVATE -lwindows)

set_target_properties(invoice-gen-site PROPERTIES C_STANDARD 11)
set_target_properties(invoice-gen-site PROPERTIES C_STANDARD_REQUIRED ON)
set_target_properties(invoice-gen-site PROPERTIES C_EXTENSIONS OFF)

install(PROGRAMS invoice-gen-site DESTINATION bin)