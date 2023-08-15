cmake_minimum_required(VERSION 3.5)
project(openmohaa)

include(TargetArch.cmake)

target_architecture(TARGET_ARCH)


if(TARGET_GAME_TYPE)
	message(SEND_ERROR "TARGET_GAME_TYPE is now unsupported, it is now done at runtime.")
endif()


set(TARGET_BASE_GAME "main")
set(CMAKE_DEBUG_POSTFIX "-dbg")

if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	add_compile_options(-Wno-comment)
	# Ignore warnings for code like 'assert("Assert string")'
	add_compile_options(-Wno-pointer-bool-conversion)
endif()

IF("${TARGET_ARCH}" STREQUAL "i386")
	set(TARGET_ARCH_SUFFIX "x86")
ELSE()
	set(TARGET_ARCH_SUFFIX ${TARGET_ARCH})
ENDIF()

message(STATUS "Architecture detected: ${TARGET_ARCH}, suffix set to ${TARGET_ARCH_SUFFIX}.")

IF(WIN32)
	set(TARGET_PLATFORM_PREFIX "")
	message(STATUS "Using Win32 naming convention")
ELSEIF(UNIX)
	set(TARGET_PLATFORM_PREFIX "")
	message(STATUS "Using Unix naming convention")
ELSE()
	set(TARGET_PLATFORM_PREFIX "")
ENDIF()

IF(CMAKE_BUILD_TYPE MATCHES Debug)
	add_definitions(-D_DEBUG)

	#if(UNIX)
	#	# Enable all exports so all functions name can be seen during executable crash
	#	set(CMAKE_ENABLE_EXPORTS ON)
	#	message(STATUS "Enabling exports on Unix for backtrace")
	#endif()
ENDIF()

# Common stuff

add_subdirectory("code/qcommon")
add_subdirectory("code/gamespy")

# Application

add_subdirectory("code/sys")

## Server app
add_subdirectory("code/server")

add_executable(omohaaded "code/null/null_client.c" "code/null/null_input.c" "code/null/null_snddma.c")
target_compile_definitions(omohaaded PRIVATE APP_MODULE DEDICATED)
target_compile_features(omohaaded PUBLIC cxx_nullptr)
target_compile_features(omohaaded PUBLIC c_variadic_macros)
target_link_libraries(omohaaded PRIVATE omohserver)
target_link_libraries(omohaaded PRIVATE syslib)
target_link_libraries(omohaaded PRIVATE qcommon qcommon_standalone)
# Gamespy dependency
target_include_directories(omohaaded PUBLIC "code/qcommon" "code/script" "code/gamespy" "code/server")
set_target_properties(omohaaded PROPERTIES OUTPUT_NAME "omohaaded${TARGET_BASE_SUFFIX}${TARGET_ARCH_SUFFIX}")
set_target_properties(omohaaded PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

if (MSVC)
	target_link_options(omohaaded PRIVATE "/MANIFEST:NO")
	INSTALL(FILES $<TARGET_PDB_FILE:omohaaded> DESTINATION "./" OPTIONAL)
endif()

INSTALL(TARGETS omohaaded DESTINATION "./")

if (NOT BUILD_NO_CLIENT)
	## Client app

	add_subdirectory("code/renderer")
	add_subdirectory("code/sdl")
	add_subdirectory("code/client")

	#include("code/renderergl2/glsl/shaders.cmake")
	#file(GLOB_RECURSE SOURCES_RENDERER "code/sdl/*.c" "code/renderercommon/*.c" "code/renderergl2/*.c" "code/renderergl2/*.cpp")
	#list(FILTER SOURCES_RENDERER EXCLUDE REGEX "code/renderergl2/tr_subs.c")
	#list(FILTER SOURCES_RENDERER EXCLUDE REGEX "code/renderergl2/tr_model.c")

	add_executable(openmohaa "misc/dummy.c")
	target_link_libraries(openmohaa PRIVATE syslib)
	target_link_libraries(openmohaa PRIVATE omohserver)
	target_link_libraries(openmohaa PRIVATE omohclient)
	target_link_libraries(openmohaa PRIVATE omohrenderer omohsdl)
	target_link_libraries(openmohaa PRIVATE qcommon qcommon_standalone)

	### Gamespy dependency
	target_include_directories(openmohaa PUBLIC "code/qcommon" "code/script" "code/gamespy" "code/server" "code/client" "code/uilib" "code/jpeg-8c")
	set_target_properties(openmohaa PROPERTIES OUTPUT_NAME "openmohaa${TARGET_BASE_SUFFIX}${TARGET_ARCH_SUFFIX}")
	set_target_properties(openmohaa PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

	target_link_libraries(openmohaa PRIVATE jpeg8)
	if (USE_OPENAL)
		# Try to use OpenAL
		find_package(OpenAL)
		if (OPENAL_FOUND)
			target_compile_definitions(openmohaa PRIVATE USE_OPENAL=1)
			target_include_directories(openmohaa PRIVATE ${OPENAL_INCLUDE_DIR})
			target_link_libraries(openmohaa PRIVATE ${OPENAL_LIBRARY})
		endif()
	endif()

	if (MSVC)
		target_link_options(openmohaa PRIVATE "/MANIFEST:NO")
		INSTALL(FILES $<TARGET_PDB_FILE:openmohaa> DESTINATION "./" OPTIONAL)
	endif()

	INSTALL(TARGETS openmohaa DESTINATION "./")
endif()
