
project(make-site VERSION 0.1 LANGUAGES C)

set(PCRE2_USE_STATIC_LIBS ON)
find_package(PCRE2 CONFIG COMPONENTS 8BIT REQUIRED)

configure_file(config.h.in config.h)

add_executable(make-site
    main.c
    settings.c
    cli-interface.c
)

target_link_libraries(make-site
    PRIVATE
    hemlock-argparser
    logging
    mystring
    invoice-db
    "${PCRE2_LIBRARIES}"
)

target_include_directories(make-site
    PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${PCRE2_INCLUDE_DIR}"
)

install(TARGETS make-site)


# end of file