cmake_minimum_required(VERSION 3.5)
project(qcommon)

cmake_policy(SET CMP0076 NEW)

# Shared source files for modules
set(SOURCES_SHARED
	"./class.cpp"
	"./con_set.cpp"
	"./con_timer.cpp"
	"./listener.cpp"
	"./lz77.cpp"
	"./mem_blockalloc.cpp"
	"./mem_tempalloc.cpp"
	"./q_math.c"
	"./q_shared.c"
	"./script.cpp"
	"./str.cpp"
	"../script/scriptexception.cpp"
	"../script/scriptvariable.cpp"
)

add_library(qcommon INTERFACE)
target_sources(qcommon PUBLIC ${SOURCES_SHARED})
target_compile_definitions(qcommon INTERFACE TARGET_GAME_TYPE=${TARGET_GAME_TYPE})
target_include_directories(qcommon INTERFACE "../qcommon" "../script")
target_compile_features(qcommon INTERFACE cxx_nullptr)
target_compile_features(qcommon INTERFACE c_variadic_macros)

# Source files for standalone executable
set(SOURCES_COMMON
	"./alias.c"
	"./cm_fencemask.c"
	"./cm_load.c"
	"./cm_patch.c"
	"./cm_polylib.c"
	"./cm_terrain.c"
	"./cm_test.c"
	"./cm_trace.c"
	"./cm_trace_lbd.cpp"
	"./cmd.c"
	"./common.cpp"
	"./crc.c"
	"./cvar.c"
	"./files.cpp"
	"./huffman.cpp"
	"./md4.c"
	"./md5.c"
	"./memory.c"
	"./msg.cpp"
	"./net_chan.c"
	"./net_ip.c"
	"./q_math.c"
	"./q_shared.c"
	"./unzip.c"
	# Gamespy
	"../gamespy/sv_gamespy.c"
	"../gamespy/sv_gqueryreporting.c"
)

add_subdirectory("../skeletor" "./skeletor")
add_subdirectory("../tiki" "./tiki")

add_library(qcommon_standalone INTERFACE)
target_sources(qcommon_standalone PUBLIC ${SOURCES_COMMON})
target_compile_definitions(qcommon_standalone INTERFACE APP_MODULE)
target_compile_features(qcommon_standalone INTERFACE cxx_nullptr)
target_compile_features(qcommon_standalone INTERFACE c_variadic_macros)
target_link_libraries(qcommon_standalone INTERFACE omohtiki omohskeletor)