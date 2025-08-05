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
