# GNU style (GCC/Clang) compiler specific settings

if(NOT CMAKE_C_COMPILER_ID STREQUAL "GNU" AND NOT CMAKE_C_COMPILER_ID MATCHES "^(Apple)?Clang$")
    return()
endif()

set(ASM_SOURCES
    ${SOURCE_DIR}/asm/snapvector.c
    ${SOURCE_DIR}/asm/ftola.c
)

add_compile_options(-Wall -Wimplicit
    -Wstrict-prototypes -Wformat=2  -Wformat-security
    -Wstrict-aliasing=2 -Wmissing-format-attribute
    -Wdisabled-optimization -Werror-implicit-function-declaration)

add_compile_options(-Wno-strict-aliasing
    -Wno-format-zero-length -Wno-format-nonliteral)

add_compile_options(-Wno-comment)
# Treat no return type as error
add_compile_options(-Werror=return-type)

set(ASM_SOURCES)

execute_process(COMMAND ${CMAKE_C_COMPILER} -v
                OUTPUT_VARIABLE COMPILER_OUTPUT
                ERROR_VARIABLE COMPILER_OUTPUT
                OUTPUT_STRIP_TRAILING_WHITESPACE)

if(COMPILER_OUTPUT MATCHES "musl|freebsd|openbsd|netbsd")
    list(APPEND COMMON_LIBRARIES execinfo m)
else()
    list(APPEND COMMON_LIBRARIES m)
   if (NOT APPLE)
        # For when using GLIBC versions older than 2.34
        list(APPEND COMMON_LIBRARIES rt)
    endif()
endif()