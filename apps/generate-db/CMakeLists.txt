
project(generate-db VERSION 0.1 LANGUAGES C)

set(PCRE2_USE_STATIC_LIBS ON)
find_package(PCRE2 CONFIG COMPONENTS 8BIT REQUIRED)

configure_file(config.h.in config.h)

add_executable(generate-db
    main.c
    settings.c
    cli-interface.c
    parser.c
)

target_link_libraries(generate-db
    PRIVATE
    hemlock-argparser
    date
    logging
    myfileio
    mystring
    invoice-db
    "${PCRE2_LIBRARIES}"
)

target_include_directories(generate-db
    PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${PCRE2_INCLUDE_DIR}"
)

install(TARGETS generate-db)


# end of file