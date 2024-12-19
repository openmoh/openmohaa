cmake_minimum_required(VERSION 3.12)

project(omohrenderer)

file(GLOB_RECURSE SOURCES_RENDERER_COMMON "./*.c" "./*.cpp")

set(SOURCES_RENDERER_COMMON ${SOURCES_RENDERER_COMMON}
	"../tiki/tiki_mesh.cpp"
)

add_library(omohrenderer_common STATIC ${SOURCES_RENDERER_COMMON})
target_include_directories(omohrenderer_common PUBLIC "../sdl" "./")
target_compile_features(omohrenderer_common PUBLIC cxx_nullptr)
target_compile_features(omohrenderer_common PUBLIC c_variadic_macros)
target_link_libraries(omohrenderer_common PUBLIC qcommon_shared)
target_link_libraries(omohrenderer_common PUBLIC omohsdl_gl)
target_compile_definitions(omohrenderer_common PRIVATE USE_INTERNAL_JPEG=1)

# Add JPEG
if(USE_INTERNAL_JPEG)
	file(GLOB_RECURSE SOURCES_JPEG_8 "../jpeg-8c/*.c")
	add_library(jpeg8 STATIC ${SOURCES_JPEG_8})
	target_include_directories(jpeg8 PRIVATE "../jpeg-8c")
	target_include_directories(omohrenderer_common PUBLIC "../jpeg-8c")
	target_link_libraries(omohrenderer_common PRIVATE jpeg8)
else()
	target_include_directories(omohrenderer_common PUBLIC ${JPEG_INCLUDE_DIRS})
	target_link_libraries(omohrenderer_common PRIVATE ${JPEG_LIBRARIES})
endif()

add_subdirectory("../renderergl1" renderergl1)

add_library(omohrenderer INTERFACE)
target_link_libraries(omohrenderer INTERFACE omohrenderergl1)

if (${USE_RENDERER_DLOPEN})
	add_subdirectory("../renderergl2" renderergl2)
	
	target_link_libraries(omohrenderer INTERFACE omohrenderergl2)
endif()