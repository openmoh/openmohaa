# Unix specific settings (this includes macOS and emscripten)

if(NOT UNIX)
    return()
endif()

list(APPEND SYSTEM_PLATFORM_SOURCES ${SOURCE_DIR}/sys/sys_unix.c)

if(EMSCRIPTEN)
    list(APPEND SYSTEM_PLATFORM_SOURCES ${SOURCE_DIR}/sys/con_passive.c)
else()
    list(APPEND SYSTEM_PLATFORM_SOURCES ${SOURCE_DIR}/sys/con_tty.c)
endif()

#if(USE_HTTP)
#    list(APPEND CLIENT_PLATFORM_SOURCES ${SOURCE_DIR}/client/cl_http_curl.c)
#endif()

list(APPEND COMMON_LIBRARIES
    dl  # Dynamic loader
    m   # Math library
)

list(APPEND SYSTEM_PLATFORM_SOURCES ${SOURCE_DIR}/sys/new/sys_unix_new.c)

find_package(Threads)
list(APPEND COMMON_LIBRARIES ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})

#
# Setup the installation directory
#
set(CMAKE_DEFAULT_INSTALL_RUNTIME_DIR lib${CMAKE_LIB_SUFFIX})
set(BIN_INSTALL_SUBDIR ${CMAKE_PROJECT_NAME})
set(LIB_INSTALL_SUBDIR ${CMAKE_PROJECT_NAME})
