cmake_minimum_required(VERSION 3.12)

project(omohrenderer)

file(GLOB_RECURSE SOURCES_RENDERER "./*.c" "./*.cpp" "../renderercommon/tr_*.c")

add_library(omohrenderer STATIC ${SOURCES_RENDERER})
target_include_directories(omohrenderer PUBLIC "../sdl" "../renderercommon")
target_compile_features(omohrenderer PUBLIC cxx_nullptr)
target_compile_features(omohrenderer PUBLIC c_variadic_macros)
target_compile_definitions(omohrenderer PRIVATE USE_INTERNAL_JPEG=1)
target_link_libraries(omohrenderer PRIVATE qcommon)
target_link_libraries(omohrenderer PRIVATE omohsdl)

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
