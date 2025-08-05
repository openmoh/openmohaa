# Windows specific settings

if(NOT WIN32)
    return()
endif()

list(APPEND SYSTEM_PLATFORM_SOURCES
    ${SOURCE_DIR}/sys/sys_win32.c
    ${SOURCE_DIR}/sys/con_passive.c
    ${SOURCE_DIR}/sys/win_resource.rc
)

if(USE_HTTP)
    list(APPEND CLIENT_PLATFORM_SOURCES ${SOURCE_DIR}/client/cl_http_windows.c)
    list(APPEND CLIENT_LIBRARIES wininet)
endif()

list(APPEND COMMON_LIBRARIES ws2_32 winmm psapi)

if(MINGW)
    list(APPEND COMMON_LIBRARIES mingw32)
endif()

list(APPEND CLIENT_DEFINITIONS USE_ICON)

set_source_files_properties(${SOURCE_DIR}/sys/win_resource.rc
    PROPERTIES COMPILE_DEFINITIONS WINDOWS_ICON_PATH=${WINDOWS_ICON_PATH})

if(MSVC)
    # We have our own manifest, disable auto creation
    list(APPEND SERVER_LINK_OPTIONS "/MANIFEST:NO")
    list(APPEND CLIENT_LINK_OPTIONS "/MANIFEST:NO")
endif()

set(CLIENT_EXECUTABLE_OPTIONS WIN32)
