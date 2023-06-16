cmake_minimum_required(VERSION 3.5)
project(omohserver)

add_subdirectory("../fgame" "./fgame")

file(GLOB_RECURSE SOURCES_SERVER "./*.c*")

add_library(omohserver STATIC ${SOURCES_SERVER})
target_compile_features(omohserver PUBLIC cxx_nullptr)
target_compile_features(omohserver PUBLIC c_variadic_macros)
target_link_libraries(omohserver PRIVATE qcommon qcommon_standalone)
target_link_libraries(omohserver PRIVATE gcd)
