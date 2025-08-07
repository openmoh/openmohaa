include_guard(GLOBAL)

function(find_include_dirs OUT_VAR)
    set(SOURCES ${ARGN})

    # Get top most common directory prefix for all source files
    set(COMMON_PATH "")
    foreach(FILE IN LISTS SOURCES)
        get_filename_component(DIR ${FILE} DIRECTORY)
        file(REAL_PATH ${DIR} DIR)
        if(COMMON_PATH STREQUAL "")
            set(COMMON_PATH ${DIR})
        else()
            string(LENGTH ${COMMON_PATH} PREFIX_LEN)
            while(NOT ${DIR} MATCHES "^${COMMON_PATH}(/|$)" AND PREFIX_LEN GREATER 0)
                string(SUBSTRING ${COMMON_PATH} 0 ${PREFIX_LEN} COMMON_PATH)
                math(EXPR PREFIX_LEN "${PREFIX_LEN} - 1")
            endwhile()
        endif()
    endforeach()

    if(NOT IS_DIRECTORY ${COMMON_PATH})
        message(FATAL_ERROR "Could not determine common directory for source files")
    endif()

    # Recursively find directories that contain .h files under common directory
    file(GLOB_RECURSE HEADER_FILES ${COMMON_PATH}/*.h)
    set(INCLUDE_DIRS "")
    foreach(HEADER_FILE IN LISTS HEADER_FILES)
        get_filename_component(HEADER_DIR ${HEADER_FILE} DIRECTORY)
        list(APPEND INCLUDE_DIRS ${HEADER_DIR})
    endforeach()

    list(REMOVE_DUPLICATES INCLUDE_DIRS)

    set(${OUT_VAR} ${INCLUDE_DIRS} PARENT_SCOPE)
endfunction()
