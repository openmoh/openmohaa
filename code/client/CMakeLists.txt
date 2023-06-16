cmake_minimum_required(VERSION 3.5)
project(omohclient)

add_subdirectory("../cgame" "./cgame")

file(GLOB_RECURSE SOURCES_CLIENT "./*.c*")
file(GLOB_RECURSE SOURCES_UILIB "../uilib/*.c*")

# Made as an interface and not static, as static only links used methods
add_library(omohclient INTERFACE)
target_sources(omohclient PUBLIC ${SOURCES_CLIENT} ${SOURCES_UILIB})
target_compile_definitions(omohclient INTERFACE APP_MODULE)
target_compile_features(omohclient INTERFACE cxx_nullptr)
target_compile_features(omohclient INTERFACE c_variadic_macros)
target_link_libraries(omohclient INTERFACE omohsdl)
target_link_libraries(omohclient INTERFACE gcd)

# Sound stuff
target_compile_definitions(omohclient INTERFACE USE_CODEC_MP3)

add_subdirectory("../libmad-0.15.1b" "./libmad")
target_link_libraries(omohclient INTERFACE libmad)
target_include_directories(omohclient INTERFACE "../libmad-0.15.1b")