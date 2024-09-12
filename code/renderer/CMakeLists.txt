cmake_minimum_required(VERSION 3.5)
project(omohrenderer)

file(GLOB_RECURSE SOURCES_RENDERER "./*.c" "./*.cpp" "../renderercommon/tr_*.c")

file(GLOB_RECURSE SOURCES_JPEG_8 "../jpeg-8c/*.c")
add_library(jpeg8 STATIC ${SOURCES_JPEG_8})
target_include_directories(jpeg8 PRIVATE "../jpeg-8c")

add_library(omohrenderer STATIC ${SOURCES_RENDERER})
target_include_directories(omohrenderer PUBLIC "../sdl" "../renderercommon" "../jpeg-8c")
target_compile_features(omohrenderer PUBLIC cxx_nullptr)
target_compile_features(omohrenderer PUBLIC c_variadic_macros)
target_compile_definitions(omohrenderer PRIVATE USE_INTERNAL_JPEG=1)
target_link_libraries(omohrenderer PRIVATE qcommon)
target_link_libraries(omohrenderer PRIVATE omohsdl)
target_link_libraries(omohrenderer PRIVATE jpeg8)
