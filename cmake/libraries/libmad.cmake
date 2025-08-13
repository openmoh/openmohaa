if(NOT USE_CODEC_MAD)
    return()
endif()

if(NOT BUILD_CLIENT)
    return()
endif()

set(OLD_VALUE ${BUILD_SHARED_LIBS})
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

if(USE_INTERNAL_MAD)
	add_subdirectory("${SOURCE_DIR}/thirdparty/libmad" "./libmad" EXCLUDE_FROM_ALL)

    list(APPEND CLIENT_LIBRARIES mad)
else()
	include(FindPackageHandleStandardArgs)
	find_path(LIBMAD_INCLUDE_DIRS mad.h)
	find_library(LIBMAD_LIBRARIES mad)
	find_package_handle_standard_args(
		LibMad
		DEFAULT_MSG
		LIBMAD_LIBRARIES
		LIBMAD_INCLUDE_DIRS
	)

    list(APPEND CLIENT_INCLUDE_DIRS ${LIBMAD_INCLUDE_DIRS})
    list(APPEND CLIENT_LIBRARIES ${LIBMAD_LIBRARIES})
endif()

set(BUILD_SHARED_LIBS ${OLD_VALUE} CACHE BOOL "" FORCE)

list(APPEND CLIENT_DEFINITIONS USE_CODEC_MAD=1)
