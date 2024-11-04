
add_library(logging
    logging.c
)

target_include_directories(logging
    PUBLIC
    "${PROJECT_SOURCE_DIR}/src/include"
)

target_compile_definitions(logging
    PUBLIC
    PUBLIC_LOG
    PRIVATE
    PRIVATE_LOG
)



# end of file