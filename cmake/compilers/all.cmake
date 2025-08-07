include(compilers/appleclang)
include(compilers/clang)
include(compilers/gcc)
include(compilers/gnu)
include(compilers/msvc)

set(CMAKE_DEBUG_POSTFIX "-dbg")

if(CMAKE_BUILD_TYPE MATCHES Debug)
	add_compile_definitions(_DEBUG)

	# NOTE: The following may mess up function importation
	#if(UNIX)
	#	# Enable all exports so all functions name can be seen during executable crash
	#	set(CMAKE_ENABLE_EXPORTS ON)
	#	message(STATUS "Enabling exports on Unix for backtrace")
	#endif()
else()
	# Non-debug builds
	add_compile_definitions(NDEBUG)
endif()
