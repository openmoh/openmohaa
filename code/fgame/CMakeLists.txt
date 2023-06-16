cmake_minimum_required(VERSION 3.5)
project(fgame)

# Shared source files for modules
set(SOURCES_SHARED
	"../qcommon/class.cpp"
	"../qcommon/con_set.cpp"
	"../qcommon/con_timer.cpp"
	"../qcommon/listener.cpp"
	"../qcommon/lz77.cpp"
	"../qcommon/mem_blockalloc.cpp"
	"../qcommon/mem_tempalloc.cpp"
	"../qcommon/q_math.c"
	"../qcommon/q_shared.c"
	"../qcommon/script.cpp"
	"../qcommon/str.cpp"
)

file(GLOB_RECURSE SOURCES_GAME "./*.c*" "../parser/*.c*" "../script/*.c*")

add_library(fgame SHARED ${SOURCES_GAME})
target_compile_definitions(fgame PRIVATE GAME_DLL WITH_SCRIPT_ENGINE ARCHIVE_SUPPORTED)
target_compile_features(fgame PUBLIC cxx_nullptr)
target_compile_features(fgame PUBLIC c_variadic_macros)
target_link_libraries(fgame PUBLIC qcommon)

set_target_properties(fgame PROPERTIES PREFIX "${TARGET_PLATFORM_PREFIX}")
set_target_properties(fgame PROPERTIES OUTPUT_NAME "game${TARGET_ARCH_SUFFIX}${TARGET_CONFIG_SUFFIX}")
set_target_properties(fgame PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${TARGET_BASE_GAME}")

INSTALL(TARGETS fgame DESTINATION "${TARGET_BASE_GAME}")

if(MSVC)
	INSTALL(FILES $<TARGET_PDB_FILE:fgame> DESTINATION "${TARGET_BASE_GAME}" OPTIONAL)
endif()
