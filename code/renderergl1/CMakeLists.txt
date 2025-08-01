cmake_minimum_required(VERSION 3.12)

project(omohrenderergl1)

file(GLOB_RECURSE SOURCES_RENDERER "./*.c" "./*.cpp")

if (${USE_RENDERER_DLOPEN})
	add_library(omohrenderergl1 SHARED ${SOURCES_RENDERER})
else()
	add_library(omohrenderergl1 STATIC ${SOURCES_RENDERER})
endif()

target_compile_features(omohrenderergl1 PUBLIC cxx_nullptr)
target_compile_features(omohrenderergl1 PUBLIC c_variadic_macros)
target_link_libraries(omohrenderergl1 PRIVATE omohrenderer_common)

get_target_property(target_type omohrenderergl1 TYPE)
if (target_type STREQUAL "SHARED_LIBRARY")
	target_compile_definitions(omohrenderergl1 PUBLIC USE_RENDERER_DLOPEN=1)
	target_compile_definitions(omohrenderergl1 PRIVATE REF_DLL=1)
endif ()

set_target_properties(omohrenderergl1 PROPERTIES PREFIX "")
set_target_properties(omohrenderergl1 PROPERTIES OUTPUT_NAME "renderer_opengl1${TARGET_BIN_SUFFIX}")

if (target_type STREQUAL "SHARED_LIBRARY")
	INSTALL(TARGETS omohrenderergl1 DESTINATION ${CMAKE_INSTALL_LIBDIR}/${LIB_INSTALL_SUBDIR})

	if(MSVC)
		INSTALL(FILES $<TARGET_PDB_FILE:omohrenderergl1> DESTINATION ${CMAKE_INSTALL_LIBDIR}/${LIB_INSTALL_SUBDIR} OPTIONAL)
	endif()
endif ()
