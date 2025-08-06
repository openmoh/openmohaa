if(NOT BUILD_RENDERER_GL1)
    return()
endif()

include(utils/arch)
include(utils/set_output_dirs)
include(renderer_common)

set(RENDERER_GL1_SOURCES
    ${SOURCE_DIR}/renderergl1/tr_backend.c
    ${SOURCE_DIR}/renderergl1/tr_bsp.c
    ${SOURCE_DIR}/renderergl1/tr_cmds.c
    ${SOURCE_DIR}/renderergl1/tr_curve.c
    ${SOURCE_DIR}/renderergl1/tr_draw.c
    ${SOURCE_DIR}/renderergl1/tr_flares.c
    ${SOURCE_DIR}/renderergl1/tr_font.cpp
    ${SOURCE_DIR}/renderergl1/tr_ghost.cpp
    ${SOURCE_DIR}/renderergl1/tr_image.c
    ${SOURCE_DIR}/renderergl1/tr_init.c
    ${SOURCE_DIR}/renderergl1/tr_light.c
    ${SOURCE_DIR}/renderergl1/tr_main.c
    ${SOURCE_DIR}/renderergl1/tr_marks_permanent.c
    ${SOURCE_DIR}/renderergl1/tr_marks.c
    ${SOURCE_DIR}/renderergl1/tr_model.cpp
    ${SOURCE_DIR}/renderergl1/tr_scene.c
    ${SOURCE_DIR}/renderergl1/tr_shade_calc.c
    ${SOURCE_DIR}/renderergl1/tr_shade.c
    ${SOURCE_DIR}/renderergl1/tr_shader.c
    ${SOURCE_DIR}/renderergl1/tr_shadows.c
    ${SOURCE_DIR}/renderergl1/tr_sky_portal.cpp
    ${SOURCE_DIR}/renderergl1/tr_sky.c
    ${SOURCE_DIR}/renderergl1/tr_sphere_shade.cpp
    ${SOURCE_DIR}/renderergl1/tr_sprite.c
    ${SOURCE_DIR}/renderergl1/tr_staticmodels.cpp
    ${SOURCE_DIR}/renderergl1/tr_sun_flare.cpp
    ${SOURCE_DIR}/renderergl1/tr_surface.c
    ${SOURCE_DIR}/renderergl1/tr_swipe.cpp
    ${SOURCE_DIR}/renderergl1/tr_terrain.c
    ${SOURCE_DIR}/renderergl1/tr_util.cpp
    ${SOURCE_DIR}/renderergl1/tr_vis.cpp
    ${SOURCE_DIR}/renderergl1/tr_world.c
)

set(RENDERER_GL1_BASENAME renderer_opengl1)

if(USE_ARCHLESS_FILENAMES)
    set(RENDERER_GL1_BINARY ${RENDERER_GL1_BASENAME})
    list(APPEND RENDERER_DEFINITIONS USE_ARCHLESS_FILENAMES)
else()
    set(RENDERER_GL1_BINARY ${RENDERER_GL1_BASENAME}_${ARCH})
endif()

list(APPEND RENDERER_GL1_BINARY_SOURCES
    ${RENDERER_COMMON_SOURCES}
    ${RENDERER_GL1_SOURCES}
    ${SDL_RENDERER_SOURCES}
    ${RENDERER_LIBRARY_SOURCES})

if(USE_RENDERER_DLOPEN)
    list(APPEND RENDERER_GL1_BINARY_SOURCES ${DYNAMIC_RENDERER_SOURCES})

    add_library(${RENDERER_GL1_BINARY} SHARED ${RENDERER_GL1_BINARY_SOURCES})

    target_link_libraries(      ${RENDERER_GL1_BINARY} PRIVATE ${RENDERER_LIBRARIES})
    target_include_directories( ${RENDERER_GL1_BINARY} PRIVATE ${RENDERER_INCLUDE_DIRS})
    target_compile_definitions( ${RENDERER_GL1_BINARY} PRIVATE ${RENDERER_DEFINITIONS})
    target_compile_options(     ${RENDERER_GL1_BINARY} PRIVATE ${RENDERER_COMPILE_OPTIONS})
    target_link_options(        ${RENDERER_GL1_BINARY} PRIVATE ${RENDERER_LINK_OPTIONS})

    set_output_dirs(${RENDERER_GL1_BINARY})

	INSTALL(TARGETS ${RENDERER_GL1_BINARY} DESTINATION ${INSTALL_LIBDIR_FULL})

	if(MSVC)
		INSTALL(FILES $<TARGET_PDB_FILE:${RENDERER_GL1_BINARY}> DESTINATION ${INSTALL_LIBDIR_FULL} OPTIONAL)
	endif()
endif()
