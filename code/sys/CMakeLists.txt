cmake_minimum_required(VERSION 3.5)
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

add_library(syslib INTERFACE)
target_sources(syslib INTERFACE ${SOURCES_PLATFORM_SPECIFIC})
target_compile_features(syslib INTERFACE cxx_nullptr)
target_compile_features(syslib INTERFACE c_variadic_macros)
target_link_libraries(syslib INTERFACE qcommon)

if(WIN32)
	target_link_libraries(syslib INTERFACE wsock32 ws2_32)
	target_link_libraries(syslib INTERFACE winmm)
elseif(UNIX)
	find_package(Threads)
	target_link_libraries(syslib INTERFACE ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
	target_link_libraries(syslib INTERFACE m rt)
	target_link_libraries(syslib INTERFACE stdc++)
endif()