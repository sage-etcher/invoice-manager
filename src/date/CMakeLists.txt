
add_library(date
    date.c
)

target_include_directories(date
    PUBLIC
    "${PROJECT_SOURCE_DIR}/src/include"
)

target_compile_definitions(date
    PUBLIC
    PUBLIC_DATE
    PRIVATE
    PRIVATE_DATE
)


# end of file