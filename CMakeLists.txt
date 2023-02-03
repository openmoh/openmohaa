cmake_minimum_required(VERSION 3.10)
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
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	add_compile_options(-Wno-comment)
	# Ignore warnings for code like 'assert("Assert string")'
	add_compile_options(-Wno-pointer-bool-conversion)
endif()

IF("${TARGET_ARCH}" STREQUAL "x86_64")
	set(TARGET_ARCH_SUFFIX "x64")
ELSEIF("${TARGET_ARCH}" STREQUAL "i386")
	set(TARGET_ARCH_SUFFIX "x86")
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

# Shared libraries

## Server game library
file(GLOB_RECURSE SOURCES_GAME "code/game/*.c" "code/game/*.cpp" "code/parser/*.cpp" "code/script/*.cpp")
set(SOURCES_GAME_LIB ${SOURCES_SHARED} ${SOURCES_GAME})

add_library(fgame SHARED ${SOURCES_GAME_LIB})
target_compile_definitions(fgame PRIVATE GAME_DLL ARCHIVE_SUPPORTED TARGET_GAME_TYPE=${TARGET_GAME_TYPE})
target_include_directories(fgame PUBLIC "code/qcommon" "code/script" "code/game")
target_include_directories(fgame PRIVATE "code/parser")
target_compile_features(fgame PUBLIC cxx_std_17)

set_target_properties(fgame PROPERTIES PREFIX "${TARGET_PLATFORM_PREFIX}")
set_target_properties(fgame PROPERTIES OUTPUT_NAME "game${TARGET_ARCH_SUFFIX}")
set_target_properties(fgame PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${TARGET_BASE_GAME}")
INSTALL(TARGETS fgame DESTINATION "${TARGET_BASE_GAME}")

## Client game library
## TODO! Build the cgame shared library

# Applications

## Code for all executables

### Platform-specific code
if(WIN32)
	set(SOURCES_PLATFORM_SPECIFIC
		"code/sys/con_win32.c"
		"code/sys/sys_win32.c"
		"code/sys/win_bounds.cpp"
		"code/sys/win_localization.cpp"
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
)

file(GLOB_RECURSE SOURCES_SKEL "code/tiki/*.cpp" "code/skeletor/*.cpp")

set(SOURCES_APP ${SOURCES_SHARED} ${SOURCES_COMMON} ${SOURCES_SKEL})

## Executables

### Client version
### TODO! Build the executable client (without the server part)

### Listen version
### TODO! Build the executable, full version (both client and server)

### Dedicated version
file(GLOB_RECURSE SOURCES_SERVER "code/server/*.c" "code/server/*.cpp")
set(SOURCES_SERVER_APP ${SOURCES_APP} ${SOURCES_SERVER})

add_executable(omohaaded ${SOURCES_SERVER_APP} ${SOURCES_PLATFORM_SPECIFIC} "code/null/null_client.c" "code/null/null_input.c" "code/null/null_snddma.c")
target_compile_definitions(omohaaded PRIVATE NO_SCRIPTENGINE DEDICATED TARGET_GAME_TYPE=${TARGET_GAME_TYPE})
target_compile_features(omohaaded PUBLIC cxx_std_17)
target_include_directories(omohaaded PUBLIC "code/qcommon" "code/script" "code/server" "code/SDL2/include")
set_target_properties(omohaaded PROPERTIES OUTPUT_NAME "omohaaded${TARGET_BASE_SUFFIX}${TARGET_ARCH_SUFFIX}")

if(WIN32)
	target_link_libraries(omohaaded wsock32 ws2_32)
	target_link_libraries(omohaaded winmm)
elseif(UNIX)
	target_link_libraries(omohaaded ${CMAKE_DL_LIBS})
endif()

INSTALL(TARGETS omohaaded DESTINATION "./")

