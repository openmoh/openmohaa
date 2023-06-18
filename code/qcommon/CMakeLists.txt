cmake_minimum_required(VERSION 3.5)
project(qcommon)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.13.0")
	cmake_policy(SET CMP0076 NEW)
endif()

# Shared source files for modules
set(SOURCES_SHARED
	"${CMAKE_SOURCE_DIR}/code/qcommon/class.cpp"
	"${CMAKE_SOURCE_DIR}/code/qcommon/con_set.cpp"
	"${CMAKE_SOURCE_DIR}/code/qcommon/con_timer.cpp"
	"${CMAKE_SOURCE_DIR}/code/qcommon/listener.cpp"
	"${CMAKE_SOURCE_DIR}/code/qcommon/lz77.cpp"
	"${CMAKE_SOURCE_DIR}/code/qcommon/mem_blockalloc.cpp"
	"${CMAKE_SOURCE_DIR}/code/qcommon/mem_tempalloc.cpp"
	"${CMAKE_SOURCE_DIR}/code/qcommon/q_math.c"
	"${CMAKE_SOURCE_DIR}/code/qcommon/q_shared.c"
	"${CMAKE_SOURCE_DIR}/code/qcommon/script.cpp"
	"${CMAKE_SOURCE_DIR}/code/qcommon/str.cpp"
	"${CMAKE_SOURCE_DIR}/code/script/scriptexception.cpp"
	"${CMAKE_SOURCE_DIR}/code/script/scriptvariable.cpp"
)

add_library(qcommon INTERFACE)
target_sources(qcommon INTERFACE ${SOURCES_SHARED})
target_compile_definitions(qcommon INTERFACE TARGET_GAME_TYPE=${TARGET_GAME_TYPE})
target_include_directories(qcommon INTERFACE "../qcommon" "../script")
target_compile_features(qcommon INTERFACE cxx_nullptr)
target_compile_features(qcommon INTERFACE c_variadic_macros)

# Source files for standalone executable
set(SOURCES_COMMON
	"${CMAKE_SOURCE_DIR}/code/qcommon/alias.c"
	"${CMAKE_SOURCE_DIR}/code/qcommon/cm_fencemask.c"
	"${CMAKE_SOURCE_DIR}/code/qcommon/cm_load.c"
	"${CMAKE_SOURCE_DIR}/code/qcommon/cm_patch.c"
	"${CMAKE_SOURCE_DIR}/code/qcommon/cm_polylib.c"
	"${CMAKE_SOURCE_DIR}/code/qcommon/cm_terrain.c"
	"${CMAKE_SOURCE_DIR}/code/qcommon/cm_test.c"
	"${CMAKE_SOURCE_DIR}/code/qcommon/cm_trace.c"
	"${CMAKE_SOURCE_DIR}/code/qcommon/cm_trace_lbd.cpp"
	"${CMAKE_SOURCE_DIR}/code/qcommon/cmd.c"
	"${CMAKE_SOURCE_DIR}/code/qcommon/common.cpp"
	"${CMAKE_SOURCE_DIR}/code/qcommon/crc.c"
	"${CMAKE_SOURCE_DIR}/code/qcommon/cvar.c"
	"${CMAKE_SOURCE_DIR}/code/qcommon/files.cpp"
	"${CMAKE_SOURCE_DIR}/code/qcommon/huffman.cpp"
	"${CMAKE_SOURCE_DIR}/code/qcommon/md4.c"
	"${CMAKE_SOURCE_DIR}/code/qcommon/md5.c"
	"${CMAKE_SOURCE_DIR}/code/qcommon/memory.c"
	"${CMAKE_SOURCE_DIR}/code/qcommon/msg.cpp"
	"${CMAKE_SOURCE_DIR}/code/qcommon/net_chan.c"
	"${CMAKE_SOURCE_DIR}/code/qcommon/net_ip.c"
	"${CMAKE_SOURCE_DIR}/code/qcommon/q_math.c"
	"${CMAKE_SOURCE_DIR}/code/qcommon/q_shared.c"
	"${CMAKE_SOURCE_DIR}/code/qcommon/unzip.c"
	# Gamespy
	"${CMAKE_SOURCE_DIR}/code/gamespy/sv_gamespy.c"
	"${CMAKE_SOURCE_DIR}/code/gamespy/sv_gqueryreporting.c"
)

add_subdirectory("../skeletor" "./skeletor")
add_subdirectory("../tiki" "./tiki")

add_library(qcommon_standalone INTERFACE)
target_sources(qcommon_standalone INTERFACE ${SOURCES_COMMON})
target_compile_definitions(qcommon_standalone INTERFACE APP_MODULE)
target_compile_features(qcommon_standalone INTERFACE cxx_nullptr)
target_compile_features(qcommon_standalone INTERFACE c_variadic_macros)
target_link_libraries(qcommon_standalone INTERFACE omohtiki omohskeletor)