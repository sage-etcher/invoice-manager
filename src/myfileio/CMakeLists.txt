
add_library(myfileio
    myfileio.c
)

target_link_libraries(myfileio
    PRIVATE
    logging
)

target_include_directories(myfileio 
    PUBLIC
    "${PROJECT_SOURCE_DIR}/src/include"
)


# end of file