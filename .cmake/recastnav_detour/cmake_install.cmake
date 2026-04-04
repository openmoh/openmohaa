# Install script for directory: /run/media/elgan/bdb8dbed-86d7-41d2-90f3-09a45ed5ad9c/dev/openmohaa-central/code/thirdparty/recastnavigation/Detour

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/opt/mohaa")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/run/media/elgan/bdb8dbed-86d7-41d2-90f3-09a45ed5ad9c/dev/openmohaa-central/.cmake/recastnav_detour/libDetour.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/recastnavigation" TYPE FILE FILES
    "/run/media/elgan/bdb8dbed-86d7-41d2-90f3-09a45ed5ad9c/dev/openmohaa-central/code/thirdparty/recastnavigation/Detour/Include/DetourAlloc.h"
    "/run/media/elgan/bdb8dbed-86d7-41d2-90f3-09a45ed5ad9c/dev/openmohaa-central/code/thirdparty/recastnavigation/Detour/Include/DetourAssert.h"
    "/run/media/elgan/bdb8dbed-86d7-41d2-90f3-09a45ed5ad9c/dev/openmohaa-central/code/thirdparty/recastnavigation/Detour/Include/DetourCommon.h"
    "/run/media/elgan/bdb8dbed-86d7-41d2-90f3-09a45ed5ad9c/dev/openmohaa-central/code/thirdparty/recastnavigation/Detour/Include/DetourMath.h"
    "/run/media/elgan/bdb8dbed-86d7-41d2-90f3-09a45ed5ad9c/dev/openmohaa-central/code/thirdparty/recastnavigation/Detour/Include/DetourNavMesh.h"
    "/run/media/elgan/bdb8dbed-86d7-41d2-90f3-09a45ed5ad9c/dev/openmohaa-central/code/thirdparty/recastnavigation/Detour/Include/DetourNavMeshBuilder.h"
    "/run/media/elgan/bdb8dbed-86d7-41d2-90f3-09a45ed5ad9c/dev/openmohaa-central/code/thirdparty/recastnavigation/Detour/Include/DetourNavMeshQuery.h"
    "/run/media/elgan/bdb8dbed-86d7-41d2-90f3-09a45ed5ad9c/dev/openmohaa-central/code/thirdparty/recastnavigation/Detour/Include/DetourNode.h"
    "/run/media/elgan/bdb8dbed-86d7-41d2-90f3-09a45ed5ad9c/dev/openmohaa-central/code/thirdparty/recastnavigation/Detour/Include/DetourStatus.h"
    )
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/run/media/elgan/bdb8dbed-86d7-41d2-90f3-09a45ed5ad9c/dev/openmohaa-central/.cmake/recastnav_detour/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
