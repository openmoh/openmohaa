cmake_minimum_required(VERSION 3.12)

project(omohrenderer)

file(GLOB_RECURSE SOURCES_RENDERER_COMMON "./*.c" "./*.cpp")

set(SOURCES_RENDERER_COMMON ${SOURCES_RENDERER_COMMON}
	"../tiki/tiki_mesh.cpp"
)

add_library(omohrenderer_common STATIC ${SOURCES_RENDERER_COMMON})
target_include_directories(omohrenderer_common PUBLIC "../sdl" "./")
target_compile_features(omohrenderer_common PUBLIC cxx_nullptr)
target_compile_features(omohrenderer_common PUBLIC c_variadic_macros)
target_link_libraries(omohrenderer_common PUBLIC qcommon_shared)
target_link_libraries(omohrenderer_common PUBLIC omohsdl_gl)
target_compile_definitions(omohrenderer_common PRIVATE USE_INTERNAL_JPEG=1)

# Add JPEG
if(USE_INTERNAL_JPEG)
    set(SOURCES_JPEG_DIR "../jpeg-9f")

    set(SOURCES_JPEG "${SOURCES_JPEG_DIR}/jaricom.c"
    "${SOURCES_JPEG_DIR}/jcapimin.c"
    "${SOURCES_JPEG_DIR}/jcapistd.c"
    "${SOURCES_JPEG_DIR}/jcarith.c"
    "${SOURCES_JPEG_DIR}/jccoefct.c"
    "${SOURCES_JPEG_DIR}/jccolor.c"
    "${SOURCES_JPEG_DIR}/jcdctmgr.c"
    "${SOURCES_JPEG_DIR}/jchuff.c"
    "${SOURCES_JPEG_DIR}/jcinit.c"
    "${SOURCES_JPEG_DIR}/jcmainct.c"
    "${SOURCES_JPEG_DIR}/jcmarker.c"
    "${SOURCES_JPEG_DIR}/jcmaster.c"
    "${SOURCES_JPEG_DIR}/jcomapi.c"
    "${SOURCES_JPEG_DIR}/jcparam.c"
    "${SOURCES_JPEG_DIR}/jcprepct.c"
    "${SOURCES_JPEG_DIR}/jcsample.c"
    "${SOURCES_JPEG_DIR}/jctrans.c"
    "${SOURCES_JPEG_DIR}/jdapimin.c"
    "${SOURCES_JPEG_DIR}/jdapistd.c"
    "${SOURCES_JPEG_DIR}/jdarith.c"
    "${SOURCES_JPEG_DIR}/jdatadst.c"
    "${SOURCES_JPEG_DIR}/jdatasrc.c"
    "${SOURCES_JPEG_DIR}/jdcoefct.c"
    "${SOURCES_JPEG_DIR}/jdcolor.c"
    "${SOURCES_JPEG_DIR}/jddctmgr.c"
    "${SOURCES_JPEG_DIR}/jdhuff.c"
    "${SOURCES_JPEG_DIR}/jdinput.c"
    "${SOURCES_JPEG_DIR}/jdmainct.c"
    "${SOURCES_JPEG_DIR}/jdmarker.c"
    "${SOURCES_JPEG_DIR}/jdmaster.c"
    "${SOURCES_JPEG_DIR}/jdmerge.c"
    "${SOURCES_JPEG_DIR}/jdpostct.c"
    "${SOURCES_JPEG_DIR}/jdsample.c"
    "${SOURCES_JPEG_DIR}/jdtrans.c"
    "${SOURCES_JPEG_DIR}/jerror.c"
    "${SOURCES_JPEG_DIR}/jfdctflt.c"
    "${SOURCES_JPEG_DIR}/jfdctfst.c"
    "${SOURCES_JPEG_DIR}/jfdctint.c"
    "${SOURCES_JPEG_DIR}/jidctflt.c"
    "${SOURCES_JPEG_DIR}/jidctfst.c"
    "${SOURCES_JPEG_DIR}/jidctint.c"
    "${SOURCES_JPEG_DIR}/jmemmgr.c"
    "${SOURCES_JPEG_DIR}/jmemnobs.c"
    "${SOURCES_JPEG_DIR}/jquant1.c"
    "${SOURCES_JPEG_DIR}/jquant2.c"
    "${SOURCES_JPEG_DIR}/jutils.c")

	add_library(jpeg STATIC ${SOURCES_JPEG})
	target_include_directories(jpeg PRIVATE "${SOURCES_JPEG_DIR}")
	target_include_directories(omohrenderer_common PUBLIC "${SOURCES_JPEG_DIR}")
	target_link_libraries(omohrenderer_common PRIVATE jpeg)
	target_compile_definitions(omohrenderer_common PUBLIC USE_INTERNAL_JPEG)
else()
	find_package(JPEG REQUIRED)
	target_include_directories(omohrenderer_common PUBLIC ${JPEG_INCLUDE_DIRS})
	target_link_libraries(omohrenderer_common PRIVATE ${JPEG_LIBRARIES})
endif()

add_subdirectory("../renderergl1" renderergl1)

add_library(omohrenderer INTERFACE)
target_link_libraries(omohrenderer INTERFACE omohrenderergl1)

if (${USE_RENDERER_DLOPEN})
	add_subdirectory("../renderergl2" renderergl2)
	
	target_link_libraries(omohrenderer INTERFACE omohrenderergl2)
endif()