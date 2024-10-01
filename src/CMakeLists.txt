

add_subdirectory(invoice-update-db)
add_subdirectory(invoice-gen-site)

if (WIN32)
    add_subdirectory(windows-scripts)
else()
    add_subdirectory(gnu-scripts)
endif()


# end of file