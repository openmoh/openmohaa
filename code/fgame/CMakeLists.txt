cmake_minimum_required(VERSION 3.12)
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

file(
	GLOB_RECURSE
	SOURCES_GAME
	"./*.c*"
	"../script/*.c*"
	"../parser/generated/*.c*"
	"../parser/parsetree.cpp"
)

add_library(fgame SHARED ${SOURCES_GAME})

#
# Workaround to get a recent version Flex and Bison executables in MacOS
# as Bison 2.3 shipped with MacOS doesn't work
#
if (CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
    execute_process(COMMAND brew --prefix bison RESULT_VARIABLE APP_RESULT OUTPUT_VARIABLE APP_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (APP_RESULT EQUAL 0 AND EXISTS "${APP_PREFIX}")
        set(BISON_EXECUTABLE "${APP_PREFIX}/bin/bison")
    endif()

    execute_process(COMMAND brew --prefix flex RESULT_VARIABLE APP_RESULT OUTPUT_VARIABLE APP_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (APP_RESULT EQUAL 0 AND EXISTS "${APP_PREFIX}")
        set(FLEX_EXECUTABLE "${APP_PREFIX}/bin/flex")
    endif()
endif()

find_package(BISON 3 REQUIRED)
find_package(FLEX REQUIRED)

if (FLEX_FOUND)
	flex_target(fgame-lexer ${CMAKE_SOURCE_DIR}/code/parser/lex_source.txt ${CMAKE_SOURCE_DIR}/code/parser/generated/yyLexer.cpp DEFINES_FILE ${CMAKE_SOURCE_DIR}/code/parser/generated/yyLexer.h COMPILE_FLAGS "-Cem --nounistd")
endif()

if (BISON_FOUND)
	bison_target(fgame-parser ${CMAKE_SOURCE_DIR}/code/parser/bison_source.txt ${CMAKE_SOURCE_DIR}/code/parser/generated/yyParser.cpp)
endif()

if (FLEX_FOUND AND BISON_FOUND)
	add_flex_bison_dependency(fgame-lexer fgame-parser)
endif()

# Recast navigation
set(OLD_VALUE ${BUILD_SHARED_LIBS})
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

add_subdirectory(../recastnavigation-1.6.0/Detour fgame_detour)
add_subdirectory(../recastnavigation-1.6.0/Recast fgame_recast)
target_link_libraries(fgame PRIVATE RecastNavigation::Detour RecastNavigation::Recast)

set(BUILD_SHARED_LIBS ${OLD_VALUE} CACHE BOOL "" FORCE)

file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/code/parser/generated")
target_sources(fgame PRIVATE ${BISON_fgame-parser_OUTPUTS} ${FLEX_fgame-lexer_OUTPUTS})

target_compile_definitions(fgame PRIVATE GAME_DLL WITH_SCRIPT_ENGINE ARCHIVE_SUPPORTED)
target_compile_features(fgame PUBLIC cxx_nullptr)
target_compile_features(fgame PUBLIC c_variadic_macros)
target_link_libraries(fgame PUBLIC qcommon)

set_target_properties(fgame PROPERTIES PREFIX "")
set_target_properties(fgame PROPERTIES OUTPUT_NAME "game${TARGET_BIN_SUFFIX}")

INSTALL(TARGETS fgame DESTINATION ${CMAKE_INSTALL_LIBDIR}/${LIB_INSTALL_SUBDIR})

if(MSVC)
	INSTALL(FILES $<TARGET_PDB_FILE:fgame> DESTINATION ${CMAKE_INSTALL_LIBDIR}/${LIB_INSTALL_SUBDIR} OPTIONAL)
endif()
