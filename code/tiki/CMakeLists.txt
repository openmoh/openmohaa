cmake_minimum_required(VERSION 3.12)

project(omohtiki)

set(SOURCES_TIKI
	"./tiki_anim.cpp"
	"./tiki_cache.cpp"
	"./tiki_commands.cpp"
	"./tiki_files.cpp"
	"./tiki_frame.cpp"
	"./tiki_imports.cpp"
	"./tiki_parse.cpp"
	"./tiki_skel.cpp"
	"./tiki_surface.cpp"
	"./tiki_tag.cpp"
	"./tiki_utility.cpp"
)


set(SOURCES_TIKI_COMMON
	"../qcommon/tiki_main.cpp"
	"../qcommon/tiki_script.cpp"
)

add_library(omohtiki STATIC ${SOURCES_TIKI} ${SOURCES_TIKI_COMMON})
target_compile_features(omohtiki PRIVATE cxx_nullptr)
target_compile_features(omohtiki PRIVATE c_variadic_macros)
target_link_libraries(omohtiki PUBLIC qcommon)