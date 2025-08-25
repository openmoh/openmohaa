include_guard(GLOBAL)

function(find_include_dirs OUT_VAR LIBRARY_DIR)
    # Recursively find directories that contain .h files under LIBRARY_DIR
    file(GLOB_RECURSE HEADER_FILES ${LIBRARY_DIR}/*.h)
    set(INCLUDE_DIRS "")
    foreach(HEADER_FILE IN LISTS HEADER_FILES)
        get_filename_component(HEADER_DIR ${HEADER_FILE} DIRECTORY)
        list(APPEND INCLUDE_DIRS ${HEADER_DIR})
    endforeach()

    list(REMOVE_DUPLICATES INCLUDE_DIRS)

    set(${OUT_VAR} ${INCLUDE_DIRS} PARENT_SCOPE)
endfunction()
