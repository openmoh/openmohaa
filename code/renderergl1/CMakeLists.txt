cmake_minimum_required(VERSION 3.12)

project(omohrenderer)

file(GLOB_RECURSE SOURCES_RENDERER "./*.c" "./*.cpp" "../renderercommon/tr_*.c")
list(REMOVE_ITEM SOURCES_RENDERER "./tr_subs.c")

set(SOURCES_RENDERER ${SOURCES_RENDERER}
	"../tiki/tiki_mesh.cpp"
)

if (${USE_RENDERER_DLOPEN})
	add_library(omohrenderer SHARED ${SOURCES_RENDERER})
else()
	add_library(omohrenderer STATIC ${SOURCES_RENDERER})
endif()
target_include_directories(omohrenderer PUBLIC "../sdl" "../renderercommon")
target_compile_features(omohrenderer PUBLIC cxx_nullptr)
target_compile_features(omohrenderer PUBLIC c_variadic_macros)
target_compile_definitions(omohrenderer PRIVATE USE_INTERNAL_JPEG=1)
target_link_libraries(omohrenderer PRIVATE qcommon_shared)
target_link_libraries(omohrenderer PRIVATE omohsdl_gl)

get_target_property(target_type omohrenderer TYPE)
if (target_type STREQUAL "SHARED_LIBRARY")
	target_sources(omohrenderer PRIVATE "./tr_subs.c")
	target_compile_definitions(omohrenderer PUBLIC USE_RENDERER_DLOPEN=1)
endif ()

if(USE_INTERNAL_JPEG)
	file(GLOB_RECURSE SOURCES_JPEG_8 "../jpeg-8c/*.c")
	add_library(jpeg8 STATIC ${SOURCES_JPEG_8})
	target_include_directories(jpeg8 PRIVATE "../jpeg-8c")
	target_include_directories(omohrenderer PUBLIC "../jpeg-8c")
	target_link_libraries(omohrenderer PRIVATE jpeg8)
else()
	target_include_directories(omohrenderer PUBLIC ${JPEG_INCLUDE_DIRS})
	target_link_libraries(omohrenderer PRIVATE ${JPEG_LIBRARIES})
endif()

set_target_properties(omohrenderer PROPERTIES PREFIX "")
set_target_properties(omohrenderer PROPERTIES OUTPUT_NAME "renderer_opengl1${TARGET_BIN_SUFFIX}")


if (target_type STREQUAL "SHARED_LIBRARY")
	INSTALL(TARGETS omohrenderer DESTINATION ${CMAKE_INSTALL_LIBDIR}/${LIB_INSTALL_SUBDIR})

	if(MSVC)
		INSTALL(FILES $<TARGET_PDB_FILE:omohrenderer> DESTINATION ${CMAKE_INSTALL_LIBDIR}/${LIB_INSTALL_SUBDIR} OPTIONAL)
	endif()
endif ()
