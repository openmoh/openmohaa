if(NOT USE_CODEC_VORBIS)
    return()
endif()

include(utils/disable_warnings)

set(INTERNAL_OGG_DIR ${SOURCE_DIR}/thirdparty/libogg-1.3.6)

if(USE_INTERNAL_OGG)
    file(GLOB_RECURSE OGG_SOURCES ${INTERNAL_OGG_DIR}/*.c)
    disable_warnings(${OGG_SOURCES})
    set(OGG_INCLUDE_DIRS ${INTERNAL_OGG_DIR}/include)
    list(APPEND CLIENT_LIBRARY_SOURCES ${OGG_SOURCES})
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(OGG REQUIRED ogg)
endif()

list(APPEND CLIENT_LIBRARIES ${OGG_LIBRARIES})
list(APPEND CLIENT_INCLUDE_DIRS ${OGG_INCLUDE_DIRS})
list(APPEND CLIENT_DEFINITIONS ${OGG_DEFINITIONS})
