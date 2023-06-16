cmake_minimum_required(VERSION 3.5)
project(platform)

cmake_policy(SET CMP0076 NEW)

### Platform-specific code
if(WIN32)
	set(SOURCES_PLATFORM_SPECIFIC
		"./con_log.c"
		"./sys_autoupdater.c"
		"./sys_main.c"
		"./new/sys_main_new.c"
		"./new/sys_win32_new.c"
		"./con_win32.c"
		"./sys_win32.c"
		"./win_resource.rc"
		# These are still used even they're prefixed 'win'
		"./win_bounds.cpp"
		"./win_localization.cpp"
	)
else()
	set(SOURCES_PLATFORM_SPECIFIC
		"./con_log.c"
		"./sys_autoupdater.c"
		"./sys_main.c"
		"./new/sys_main_new.c"
		"./new/sys_unix_new.c"
		"./con_tty.c"
		"./sys_unix.c"
		# These are still used even they're prefixed 'win'
		"./win_bounds.cpp"
		"./win_localization.cpp"
	)
endif()

add_library(syslib INTERFACE)
target_sources(syslib PUBLIC ${SOURCES_PLATFORM_SPECIFIC})
target_compile_definitions(syslib INTERFACE TARGET_GAME_TYPE=${TARGET_GAME_TYPE})
target_compile_features(syslib INTERFACE cxx_nullptr)
target_compile_features(syslib INTERFACE c_variadic_macros)
target_link_libraries(syslib INTERFACE qcommon)
