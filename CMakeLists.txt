cmake_minimum_required(VERSION 3.5)
project(omohaaded)

include(TargetArch.cmake)

target_architecture(TARGET_ARCH)

if(TARGET_GAME_TYPE)
	if(TARGET_GAME_TYPE EQUAL 1)
		# Build for Team Assault (Spearhead)
		set(TARGET_BASE_SUFFIX "ta")
	elseif(TARGET_GAME_TYPE EQUAL 2)
		# Build for Team Tactics (Breakthrough)
		set(TARGET_BASE_SUFFIX "tt")
	else()
		set(TARGET_BASE_SUFFIX)
		message(SEND_ERROR "Invalid game type. Game type can be 0, 1 or 2")
	endif()
else()
	set(TARGET_BASE_SUFFIX)
	set(TARGET_GAME_TYPE 0)
endif()

set(TARGET_BASE_GAME "main${TARGET_BASE_SUFFIX}")

if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	add_compile_options(-Wno-comment)
	# Ignore warnings for code like 'assert("Assert string")'
	add_compile_options(-Wno-pointer-bool-conversion)
endif()

IF("${TARGET_ARCH}" STREQUAL "x86_64")
	set(TARGET_ARCH_SUFFIX "x86_64")
ELSEIF("${TARGET_ARCH}" STREQUAL "i386")
	set(TARGET_ARCH_SUFFIX "x86")
ELSEIF("${TARGET_ARCH}" STREQUAL "arm64")
        set(TARGET_ARCH_SUFFIX "arm64")
ELSEIF("${TARGET_ARCH}" MATCHES "arm(.*)")
	set(TARGET_ARCH_SUFFIX "arm")
ELSE()
	set(TARGET_ARCH_SUFFIX ${TARGET_ARCH})
ENDIF()

message(STATUS "Architecture detected: ${TARGET_ARCH}, suffix set to ${TARGET_ARCH_SUFFIX}")

IF(WIN32)
	set(TARGET_PLATFORM_PREFIX "")
	message(STATUS "Using Win32 naming convention")
ELSEIF(UNIX)
	set(TARGET_PLATFORM_PREFIX "")
	message(STATUS "Using Unix naming convention")
ELSE()
	set(TARGET_PLATFORM_PREFIX "")
ENDIF()

IF(CMAKE_BUILD_TYPE MATCHES Debug)
	set(TARGET_CONFIG_SUFFIX "-dbg")
	add_definitions(-D_DEBUG)
ELSE()
	set(TARGET_CONFIG_SUFFIX "")
ENDIF()

# Common stuff

## For modules

set(SOURCES_SHARED
	"code/qcommon/class.cpp"
	"code/qcommon/con_set.cpp"
	"code/qcommon/con_timer.cpp"
	"code/qcommon/listener.cpp"
	"code/qcommon/lz77.cpp"
	"code/qcommon/mem_blockalloc.cpp"
	"code/qcommon/mem_tempalloc.cpp"
	"code/qcommon/q_math.c"
	"code/qcommon/q_shared.c"
	"code/qcommon/script.cpp"
	"code/qcommon/str.cpp"
	"code/script/scriptexception.cpp"
	"code/script/scriptvariable.cpp"
)

file(GLOB_RECURSE SOURCES_BG "code/game/bg_*.cpp")

## For executable

### Platform-specific code
if(WIN32)
	set(SOURCES_PLATFORM_SPECIFIC
		"code/sys/con_win32.c"
		"code/sys/sys_win32.c"
		"code/sys/win_bounds.cpp"
		"code/sys/win_localization.cpp"
		"code/sys/win_resource.rc"
	)
else()
	set(SOURCES_PLATFORM_SPECIFIC
		"code/sys/con_tty.c"
		"code/sys/sys_unix.c"
		# These are still used even they're prefixed 'win'
		"code/sys/win_bounds.cpp"
		"code/sys/win_localization.cpp"
	)
endif()

### Common executable code
set(SOURCES_COMMON
	"code/qcommon/alias.c"
	"code/qcommon/cm_fencemask.c"
	"code/qcommon/cm_load.c"
	"code/qcommon/cm_patch.c"
	"code/qcommon/cm_polylib.c"
	"code/qcommon/cm_terrain.c"
	"code/qcommon/cm_test.c"
	"code/qcommon/cm_trace.c"
	"code/qcommon/cm_trace_lbd.cpp"
	"code/qcommon/cmd.c"
	"code/qcommon/common.cpp"
	"code/qcommon/crc.c"
	"code/qcommon/cvar.c"
	"code/qcommon/files.cpp"
	"code/qcommon/huffman.cpp"
	"code/qcommon/md4.c"
	"code/qcommon/md5.c"
	"code/qcommon/memory.c"
	"code/qcommon/msg.cpp"
	"code/qcommon/net_chan.c"
	"code/qcommon/net_ip.c"
	"code/qcommon/q_math.c"
	"code/qcommon/q_shared.c"
	"code/qcommon/tiki_main.cpp"
	"code/qcommon/tiki_script.cpp"
	"code/qcommon/unzip.c"
	# Main stuff
	"code/sys/sys_main.c"
	"code/sys/sys_autoupdater.c"
	"code/sys/con_log.c"
	# Gamespy
	"code/gamespy/sv_gamespy.c"
	"code/gamespy/sv_gqueryreporting.c"
)

file(GLOB_RECURSE SOURCES_SKEL "code/tiki/*.cpp" "code/skeletor/*.cpp")

set(SOURCES_APP ${SOURCES_SHARED} ${SOURCES_COMMON} ${SOURCES_SKEL})

# Targets

## Server

### Server game library
file(GLOB_RECURSE SOURCES_GAME "code/game/*.c" "code/game/*.cpp" "code/parser/*.cpp" "code/script/*.cpp")
set(SOURCES_GAME_LIB ${SOURCES_SHARED} ${SOURCES_GAME})

add_library(fgame SHARED ${SOURCES_GAME_LIB})
target_compile_definitions(fgame PRIVATE GAME_DLL ARCHIVE_SUPPORTED TARGET_GAME_TYPE=${TARGET_GAME_TYPE})
target_include_directories(fgame PUBLIC "code/qcommon" "code/script" "code/game")
target_include_directories(fgame PRIVATE "code/parser")
target_compile_features(fgame PUBLIC cxx_nullptr)
target_compile_features(fgame PUBLIC c_variadic_macros)

set_target_properties(fgame PROPERTIES PREFIX "${TARGET_PLATFORM_PREFIX}")
set_target_properties(fgame PROPERTIES OUTPUT_NAME "game${TARGET_ARCH_SUFFIX}${TARGET_CONFIG_SUFFIX}")
set_target_properties(fgame PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${TARGET_BASE_GAME}")
INSTALL(TARGETS fgame DESTINATION "${TARGET_BASE_GAME}")

include("code/gamespy/gcd.cmake")

### Server executable
file(GLOB_RECURSE SOURCES_SERVER "code/server/*.c" "code/server/*.cpp")
set(SOURCES_SERVER_APP ${SOURCES_APP} ${SOURCES_SERVER})

add_executable(omohaaded ${SOURCES_SERVER_APP} ${SOURCES_PLATFORM_SPECIFIC} "code/null/null_client.c" "code/null/null_input.c" "code/null/null_snddma.c")
target_compile_definitions(omohaaded PRIVATE NO_SCRIPTENGINE APP_MODULE DEDICATED TARGET_GAME_TYPE=${TARGET_GAME_TYPE})
target_compile_features(omohaaded PUBLIC cxx_nullptr)
target_compile_features(omohaaded PUBLIC c_variadic_macros)
# Gamespy dependency
add_dependencies(omohaaded gcd)
target_link_libraries(omohaaded PRIVATE gcd)
target_include_directories(omohaaded PUBLIC "code/qcommon" "code/script" "code/gamespy" "code/server" "code/SDL2/include")
set_target_properties(omohaaded PROPERTIES OUTPUT_NAME "omohaaded${TARGET_BASE_SUFFIX}${TARGET_ARCH_SUFFIX}${TARGET_CONFIG_SUFFIX}")

if(WIN32)
	if (MSVC)
		target_link_options(omohaaded PRIVATE "/MANIFEST:NO")
	endif()
	target_link_libraries(omohaaded PRIVATE wsock32 ws2_32)
	target_link_libraries(omohaaded PRIVATE winmm)
elseif(UNIX)
	find_package(Threads)
	target_link_libraries(omohaaded PRIVATE ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
	target_link_libraries(omohaaded PRIVATE m)
	target_link_libraries(omohaaded PRIVATE stdc++)
endif()

INSTALL(TARGETS omohaaded DESTINATION "./")

if (WITH_CLIENT)
	## Client

	### Client game library
	file(GLOB_RECURSE SOURCES_CGAME "code/cgame/*.c" "code/cgame/*.cpp")
	set(SOURCES_CGAME_LIB ${SOURCES_SHARED} ${SOURCES_CGAME} ${SOURCES_BG})

	add_library(cgame SHARED ${SOURCES_CGAME_LIB})
	target_compile_definitions(cgame PRIVATE CGAME_DLL NO_SCRIPTENGINE TARGET_GAME_TYPE=${TARGET_GAME_TYPE})
	target_include_directories(cgame PUBLIC "code/qcommon" "code/cgame" "code/script")
	target_compile_features(cgame PUBLIC cxx_nullptr)
	target_compile_features(cgame PUBLIC c_variadic_macros)

	set_target_properties(cgame PROPERTIES PREFIX "${TARGET_PLATFORM_PREFIX}")
	set_target_properties(cgame PROPERTIES OUTPUT_NAME "cgame${TARGET_ARCH_SUFFIX}${TARGET_CONFIG_SUFFIX}")
	set_target_properties(cgame PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${TARGET_BASE_GAME}")
	INSTALL(TARGETS cgame DESTINATION "${TARGET_BASE_GAME}")

	### Executable
#	file(GLOB_RECURSE SOURCES_CLIENT "code/client/*.c" "code/client/*.cpp")
#	set(SOURCES_CLIENT_APP ${SOURCES_APP} ${SOURCES_SERVER} ${SOURCES_CLIENT})
#
#	add_executable(openmohaa ${SOURCES_CLIENT_APP} ${SOURCES_PLATFORM_SPECIFIC})
#	target_compile_definitions(openmohaa PRIVATE NO_SCRIPTENGINE APP_MODULE TARGET_GAME_TYPE=${TARGET_GAME_TYPE})
#	target_compile_features(openmohaa PUBLIC cxx_nullptr)
#	target_compile_features(openmohaa PUBLIC c_variadic_macros)
#	# Gamespy dependency
#	add_dependencies(openmohaa gcd)
#	target_link_libraries(openmohaa PRIVATE gcd)
#	target_include_directories(openmohaa PUBLIC "code/qcommon" "code/script" "code/gamespy" "code/server" "code/client" "code/SDL2/include")
#	set_target_properties(openmohaa PROPERTIES OUTPUT_NAME "openmohaa{TARGET_BASE_SUFFIX}${TARGET_ARCH_SUFFIX}${TARGET_CONFIG_SUFFIX}")
#
#	if(WIN32)
#		if (MSVC)
#			target_link_options(openmohaa PRIVATE "/MANIFEST:NO")
#		endif()
#		target_link_libraries(openmohaa PRIVATE wsock32 ws2_32)
#		target_link_libraries(openmohaa PRIVATE winmm)
#	elseif(UNIX)
#		find_package(Threads)
#		target_link_libraries(openmohaa PRIVATE ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
#		target_link_libraries(openmohaa PRIVATE m)
#		target_link_libraries(openmohaa PRIVATE stdc++)
#	endif()
#
#	INSTALL(TARGETS openmohaa DESTINATION "./")
endif()
