cmake_minimum_required(VERSION 3.5)
project(omohskeletor)

file(GLOB_RECURSE SOURCES_SKEL "./*.c" "./*.cpp")

add_library(omohskeletor STATIC ${SOURCES_SKEL})
target_compile_features(omohskeletor PRIVATE cxx_nullptr)
target_compile_features(omohskeletor PRIVATE c_variadic_macros)
target_link_libraries(omohskeletor PUBLIC qcommon)