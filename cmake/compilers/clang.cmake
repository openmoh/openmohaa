# Clang compiler specific settings

if(NOT CMAKE_C_COMPILER_ID STREQUAL "Clang")
    return()
endif()

# Ignore warnings for code like 'assert("Assert string")'
add_compile_options(-Wno-pointer-bool-conversion)
