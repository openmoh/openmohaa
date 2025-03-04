cmake_minimum_required(VERSION 3.12)
project(platform)

### Platform-specific code
if(WIN32)
	set(SOURCES_PLATFORM_SPECIFIC
		"${CMAKE_SOURCE_DIR}/code/sys/con_log.c"
		"${CMAKE_SOURCE_DIR}/code/sys/sys_autoupdater.c"
		"${CMAKE_SOURCE_DIR}/code/sys/sys_main.c"
		"${CMAKE_SOURCE_DIR}/code/sys/new/sys_main_new.c"
		"${CMAKE_SOURCE_DIR}/code/sys/new/sys_win32_new.c"
		"${CMAKE_SOURCE_DIR}/code/sys/con_win32.c"
		"${CMAKE_SOURCE_DIR}/code/sys/sys_win32.c"
		"${CMAKE_SOURCE_DIR}/code/sys/win_resource.rc"
		# These are still used even they're prefixed 'win'
		"${CMAKE_SOURCE_DIR}/code/sys/win_bounds.cpp"
		"${CMAKE_SOURCE_DIR}/code/sys/win_localization.cpp"
	)
else()
	set(SOURCES_PLATFORM_SPECIFIC
		"${CMAKE_SOURCE_DIR}/code/sys/con_log.c"
		"${CMAKE_SOURCE_DIR}/code/sys/sys_autoupdater.c"
		"${CMAKE_SOURCE_DIR}/code/sys/sys_main.c"
		"${CMAKE_SOURCE_DIR}/code/sys/new/sys_main_new.c"
		"${CMAKE_SOURCE_DIR}/code/sys/new/sys_unix_new.c"
		"${CMAKE_SOURCE_DIR}/code/sys/con_tty.c"
		"${CMAKE_SOURCE_DIR}/code/sys/sys_unix.c"
		# These are still used even they're prefixed 'win'
		"${CMAKE_SOURCE_DIR}/code/sys/win_bounds.cpp"
		"${CMAKE_SOURCE_DIR}/code/sys/win_localization.cpp"
	)
endif()

if (APPLE)
	set(SOURCES_PLATFORM_SPECIFIC ${SOURCES_PLATFORM_SPECIFIC} "${CMAKE_SOURCE_DIR}/code/sys/sys_osx.m")
endif()

set(SOURCES_PLATFORM_COMMON
    "${CMAKE_SOURCE_DIR}/code/sys/sys_update_checker.cpp"
)

add_library(syslib INTERFACE)
target_sources(syslib INTERFACE ${SOURCES_PLATFORM_SPECIFIC} ${SOURCES_PLATFORM_COMMON})
target_compile_features(syslib INTERFACE cxx_nullptr cxx_std_17)
target_compile_features(syslib INTERFACE c_variadic_macros)
target_link_libraries(syslib INTERFACE qcommon)

find_package(CURL)

if (CURL_FOUND)
    target_link_libraries(syslib INTERFACE CURL::libcurl)
    target_compile_definitions(syslib INTERFACE HAS_LIBCURL=1)
else()
    message(WARNING "CURL is not available, update checking will not work")
endif()

if(WIN32)
	target_link_libraries(syslib INTERFACE wsock32 ws2_32)
	target_link_libraries(syslib INTERFACE winmm)
	target_link_libraries(syslib INTERFACE dbghelp)
elseif(UNIX)
	find_package(Threads)
	target_link_libraries(syslib INTERFACE ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
	# Check for libc
	if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
		execute_process(COMMAND ${CMAKE_C_COMPILER} -v
						OUTPUT_VARIABLE COMPILER_OUTPUT
						ERROR_VARIABLE COMPILER_OUTPUT
						OUTPUT_STRIP_TRAILING_WHITESPACE)

		if(COMPILER_OUTPUT MATCHES "musl|freebsd|openbsd|netbsd")
			target_link_libraries(syslib INTERFACE execinfo m)
		else()
			target_link_libraries(syslib INTERFACE m)
			if (NOT APPLE)
				# For when using GLIBC versions older than 2.34
				target_link_libraries(syslib INTERFACE rt)
			endif()
		endif()
	endif()
	target_link_libraries(syslib INTERFACE stdc++)
endif()

if (APPLE)
	find_library(COCOA_LIBRARY Cocoa)
	target_link_libraries(syslib INTERFACE ${COCOA_LIBRARY})
	target_link_libraries(syslib INTERFACE "-framework Foundation")
	target_link_libraries(syslib INTERFACE objc)
	set_target_properties(syslib PROPERTIES LINK_FLAGS "-Wl,-F/Library/Frameworks")
endif()