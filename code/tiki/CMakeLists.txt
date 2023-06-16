cmake_minimum_required(VERSION 3.5)
project(omohtiki)

file(GLOB_RECURSE SOURCES_TIKI "./*.c" "./*.cpp")

set(COMMON_TIKI_SRCS
	"../qcommon/tiki_main.cpp"
	"../qcommon/tiki_script.cpp"
)

add_library(omohtiki STATIC ${SOURCES_TIKI} ${COMMON_TIKI_SRCS})
target_compile_features(omohtiki PRIVATE cxx_nullptr)
target_compile_features(omohtiki PRIVATE c_variadic_macros)
target_link_libraries(omohtiki PUBLIC qcommon)