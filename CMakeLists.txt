cmake_minimum_required(VERSION 3.5)
project(openmohaa)

include(TargetArch.cmake)

target_architecture(TARGET_ARCH)

set(USE_INTERNAL_LIBS ON)
if(USE_SYSTEM_LIBS)
	set(USE_INTERNAL_LIBS OFF)
endif()

option(USE_INTERNAL_JPEG "If set, use bundled libjpeg." ${USE_INTERNAL_LIBS})
option(USE_INTERNAL_MAD  "If set, use bundled libmad."  ${USE_INTERNAL_LIBS})
option(USE_INTERNAL_ZLIB "If set, use bundled zlib."    ${USE_INTERNAL_LIBS})


if(TARGET_GAME_TYPE)
	message(SEND_ERROR "TARGET_GAME_TYPE is now unsupported, it is now done at runtime.")
endif()


set(TARGET_BASE_GAME "./")
set(CMAKE_DEBUG_POSTFIX "-dbg")

#
# Microsoft compiler specific parameters
#
if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE)
	# Treat no return type as error
	add_compile_options(/we4715)
endif()

#
# Clang and GCC specific parameters
#
if(CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	add_compile_options(-Wno-comment)
	# Treat no return type as error
	add_compile_options(-Werror=return-type)
endif()

#
# Clang specific parameters
#
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	# Ignore warnings for code like 'assert("Assert string")'
	add_compile_options(-Wno-pointer-bool-conversion)
endif()

#
# GCC specific parameters
#
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	# Add this option on gcc to prevent functions from having the STB_GNU_UNIQUE binding
	# Otherwise, it would prevent libraries from being unloaded
	# which will cause undefined behavior and crashes due to memory corruption
	add_compile_options(-fno-gnu-unique)
endif()

if(DEBUG_MEMORY)
	add_definitions(-D_DEBUG_MEM)
endif()

IF("${TARGET_ARCH}" STREQUAL "i386")
	set(TARGET_ARCH_SUFFIX "x86")
ELSE()
	set(TARGET_ARCH_SUFFIX ${TARGET_ARCH})
ENDIF()

set(TARGET_BIN_SUFFIX ".${TARGET_ARCH}")

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
	add_compile_definitions(_DEBUG)

	# NOTE: The following may mess up function importation
	#if(UNIX)
	#	# Enable all exports so all functions name can be seen during executable crash
	#	set(CMAKE_ENABLE_EXPORTS ON)
	#	message(STATUS "Enabling exports on Unix for backtrace")
	#endif()
ELSE()
	# Non-debug builds
	add_compile_definitions(NDEBUG)
ENDIF()

if(APPLE)
	# macOS doesn't search the executable path by default
	# so, locate libraries like SDL in the executable path
	set(CMAKE_INSTALL_RPATH "@executable_path")
	set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

	if(CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "GNU")
		# Set the visibility to hidden on macOS to prevent shared libraries from
		# using functions in the executable
		# it's ok to hide them because the backtrace on macOS will still print the name of the functions
		add_compile_options(-fvisibility=hidden)
	endif()
endif()

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
set_target_properties(omohaaded PROPERTIES OUTPUT_NAME "omohaaded${TARGET_BIN_SUFFIX}")
set_target_properties(omohaaded PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

if (MSVC)
	target_link_options(omohaaded PRIVATE "/MANIFEST:NO")
	INSTALL(FILES $<TARGET_PDB_FILE:omohaaded> DESTINATION "./" OPTIONAL)
endif()

INSTALL(TARGETS omohaaded DESTINATION "./")

if (NOT BUILD_NO_CLIENT)
	## Client app
	
	add_subdirectory("code/client")
	add_subdirectory("code/renderer")
	add_subdirectory("code/sdl")

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
	target_include_directories(openmohaa PUBLIC "code/qcommon" "code/script" "code/gamespy" "code/server" "code/client" "code/uilib")
	set_target_properties(openmohaa PROPERTIES OUTPUT_NAME "openmohaa${TARGET_BIN_SUFFIX}")
	set_target_properties(openmohaa PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

	if(USE_INTERNAL_JPEG)
		target_include_directories(openmohaa PUBLIC "code/jpeg-8c")
		target_link_libraries(openmohaa PRIVATE jpeg8)
	else()
		find_package(JPEG REQUIRED)
		target_include_directories(openmohaa PUBLIC ${JPEG_INCLUDE_DIRS})
		target_link_libraries(openmohaa PRIVATE ${JPEG_LIBRARIES})
	endif()

	if (MSVC)
		target_link_options(openmohaa PRIVATE "/MANIFEST:NO")
		INSTALL(FILES $<TARGET_PDB_FILE:openmohaa> DESTINATION "./" OPTIONAL)
	endif()

	INSTALL(TARGETS openmohaa DESTINATION "./")
endif()

# Launcher

add_subdirectory(code/Launcher)

# uninstall target
if(NOT TARGET uninstall)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

  add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()

