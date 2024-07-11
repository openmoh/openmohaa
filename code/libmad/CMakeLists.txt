cmake_minimum_required(VERSION 3.1.0)
project(mad VERSION 0.16.4)

option(BUILD_SHARED_LIBS "Build dynamic library" ON)

# The library SOVERSION. This is set to 0 for backward compatibility.
# The general policy is that minor versions of the library (e.g., 0.16.1,
# 0.16.2) don't constitute a major ABI breakage. Major versions (e.g., 0.17,
# 0.18) do.
set(LIBRARY_SOVERSION 0)

include(GNUInstallDirs)
include(CheckTypeSize)

#
# Build
#

add_library(mad
    bit.c
    decoder.c
    fixed.c
    frame.c
    huffman.c
    layer12.c
    layer3.c
    stream.c
    synth.c
    timer.c
    version.c
)
target_include_directories(mad PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(WIN32 AND BUILD_SHARED_LIBS)
    set_target_properties(mad PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

set_target_properties(mad PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION ${LIBRARY_SOVERSION}
)

#
# Compile definitions
#

option(OPTIMIZE "Optimize for SPEED (default) or ACCURACY" SPEED)
if(OPTIMIZE STREQUAL "SPEED")
  message(STATUS "Optimizing for speed over accuracy.")
  target_compile_definitions(mad PRIVATE OPT_SPEED)
else()
  message(STATUS "Optimizing for accuracy over speed.")
  target_compile_definitions(mad PRIVATE OPT_ACCURACY)
endif()

option(ASO "Enable CPU Architecture Specific Optimizations (x86, ARM, and MIPS only)" ON)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(STATUS "Using 64 bit fixed point math")
  option(FPM_64BIT "64 bit fixed point math" ON)
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86" OR "i386")
  message(STATUS "Using x86 fixed point math")
  option (FPM_INTEL "x86 fixed point math" ON)
  if(ASO)
    target_compile_definitions(mad PRIVATE ASO_ZEROCHECK)
  endif()
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES ".*(arm|ARM).*")
  message(STATUS "Using ARM fixed point math")
  option (FPM_ARM "ARM fixed point math" ON)
  if(ASO)
    enable_language(ASM)
    target_compile_definitions(mad PRIVATE ASO_INTERLEAVE1 ASO_IMDCT)
    target_sources(mad PRIVATE imdct_l_arm.S)
  endif()
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES ".*(mips|MIPS).*")
  message(STATUS "Using MIPS fixed point math")
  option(FPM_MIPS "MIPS fixed point math" ON)
  if(ASO)
    target_compile_definitions(mad PRIVATE ASO_INTERLEAVE2 ASO_ZEROCHECK)
  endif()
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES ".*(sparc|SPARC).*")
  message(STATUS "Using SPARC fixed point math")
  option(FPM_SPARC "SPARC fixed point math" ON)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES ".(ppc|PPC|powerpc).*")
  message(STATUS "Using PowerPC fixed point math")
  option(FPM_PPC "PowerPC fixed point math" ON)
else()
  message(WARNING "Target CPU architecture not detected. Fixed-point math will yield limited accuracy.")
  option(FPM_DEFAULT "Generic fixed-point math" ON)
endif()


check_type_size(int SIZEOF_INT BUILTIN_TYPES_ONLY LANGUAGE C)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mad.h.in ${CMAKE_CURRENT_BINARY_DIR}/mad.h @ONLY)

include(CheckIncludeFile)
check_include_file(sys/types.h HAVE_SYS_TYPES_H)
if(HAVE_SYS_TYPES_H)
  target_compile_definitions(mad PRIVATE HAVE_SYS_TYPES_H)
endif()

check_include_file(sys/wait.h HAVE_SYS_WAIT_H)
if(HAVE_SYS_WAIT_H)
  target_compile_definitions(mad PRIVATE HAVE_SYS_WAIT_H)
endif()

check_include_file(sys/mman.h HAVE_SYS_MMAN_H)
if(HAVE_SYS_MMAN_H)
  target_compile_definitions(mad PRIVATE HAVE_SYS_MMAN_H)
endif()

check_include_file(sys/stat.h HAVE_SYS_STAT_H)
if(HAVE_SYS_STAT_H)
  target_compile_definitions(mad PRIVATE HAVE_SYS_STAT_H)
endif()

check_include_file(unistd.h HAVE_UNISTD_H)
if(HAVE_UNISTD_H)
  target_compile_definitions(mad PRIVATE HAVE_UNISTD_H)
endif()

check_include_file(assert.h HAVE_ASSERT_H)
if(HAVE_ASSERT_H)
  target_compile_definitions(mad PRIVATE HAVE_ASSERT_H)
endif()

check_include_file(fcntl.h HAVE_FCNTL_H)
if(HAVE_FCNTL_H)
  target_compile_definitions(mad PRIVATE HAVE_FCNTL_H)
endif()

check_include_file(limits.h HAVE_LIMITS_H)
if(HAVE_LIMITS_H)
  target_compile_definitions(mad PRIVATE HAVE_LIMITS_H)
endif()

include(CheckFunctionExists)
check_function_exists(ftruncate HAVE_FTRUNCATE)
if(HAVE_FTRUNCATE)
  target_compile_definitions(mad PRIVATE HAVE_FTRUNCATE)
endif()

check_function_exists(pipe HAVE_PIPE)
if(HAVE_PIPE)
  target_compile_definitions(mad PRIVATE HAVE_PIPE)
endif()

check_function_exists(fork HAVE_FORK)
if(HAVE_FORK)
  target_compile_definitions(mad PRIVATE HAVE_FORK)
endif()

check_function_exists(waitpid HAVE_WAITPID)
if(HAVE_WAITPID)
  target_compile_definitions(mad PRIVATE HAVE_WAITPID)
endif()

option(MADD_ASM "Enable if your MIPS CPU supports a 2-operand MADD instruction." OFF)
if(MADD_ASM)
  target_compile_definitions(mad PRIVATE HAVE_MADD_ASM)
endif()

option(MADD16_ASM "Enable if your MIPS CPU supports a 2-operand MADD16 instruction." OFF)
if(MADD16_ASM)
  target_compile_definitions(mad PRIVATE HAVE_MADD_ASM)
endif()

#
# Example application
#

include(CMakeDependentOption)
cmake_dependent_option(EXAMPLE "Build example executable" ON "HAVE_UNISTD_H;HAVE_SYS_STAT_H;HAVE_SYS_MMAN_H" OFF)
if(EXAMPLE)
  add_executable(mad_example minimad.c)
  target_link_libraries(mad_example PRIVATE mad)
endif()

#
# Installation
#

include(CMakePackageConfigHelpers)

# Library files
install(TARGETS mad
  EXPORT madTargets
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
  INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

# Header files
install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/mad.h"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

# pkgconfig
if(IS_ABSOLUTE "${CMAKE_INSTALL_LIBDIR}")
  set(PKGCONFIG_LIBDIR "${CMAKE_INSTALL_LIBDIR}")
else()
  set(PKGCONFIG_LIBDIR "\${prefix}/${CMAKE_INSTALL_LIBDIR}")
endif()
if(IS_ABSOLUTE "${CMAKE_INSTALL_INCLUDEDIR}")
  set(PKGCONFIG_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}")
else()
  set(PKGCONFIG_INCLUDEDIR "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
endif()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/packaging/mad.pc.in
  ${CMAKE_CURRENT_BINARY_DIR}/packaging/mad.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/packaging/mad.pc DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

# CMake config
set(MAD_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/mad")
install(
  EXPORT madTargets
  FILE madTargets.cmake
  NAMESPACE mad::
  DESTINATION "${MAD_INSTALL_CMAKEDIR}"
)
configure_package_config_file(packaging/madConfig.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/packaging/madConfig.cmake"
  INSTALL_DESTINATION "${MAD_INSTALL_CMAKEDIR}"
)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/packaging/madConfigVersion.cmake"
  VERSION "${PROJECT_VERSION}"
  COMPATIBILITY SameMajorVersion
)
install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/packaging/madConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/packaging/madConfigVersion.cmake"
  DESTINATION "${MAD_INSTALL_CMAKEDIR}"
)
