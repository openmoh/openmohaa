# GCC compiler specific settings

if(NOT CMAKE_C_COMPILER_ID STREQUAL "GNU")
    return()
endif()

# Add this option on gcc to prevent functions from having the STB_GNU_UNIQUE binding
# Otherwise, it would prevent libraries from being unloaded
# which will cause undefined behavior and crashes due to memory corruption
add_compile_options(-fno-gnu-unique)
