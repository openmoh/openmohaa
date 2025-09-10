get_directory_property(INSTALL_TARGETS DIRECTORY
    ${CMAKE_SOURCE_DIR} BUILDSYSTEM_TARGETS)

# Iterate over all the targets that have an INSTALL_DESTINATION
# property (set by set_output_dirs) and call install() on them
foreach(TARGET IN LISTS INSTALL_TARGETS)
    get_target_property(DESTINATION ${TARGET} INSTALL_DESTINATION)
    if(NOT DESTINATION)
        continue()
    endif()

    install(TARGETS ${TARGET}
        RUNTIME DESTINATION ${DESTINATION}
        LIBRARY DESTINATION ${DESTINATION}
        BUNDLE DESTINATION ${DESTINATION}
        ARCHIVE EXCLUDE_FROM_ALL)
endforeach()
