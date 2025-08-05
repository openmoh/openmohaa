include_guard(GLOBAL)

include(utils/add_git_dependency)
include(utils/arch)
include(utils/disable_warnings)

set(COMMON_SOURCES
    ${SOURCE_DIR}/qcommon/cm_load.c
    ${SOURCE_DIR}/qcommon/cm_patch.c
    ${SOURCE_DIR}/qcommon/cm_polylib.c
    ${SOURCE_DIR}/qcommon/cm_test.c
    ${SOURCE_DIR}/qcommon/cm_trace.c
    ${SOURCE_DIR}/qcommon/cmd.c
    ${SOURCE_DIR}/qcommon/common.c
    ${SOURCE_DIR}/qcommon/cvar.c
    ${SOURCE_DIR}/qcommon/files.c
    ${SOURCE_DIR}/qcommon/md4.c
    ${SOURCE_DIR}/qcommon/md5.c
    ${SOURCE_DIR}/qcommon/msg.c
    ${SOURCE_DIR}/qcommon/net_chan.c
    ${SOURCE_DIR}/qcommon/net_ip.c
    ${SOURCE_DIR}/qcommon/huffman.c
    ${SOURCE_DIR}/qcommon/q_math.c
    ${SOURCE_DIR}/qcommon/q_shared.c
    ${SOURCE_DIR}/qcommon/unzip.c
    ${SOURCE_DIR}/qcommon/ioapi.c
    ${SOURCE_DIR}/qcommon/vm.c
    ${SOURCE_DIR}/qcommon/vm_interpreted.c
)

disable_warnings(
    ${SOURCE_DIR}/qcommon/unzip.c
    ${SOURCE_DIR}/qcommon/ioapi.c
)

add_git_dependency(${SOURCE_DIR}/qcommon/common.c)

if(ARCH MATCHES "x86" OR ARCH MATCHES "x86_64")
    list(APPEND COMMON_SOURCES
        ${SOURCE_DIR}/qcommon/vm_x86.c
    )
elseif(ARCH MATCHES "ppc" OR ARCH MATCHES "ppc64")
    list(APPEND COMMON_SOURCES
        ${SOURCE_DIR}/qcommon/vm_powerpc.c
        ${SOURCE_DIR}/qcommon/vm_powerpc_asm.c
    )
elseif(ARCH MATCHES "arm")
    list(APPEND COMMON_SOURCES
        ${SOURCE_DIR}/qcommon/vm_armv71.c
    )
elseif(ARCH MATCHES "sparc")
    list(APPEND COMMON_SOURCES
        ${SOURCE_DIR}/qcommon/vm_sparc.c
    )
else()
    list(APPEND SERVER_DEFINITIONS NO_VM_COMPILED)
    list(APPEND CLIENT_DEFINITIONS NO_VM_COMPILED)
endif()

set(SYSTEM_SOURCES
    ${SOURCE_DIR}/sys/con_log.c
    ${SOURCE_DIR}/sys/sys_autoupdater.c
    ${SOURCE_DIR}/sys/sys_main.c
    ${SYSTEM_PLATFORM_SOURCES}
)

set(SERVER_SOURCES
    ${SOURCE_DIR}/server/sv_bot.c
    ${SOURCE_DIR}/server/sv_client.c
    ${SOURCE_DIR}/server/sv_ccmds.c
    ${SOURCE_DIR}/server/sv_game.c
    ${SOURCE_DIR}/server/sv_init.c
    ${SOURCE_DIR}/server/sv_main.c
    ${SOURCE_DIR}/server/sv_net_chan.c
    ${SOURCE_DIR}/server/sv_snapshot.c
    ${SOURCE_DIR}/server/sv_world.c
)

set(BOTLIB_SOURCES
    ${SOURCE_DIR}/botlib/be_aas_bspq3.c
    ${SOURCE_DIR}/botlib/be_aas_cluster.c
    ${SOURCE_DIR}/botlib/be_aas_debug.c
    ${SOURCE_DIR}/botlib/be_aas_entity.c
    ${SOURCE_DIR}/botlib/be_aas_file.c
    ${SOURCE_DIR}/botlib/be_aas_main.c
    ${SOURCE_DIR}/botlib/be_aas_move.c
    ${SOURCE_DIR}/botlib/be_aas_optimize.c
    ${SOURCE_DIR}/botlib/be_aas_reach.c
    ${SOURCE_DIR}/botlib/be_aas_route.c
    ${SOURCE_DIR}/botlib/be_aas_routealt.c
    ${SOURCE_DIR}/botlib/be_aas_sample.c
    ${SOURCE_DIR}/botlib/be_ai_char.c
    ${SOURCE_DIR}/botlib/be_ai_chat.c
    ${SOURCE_DIR}/botlib/be_ai_gen.c
    ${SOURCE_DIR}/botlib/be_ai_goal.c
    ${SOURCE_DIR}/botlib/be_ai_move.c
    ${SOURCE_DIR}/botlib/be_ai_weap.c
    ${SOURCE_DIR}/botlib/be_ai_weight.c
    ${SOURCE_DIR}/botlib/be_ea.c
    ${SOURCE_DIR}/botlib/be_interface.c
    ${SOURCE_DIR}/botlib/l_crc.c
    ${SOURCE_DIR}/botlib/l_libvar.c
    ${SOURCE_DIR}/botlib/l_log.c
    ${SOURCE_DIR}/botlib/l_memory.c
    ${SOURCE_DIR}/botlib/l_precomp.c
    ${SOURCE_DIR}/botlib/l_script.c
    ${SOURCE_DIR}/botlib/l_struct.c
)
