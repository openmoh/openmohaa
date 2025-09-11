include(platforms/emscripten)
include(platforms/linux)
include(platforms/macos)
include(platforms/unix)
include(platforms/windows)

if(DEBUG_MEMORY)
	add_definitions(-D_DEBUG_MEM)
endif()

# By default, put both binaries and shared libraries in the same directory
# the game uses internal shared libraries that must be in the same folder as the binaries
set(CMAKE_INSTALL_BINDIR ${CMAKE_DEFAULT_INSTALL_RUNTIME_DIR} CACHE PATH "Binary dir")
set(CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_BINDIR} CACHE PATH "Library dir")

include(GNUInstallDirs)

cmake_path(SET INSTALL_BINDIR_FULL NORMALIZE "${CMAKE_INSTALL_BINDIR}/${BIN_INSTALL_SUBDIR}")
cmake_path(SET INSTALL_LIBDIR_FULL NORMALIZE "${CMAKE_INSTALL_LIBDIR}/${LIB_INSTALL_SUBDIR}")
cmake_path(SET INSTALL_DATADIR_FULL NORMALIZE "${CMAKE_INSTALL_DATADIR}")
