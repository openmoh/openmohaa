cmake_minimum_required(VERSION 3.12)
project(openmohaa)

include(misc/cmake/TargetArch.cmake)

target_architecture(TARGET_ARCH)
list(LENGTH TARGET_ARCH TARGET_ARCH_COUNT)

set(USE_INTERNAL_LIBS ON)
if(USE_SYSTEM_LIBS)
	set(USE_INTERNAL_LIBS OFF)
endif()

option(USE_INTERNAL_JPEG "If set, use bundled libjpeg." ${USE_INTERNAL_LIBS})
option(USE_INTERNAL_MAD  "If set, use bundled libmad."  ${USE_INTERNAL_LIBS})
option(USE_INTERNAL_ZLIB "If set, use bundled zlib."    ${USE_INTERNAL_LIBS})
option(USE_RENDERER_DLOPEN "Whether to compile the renderer as separate pluggable modules" OFF)

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

message(STATUS "Architecture detected: ${TARGET_ARCH}")

set(TARGET_BIN_SUFFIX "")

if(WIN32)
	set(TARGET_PLATFORM_PREFIX "")
	message(STATUS "Using Win32 naming convention")
elseif(UNIX)
	set(TARGET_PLATFORM_PREFIX "")
	message(STATUS "Using Unix naming convention")
else()
	set(TARGET_PLATFORM_PREFIX "")
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
	add_compile_definitions(_DEBUG)

	# NOTE: The following may mess up function importation
	#if(UNIX)
	#	# Enable all exports so all functions name can be seen during executable crash
	#	set(CMAKE_ENABLE_EXPORTS ON)
	#	message(STATUS "Enabling exports on Unix for backtrace")
	#endif()
else()
	# Non-debug builds
	add_compile_definitions(NDEBUG)
endif()

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

#
# Setup the installation directory
#
if(WIN32)
	# By default, both DLLs and EXEs are in the same directory
	set(CMAKE_DEFAULT_INSTALL_RUNTIME_DIR bin)
	set(BIN_INSTALL_SUBDIR ".")
	set(LIB_INSTALL_SUBDIR ".")
else()
	# Unix
	set(CMAKE_DEFAULT_INSTALL_RUNTIME_DIR lib${CMAKE_LIB_SUFFIX})
	set(BIN_INSTALL_SUBDIR ${CMAKE_PROJECT_NAME})
	set(LIB_INSTALL_SUBDIR ${CMAKE_PROJECT_NAME})
endif()

# By default, put both binaries and shared libraries in the same directory
# the game uses internal shared libraries that must be in the same folder as the binaries
set(CMAKE_INSTALL_BINDIR ${CMAKE_DEFAULT_INSTALL_RUNTIME_DIR} CACHE PATH "Binary dir")
set(CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_BINDIR} CACHE PATH "Library dir")
include(GNUInstallDirs)

#
# Common stuff
#
add_subdirectory("code/qcommon")
add_subdirectory("code/gamespy")

#
# Application
#
add_subdirectory("code/sys")

##
## Server app
##
add_subdirectory("code/server")

add_executable(omohaaded "code/null/null_client.c" "code/null/null_input.c" "code/null/null_snddma.c")
target_compile_definitions(omohaaded PRIVATE APP_MODULE DEDICATED)
target_compile_features(omohaaded PUBLIC cxx_nullptr)
target_compile_features(omohaaded PUBLIC c_variadic_macros)
target_link_libraries(omohaaded PRIVATE omohserver)
target_link_libraries(omohaaded PRIVATE syslib)
target_link_libraries(omohaaded PRIVATE qcommon qcommon_standalone)

# Add the gamespy dependency
target_include_directories(omohaaded PUBLIC "code/qcommon" "code/script" "code/gamespy" "code/server")
set_target_properties(omohaaded PROPERTIES OUTPUT_NAME "omohaaded${TARGET_BIN_SUFFIX}")
set_target_properties(omohaaded PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

INSTALL(TARGETS omohaaded DESTINATION ${CMAKE_INSTALL_BINDIR}/${BIN_INSTALL_SUBDIR})

if (MSVC)
	target_link_options(omohaaded PRIVATE "/MANIFEST:NO")
	INSTALL(FILES $<TARGET_PDB_FILE:omohaaded> DESTINATION ${CMAKE_INSTALL_BINDIR}/${BIN_INSTALL_SUBDIR} OPTIONAL)
endif()

if (NOT BUILD_NO_CLIENT)
	##
	## Client app
	##
	option(NO_MODERN_DMA "Use older sound-system" FALSE)

	add_subdirectory("code/client")
	add_subdirectory("code/renderercommon")
	add_subdirectory("code/sdl")

	add_executable(openmohaa "misc/dummy.c")
	target_link_libraries(openmohaa PRIVATE syslib)
	target_link_libraries(openmohaa PRIVATE omohserver)
	target_link_libraries(openmohaa PRIVATE omohclient)
	target_link_libraries(openmohaa PRIVATE omohrenderer)
	target_link_libraries(openmohaa PRIVATE qcommon qcommon_standalone)

	# Add the gamespy dependency
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

	INSTALL(TARGETS openmohaa DESTINATION ${CMAKE_INSTALL_BINDIR}/${BIN_INSTALL_SUBDIR})

	if (MSVC)
		target_link_options(openmohaa PRIVATE "/MANIFEST:NO")
		INSTALL(FILES $<TARGET_PDB_FILE:openmohaa> DESTINATION ${CMAKE_INSTALL_BINDIR}/${BIN_INSTALL_SUBDIR} OPTIONAL)
	endif()

	if(UNIX AND NOT APPLE)
		# Configure the .desktop entries with the arch suffix
		configure_file(
			misc/linux/org.openmoh.openmohaa.desktop.in
			${CMAKE_BINARY_DIR}/misc/linux/org.openmoh.openmohaa.desktop
			@ONLY
		)
		configure_file(
			misc/linux/org.openmoh.openmohaab.desktop.in
			${CMAKE_BINARY_DIR}/misc/linux/org.openmoh.openmohaab.desktop
			@ONLY
		)
		configure_file(
			misc/linux/org.openmoh.openmohaas.desktop.in
			${CMAKE_BINARY_DIR}/misc/linux/org.openmoh.openmohaas.desktop
			@ONLY
		)

		# Install .desktop entries
		install(FILES ${CMAKE_BINARY_DIR}/misc/linux/org.openmoh.openmohaa.desktop  DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
		install(FILES ${CMAKE_BINARY_DIR}/misc/linux/org.openmoh.openmohaab.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
		install(FILES ${CMAKE_BINARY_DIR}/misc/linux/org.openmoh.openmohaas.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)

		install(FILES misc/linux/org.openmoh.openmohaa.metainfo.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)

		install(FILES misc/openmohaa.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/symbolic/apps/ RENAME org.openmoh.openmohaa.svg)
	endif()
endif()

#
# Launcher
#
add_subdirectory(code/Launcher)

#
# uninstall target
#
if(NOT TARGET uninstall)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

  add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()
