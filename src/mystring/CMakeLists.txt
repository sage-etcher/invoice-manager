
add_library(mystring
    mystring.c
)

target_link_libraries(mystring
    PRIVATE
    logging
)

target_include_directories(mystring
    PUBLIC
    "${PROJECT_SOURCE_DIR}/src/include"
)


# end of file