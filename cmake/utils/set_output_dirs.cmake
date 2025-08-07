include_guard(GLOBAL)

function(set_output_dirs TARGET)
    list(REMOVE_AT ARGV 0)
    cmake_parse_arguments(ARG "" "" "SUBDIRECTORY" ${ARGV})

    if(CMAKE_CONFIGURATION_TYPES) # Multi-config
        set(CONFIGS ${CMAKE_CONFIGURATION_TYPES})
    else() # Single-config
        set(CONFIGS ${CMAKE_BUILD_TYPE})
    endif()

    foreach(CONFIG ${CONFIGS})
        string(TOUPPER ${CONFIG} CONFIG_UPPER)

        if(ARG_SUBDIRECTORY)
            set(OUT_DIR ${CMAKE_BINARY_DIR}/${CONFIG}/${ARG_SUBDIRECTORY})
        else()
            set(OUT_DIR ${CMAKE_BINARY_DIR}/${CONFIG})
        endif()

        set_target_properties(${TARGET} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${OUT_DIR}
            RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${OUT_DIR}
            ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${OUT_DIR})
    endforeach()
endfunction()
