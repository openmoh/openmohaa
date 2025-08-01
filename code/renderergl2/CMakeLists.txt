cmake_minimum_required(VERSION 3.12)

project(omohrenderergl2)

file(GLOB SOURCES_RENDERER "*.c" "*.cpp" "../renderercommon/tr_*.c")
# Remove Quake III old model source
list(REMOVE_ITEM SOURCES_RENDERER "${CMAKE_CURRENT_SOURCE_DIR}/tr_model.c")

include("glsl/shaders.cmake")

if (${USE_RENDERER_DLOPEN})
	add_library(omohrenderergl2 SHARED ${SOURCES_RENDERER} ${SHADER_SRCS})
else()
	add_library(omohrenderergl2 STATIC ${SOURCES_RENDERER} ${SHADER_SRCS})
endif()

target_compile_features(omohrenderergl2 PUBLIC cxx_nullptr)
target_compile_features(omohrenderergl2 PUBLIC c_variadic_macros)
target_link_libraries(omohrenderergl2 PRIVATE omohrenderer_common)

get_target_property(target_type omohrenderergl2 TYPE)
if (target_type STREQUAL "SHARED_LIBRARY")
	target_compile_definitions(omohrenderergl2 PUBLIC USE_RENDERER_DLOPEN=1)
	target_compile_definitions(omohrenderergl2 PRIVATE REF_DLL=1)
endif ()

set_target_properties(omohrenderergl2 PROPERTIES PREFIX "")
set_target_properties(omohrenderergl2 PROPERTIES OUTPUT_NAME "renderer_opengl2${TARGET_BIN_SUFFIX}")

if (target_type STREQUAL "SHARED_LIBRARY")
	INSTALL(TARGETS omohrenderergl2 DESTINATION ${CMAKE_INSTALL_LIBDIR}/${LIB_INSTALL_SUBDIR})

	if(MSVC)
		INSTALL(FILES $<TARGET_PDB_FILE:omohrenderergl2> DESTINATION ${CMAKE_INSTALL_LIBDIR}/${LIB_INSTALL_SUBDIR} OPTIONAL)
	endif()
endif ()
