# Apple Clang compiler specific settings

if(NOT CMAKE_C_COMPILER_ID STREQUAL "AppleClang")
    return()
endif()

set(CMAKE_INSTALL_RPATH "@executable_path")
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

if(CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "GNU")
	# Set the visibility to hidden on macOS to prevent shared libraries from
	# using functions in the executable
	# it's ok to hide them because the backtrace on macOS will still print the name of the functions
	add_compile_options(-fvisibility=hidden)
endif()
