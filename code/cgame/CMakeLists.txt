cmake_minimum_required(VERSION 3.12)
project(cgame)

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
	"../script/scriptexception.cpp"
	"../script/scriptvariable.cpp"
)

file(GLOB_RECURSE SOURCES_BG "../fgame/bg_misc.cpp" "../fgame/bg_pmove.cpp" "../fgame/bg_slidemove.cpp" "../fgame/bg_voteoptions.cpp")
file(GLOB_RECURSE SOURCES_CGAME "./*.c" "./*.cpp")

add_library(cgame SHARED ${SOURCES_CGAME} ${SOURCES_BG})
target_compile_definitions(cgame PRIVATE CGAME_DLL)
target_compile_features(cgame PUBLIC cxx_nullptr)
target_compile_features(cgame PUBLIC c_variadic_macros)
target_link_libraries(cgame PUBLIC qcommon)

set_target_properties(cgame PROPERTIES PREFIX "")
set_target_properties(cgame PROPERTIES OUTPUT_NAME "cgame${TARGET_BIN_SUFFIX}")

INSTALL(TARGETS cgame DESTINATION ${CMAKE_INSTALL_LIBDIR}/${LIB_INSTALL_SUBDIR})

if(MSVC)
	INSTALL(FILES $<TARGET_PDB_FILE:cgame> DESTINATION ${CMAKE_INSTALL_LIBDIR}/${LIB_INSTALL_SUBDIR} OPTIONAL)
endif()