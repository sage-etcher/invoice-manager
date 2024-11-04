
find_package(SQLite3 REQUIRED)

add_library(invoice-db
    invoice-db.c
    private/sqlite3-wrapper.c
)

target_link_libraries(invoice-db
    PUBLIC
    "${SQLite3_LIBRARIES}"
    PRIVATE
    date
    logging 
    mystring
)

target_include_directories(invoice-db
    PUBLIC
    "${SQLite3_INCLUDE_DIRS}"
    "${PROJECT_SOURCE_DIR}/src/include"
    PRIVATE
    private
)

# end of file