if(NOT BUILD_CLIENT)
    return()
endif()

include(utils/add_git_dependency)
include(utils/arch)
include(utils/set_output_dirs)
include(shared_sources)
include(shared_script)

include(renderer_common)

set(CLIENT_SOURCES
    ${SOURCE_DIR}/client/cl_avi.cpp
    ${SOURCE_DIR}/client/cl_cgame.cpp
    ${SOURCE_DIR}/client/cl_cin.cpp
    ${SOURCE_DIR}/client/cl_consolecmds.cpp
    ${SOURCE_DIR}/client/cl_curl.c
    ${SOURCE_DIR}/client/cl_input.cpp
    ${SOURCE_DIR}/client/cl_instantAction.cpp
    ${SOURCE_DIR}/client/cl_inv.cpp
    ${SOURCE_DIR}/client/cl_invrender.cpp
    ${SOURCE_DIR}/client/cl_keys.cpp
    ${SOURCE_DIR}/client/cl_main.cpp
    ${SOURCE_DIR}/client/cl_net_chan.cpp
    ${SOURCE_DIR}/client/cl_parse.cpp
    ${SOURCE_DIR}/client/cl_scrn.cpp
    ${SOURCE_DIR}/client/cl_ui.cpp
    ${SOURCE_DIR}/client/cl_uibind.cpp
    ${SOURCE_DIR}/client/cl_uidmbox.cpp
    ${SOURCE_DIR}/client/cl_uifilepicker.cpp
    ${SOURCE_DIR}/client/cl_uigamespy.cpp
    ${SOURCE_DIR}/client/cl_uigmbox.cpp
    ${SOURCE_DIR}/client/cl_uilangame.cpp
    ${SOURCE_DIR}/client/cl_uiloadsave.cpp
    ${SOURCE_DIR}/client/cl_uimaprotationsetup.cpp
    ${SOURCE_DIR}/client/cl_uimaprunner.cpp
    ${SOURCE_DIR}/client/cl_uiminicon.cpp
    ${SOURCE_DIR}/client/cl_uimpmappicker.cpp
    ${SOURCE_DIR}/client/cl_uiplayermodelpicker.cpp
    ${SOURCE_DIR}/client/cl_uiradar.cpp
    ${SOURCE_DIR}/client/cl_uiserverlist.cpp
    ${SOURCE_DIR}/client/cl_uisoundpicker.cpp
    ${SOURCE_DIR}/client/cl_uistd.cpp
    ${SOURCE_DIR}/client/cl_uiview3d.cpp
    ${SOURCE_DIR}/client/libmumblelink.c
    ${SOURCE_DIR}/client/qal.c
    ${SOURCE_DIR}/client/snd_codec_mp3.c
    ${SOURCE_DIR}/client/snd_codec_ogg.c
    ${SOURCE_DIR}/client/snd_codec_opus.c
    ${SOURCE_DIR}/client/snd_codec_wav.c
    ${SOURCE_DIR}/client/snd_codec.c
    ${SOURCE_DIR}/client/snd_dma_new.cpp
    ${SOURCE_DIR}/client/snd_info.cpp
    ${SOURCE_DIR}/client/snd_mem_new.cpp
    ${SOURCE_DIR}/client/snd_miles_new.cpp
    ${SOURCE_DIR}/client/snd_openal_new.cpp
    ${SOURCE_DIR}/client/usignal.cpp
    ${SOURCE_DIR}/sdl/sdl_input.c
    ${SOURCE_DIR}/sdl/sdl_mouse.c
    ${CLIENT_PLATFORM_SOURCES}
)

# Gamespy
list(APPEND CLIENT_SOURCES
	${SOURCE_DIR}/gamespy/cl_gamespy.c
)

file(GLOB_RECURSE UI_SOURCES "${SOURCE_DIR}/uilib/*.c" "${SOURCE_DIR}/uilib/*.cpp")

add_git_dependency(${SOURCE_DIR}/client/cl_console.c)

if(USE_ARCHLESS_FILENAMES)
    set(CLIENT_BINARY ${CLIENT_NAME})
    list(APPEND CLIENT_DEFINITIONS USE_ARCHLESS_FILENAMES)
else()
    set(CLIENT_BINARY ${CLIENT_NAME}.${ARCH})
endif()

list(APPEND CLIENT_DEFINITIONS BOTLIB)
list(APPEND CLIENT_DEFINITIONS APP_MODULE)

if(BUILD_STANDALONE)
    list(APPEND CLIENT_DEFINITIONS STANDALONE)
endif()

if(USE_RENDERER_DLOPEN)
    list(APPEND CLIENT_DEFINITIONS USE_RENDERER_DLOPEN)
endif()

if(USE_HTTP)
    list(APPEND CLIENT_DEFINITIONS USE_HTTP)
endif()

if(USE_VOIP)
    list(APPEND CLIENT_DEFINITIONS USE_VOIP)
endif()

if(USE_MUMBLE)
    list(APPEND CLIENT_DEFINITIONS USE_MUMBLE)
    list(APPEND CLIENT_LIBRARY_SOURCES ${SOURCE_DIR}/client/libmumblelink.c)
endif()

list(APPEND CLIENT_BINARY_SOURCES
    ${SERVER_SOURCES}
    ${CLIENT_SOURCES}
    ${UI_SOURCES}
    ${COMMON_SOURCES}
    ${SCRIPT_SYSTEM_SOURCES}
    ${BOTLIB_SOURCES}
    ${SYSTEM_SOURCES}
    ${ASM_SOURCES}
    ${CLIENT_LIBRARY_SOURCES})

add_executable(${CLIENT_BINARY} ${CLIENT_EXECUTABLE_OPTIONS} ${CLIENT_BINARY_SOURCES})

target_include_directories(     ${CLIENT_BINARY} PRIVATE ${CLIENT_INCLUDE_DIRS})
target_include_directories(     ${CLIENT_BINARY} PRIVATE ${SOURCE_DIR}/client)
target_compile_definitions(     ${CLIENT_BINARY} PRIVATE ${CLIENT_DEFINITIONS})
target_compile_options(         ${CLIENT_BINARY} PRIVATE ${CLIENT_COMPILE_OPTIONS})
target_link_libraries(          ${CLIENT_BINARY} PRIVATE ${COMMON_LIBRARIES} ${CLIENT_LIBRARIES})
target_link_options(            ${CLIENT_BINARY} PRIVATE ${CLIENT_LINK_OPTIONS})

set_output_dirs(${CLIENT_BINARY})

if(NOT USE_RENDERER_DLOPEN)
    target_sources(${CLIENT_BINARY} PRIVATE
        # These are never simultaneously populated
        ${RENDERER_GL1_BINARY_SOURCES}
        ${RENDERER_GL2_BINARY_SOURCES})

    target_include_directories( ${CLIENT_BINARY} PRIVATE ${RENDERER_INCLUDE_DIRS})
    target_compile_definitions( ${CLIENT_BINARY} PRIVATE ${RENDERER_DEFINITIONS})
    target_compile_options(     ${CLIENT_BINARY} PRIVATE ${RENDERER_COMPILE_OPTIONS})
    target_link_libraries(      ${CLIENT_BINARY} PRIVATE ${RENDERER_LIBRARIES})
endif()

foreach(LIBRARY IN LISTS CLIENT_DEPLOY_LIBRARIES)
    add_custom_command(TARGET ${CLIENT_BINARY} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${LIBRARY}
            $<TARGET_FILE_DIR:${CLIENT_BINARY}>)
endforeach()

if(POST_CLIENT_CONFIGURE_FUNCTION)
    cmake_language(CALL ${POST_CLIENT_CONFIGURE_FUNCTION})
endif()

set_target_properties(${CLIENT_BINARY} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

INSTALL(TARGETS ${CLIENT_BINARY} DESTINATION ${INSTALL_BINDIR_FULL})

if (MSVC)
	target_link_options(${CLIENT_BINARY} PRIVATE "/MANIFEST:NO")
	INSTALL(FILES $<TARGET_PDB_FILE:${CLIENT_BINARY}> DESTINATION ${INSTALL_BINDIR_FULL} OPTIONAL)
endif()
