cmake_minimum_required(VERSION 3.12)

project(qcommon)

if(${CMAKE_VERSION} VERSION_GREATER "3.12")
	cmake_policy(SET CMP0076 NEW)
endif()

include("./q_version.cmake")

# Shared source files for modules
set(SOURCES_SHARED_UBER
	"${CMAKE_SOURCE_DIR}/code/qcommon/class.cpp"
	"${CMAKE_SOURCE_DIR}/code/qcommon/con_set.cpp"
	"${CMAKE_SOURCE_DIR}/code/qcommon/con_timer.cpp"
	"${CMAKE_SOURCE_DIR}/code/qcommon/lightclass.cpp"
	"${CMAKE_SOURCE_DIR}/code/qcommon/listener.cpp"
	"${CMAKE_SOURCE_DIR}/code/qcommon/lz77.cpp"
	"${CMAKE_SOURCE_DIR}/code/qcommon/mem_blockalloc.cpp"
	"${CMAKE_SOURCE_DIR}/code/qcommon/mem_tempalloc.cpp"
	"${CMAKE_SOURCE_DIR}/code/qcommon/script.cpp"
	"${CMAKE_SOURCE_DIR}/code/qcommon/str.cpp"
	"${CMAKE_SOURCE_DIR}/code/script/scriptexception.cpp"
	"${CMAKE_SOURCE_DIR}/code/script/scriptvariable.cpp"
)

add_library(qcommon_uber INTERFACE)
target_sources(qcommon_uber INTERFACE ${SOURCES_SHARED_UBER})
target_compile_features(qcommon_uber INTERFACE cxx_nullptr)
target_include_directories(qcommon_uber INTERFACE "../qcommon" "../script")

if(DEBUG_MEM_BLOCK)
	target_compile_definitions(qcommon_uber INTERFACE _DEBUG_MEMBLOCK=1)
endif()

set(SOURCES_SHARED
	"${CMAKE_SOURCE_DIR}/code/qcommon/puff.c"
	"${CMAKE_SOURCE_DIR}/code/qcommon/q_math.c"
	"${CMAKE_SOURCE_DIR}/code/qcommon/q_shared.c"
)

add_library(qcommon_shared INTERFACE)
target_sources(qcommon_shared INTERFACE ${SOURCES_SHARED})
target_compile_features(qcommon_shared INTERFACE c_variadic_macros)
target_include_directories(qcommon_shared INTERFACE "../qcommon")
target_link_libraries(qcommon_shared INTERFACE qcommon_version)

add_library(qcommon INTERFACE)
target_link_libraries(qcommon INTERFACE qcommon_shared qcommon_uber)

# Source files for standalone executable
set(SOURCES_COMMON
	"${CMAKE_SOURCE_DIR}/code/qcommon/alias.c"
	"${CMAKE_SOURCE_DIR}/code/qcommon/bg_compat.cpp"
	"${CMAKE_SOURCE_DIR}/code/qcommon/cm_fencemask.c"
	"${CMAKE_SOURCE_DIR}/code/qcommon/cm_load.c"
	"${CMAKE_SOURCE_DIR}/code/qcommon/cm_patch.c"
	"${CMAKE_SOURCE_DIR}/code/qcommon/cm_polylib.c"
	"${CMAKE_SOURCE_DIR}/code/qcommon/cm_terrain.c"
	"${CMAKE_SOURCE_DIR}/code/qcommon/cm_test.c"
	"${CMAKE_SOURCE_DIR}/code/qcommon/cm_trace.c"
	"${CMAKE_SOURCE_DIR}/code/qcommon/cm_trace_lbd.cpp"
	"${CMAKE_SOURCE_DIR}/code/qcommon/cm_trace_obfuscation.cpp"
	"${CMAKE_SOURCE_DIR}/code/qcommon/cmd.c"
	"${CMAKE_SOURCE_DIR}/code/qcommon/common.c"
	"${CMAKE_SOURCE_DIR}/code/qcommon/crc.c"
	"${CMAKE_SOURCE_DIR}/code/qcommon/cvar.c"
	"${CMAKE_SOURCE_DIR}/code/qcommon/files.cpp"
	"${CMAKE_SOURCE_DIR}/code/qcommon/ioapi.c"
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

if(DEBUG_DROP_ASSERT)
    target_compile_definitions(qcommon_standalone INTERFACE COM_ERROR_DROP_ASSERT)
endif()

target_compile_features(qcommon_standalone INTERFACE cxx_nullptr)
target_compile_features(qcommon_standalone INTERFACE c_variadic_macros)
target_link_libraries(qcommon_standalone INTERFACE omohtiki omohskeletor)

if(USE_INTERNAL_ZLIB)
	add_library(zlib INTERFACE)

	target_sources(zlib INTERFACE
		# zlib
		"${CMAKE_SOURCE_DIR}/code/zlib/adler32.c"
		"${CMAKE_SOURCE_DIR}/code/zlib/crc32.c"
		"${CMAKE_SOURCE_DIR}/code/zlib/inffast.c"
		"${CMAKE_SOURCE_DIR}/code/zlib/inflate.c"
		"${CMAKE_SOURCE_DIR}/code/zlib/inftrees.c"
		"${CMAKE_SOURCE_DIR}/code/zlib/zutil.c"
	)

	target_include_directories(zlib INTERFACE "${CMAKE_SOURCE_DIR}/code/zlib")
	target_link_libraries(qcommon_standalone INTERFACE zlib)
else()
	find_package(ZLIB REQUIRED)
	target_include_directories(qcommon INTERFACE ${ZLIB_INCLUDE_DIRS})
	target_link_libraries(qcommon_standalone INTERFACE ${ZLIB_LIBRARIES})
endif()
