cmake_minimum_required(VERSION 3.12)

project(Launcher)

set(LAUNCHER_SOURCES "${CMAKE_SOURCE_DIR}/code/Launcher/launch_main.cpp")

if (WIN32)
    set(LAUNCHER_SOURCES ${LAUNCHER_SOURCES}
    "${CMAKE_SOURCE_DIR}/code/Launcher/launch_win32.cpp"
    "${CMAKE_SOURCE_DIR}/code/sys/win_resource.rc")
else()
    set(LAUNCHER_SOURCES ${LAUNCHER_SOURCES}
    "${CMAKE_SOURCE_DIR}/code/Launcher/launch_linux.cpp")
endif()

function (create_launcher name type)
    if (NOT BUILD_NO_CLIENT)
        add_executable(openmohaa_launcher_${name} ${LAUNCHER_SOURCES})
        target_include_directories(openmohaa_launcher_${name} PUBLIC "../qcommon")
        target_compile_definitions(openmohaa_launcher_${name} PRIVATE NO_RC_MANIFEST=1 TARGET_GAME=${type})
        target_compile_features(openmohaa_launcher_${name} PRIVATE cxx_std_17)
        set_target_properties(openmohaa_launcher_${name} PROPERTIES OUTPUT_NAME "launch_openmohaa_${name}${TARGET_BIN_SUFFIX}")
        set_target_properties(openmohaa_launcher_${name} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

        INSTALL(TARGETS openmohaa_launcher_${name} DESTINATION ${CMAKE_INSTALL_BINDIR}/${BIN_INSTALL_SUBDIR})
    endif()

    #add_executable(omohaaded_launcher_${name} ${LAUNCHER_SOURCES})
    #target_include_directories(omohaaded_launcher_${name} PUBLIC "../qcommon")
    #target_compile_definitions(omohaaded_launcher_${name} PRIVATE NO_RC_MANIFEST=1 TARGET_GAME=${type} DEDICATED=1)
    #target_compile_features(omohaaded_launcher_${name} PRIVATE cxx_std_17)
    #set_target_properties(omohaaded_launcher_${name} PROPERTIES OUTPUT_NAME "launch_omohaaded_${name}${TARGET_BIN_SUFFIX}")
    #set_target_properties(omohaaded_launcher_${name} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
    #
    #INSTALL(TARGETS omohaaded_launcher_${name} DESTINATION ${CMAKE_INSTALL_BINDIR}/${PROJECT_INSTALL_SUBDIR})
endfunction()

create_launcher(base 0)
create_launcher(spearhead 1)
create_launcher(breakthrough 2)
