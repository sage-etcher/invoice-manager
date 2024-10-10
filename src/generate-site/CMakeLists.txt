
project(invoice-generate-site VERSION 0.1 LANGUAGES C)

add_executable(invoice-generate-site main.c)


install(PROGRAMS invoice-generate-site DESTINATION bin)