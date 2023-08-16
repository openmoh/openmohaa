cmake_minimum_required(VERSION 3.5)
project(omohserver)

add_subdirectory("../fgame" "./fgame")

file(GLOB_RECURSE SOURCES_SERVER "./*.c*")

add_library(omohserver INTERFACE)
target_sources(omohserver INTERFACE ${SOURCES_SERVER})
target_compile_features(omohserver INTERFACE cxx_nullptr)
target_compile_features(omohserver INTERFACE c_variadic_macros)
target_link_libraries(omohserver INTERFACE qcommon qcommon_standalone)
target_link_libraries(omohserver INTERFACE gcd)
