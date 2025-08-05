include_guard(GLOBAL)

function(disable_warnings)
    set(SOURCES ${ARGN})

    foreach(FILE IN LISTS SOURCES)
        if(MSVC)
            # Annoyingly if you disable all warnings (/w) in combination with enabling
            # some warnings, which we and/or CMake inevitably do, this causes a
            # meta-warning D9025, so instead we have to individually disable them:
            set_source_files_properties(${FILE} PROPERTIES COMPILE_FLAGS
                "/wd4131 /wd4245 /wd4100 /wd4127 /wd4244 /wd4310 /wd4457 /wd4456 /wd4701 /wd4305 /wd4189 /wd4232")
        else()
            set_source_files_properties(${FILE} PROPERTIES COMPILE_FLAGS -w)
        endif()
    endforeach()
endfunction()
