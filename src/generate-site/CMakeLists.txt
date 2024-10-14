
project(invoice-generate-site VERSION 0.1 LANGUAGES C)

add_executable(invoice-generate-site 
    main.c
)

target_link_libraries(invoice-generate-site PRIVATE
    invoice-database-lib
    invoice-logging-lib
    hemlock-argparser-lib
    "${SQlite3_LIBRARIES}" 
    "${PCRE2_LIBRARIES}"
)

target_include_directories(invoice-generate-site PRIVATE
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_CURRENT_BINARY_DIR}"
    "${CMAKE_SOURCE_DIR}/src"
    "${PCRE2_INCLUDE_DIR}"
    "${SQLite3_INCLUDE_DIRS}"
)


install(PROGRAMS invoice-generate-site DESTINATION bin)