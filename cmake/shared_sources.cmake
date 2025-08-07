include_guard(GLOBAL)

include(utils/add_git_dependency)
include(utils/disable_warnings)
include(gamespy)

set(COMMON_SOURCES
    ${SOURCE_DIR}/qcommon/alias.c
    ${SOURCE_DIR}/qcommon/bg_compat.cpp
    ${SOURCE_DIR}/qcommon/cm_fencemask.c
    ${SOURCE_DIR}/qcommon/cm_load.c
    ${SOURCE_DIR}/qcommon/cm_patch.c
    ${SOURCE_DIR}/qcommon/cm_polylib.c
    ${SOURCE_DIR}/qcommon/cm_terrain.c
    ${SOURCE_DIR}/qcommon/cm_test.c
    ${SOURCE_DIR}/qcommon/cm_trace.c
    ${SOURCE_DIR}/qcommon/cm_trace_lbd.cpp
    ${SOURCE_DIR}/qcommon/cm_trace_obfuscation.cpp
    ${SOURCE_DIR}/qcommon/cmd.c
    ${SOURCE_DIR}/qcommon/common.c
    ${SOURCE_DIR}/qcommon/crc.c
    ${SOURCE_DIR}/qcommon/cvar.c
    ${SOURCE_DIR}/qcommon/files.cpp
    ${SOURCE_DIR}/qcommon/ioapi.c
    ${SOURCE_DIR}/qcommon/huffman.cpp
    ${SOURCE_DIR}/qcommon/md4.c
    ${SOURCE_DIR}/qcommon/md5.c
    ${SOURCE_DIR}/qcommon/memory.c
    ${SOURCE_DIR}/qcommon/msg.cpp
    ${SOURCE_DIR}/qcommon/net_chan.c
    ${SOURCE_DIR}/qcommon/net_ip.c
    ${SOURCE_DIR}/qcommon/q_math.c
    ${SOURCE_DIR}/qcommon/q_shared.c
    ${SOURCE_DIR}/qcommon/unzip.c
    ${SOURCE_DIR}/gamespy/q_gamespy.c
)

disable_warnings(
    ${SOURCE_DIR}/qcommon/unzip.c
    ${SOURCE_DIR}/qcommon/ioapi.c
)

add_git_dependency(${SOURCE_DIR}/qcommon/common.c)

set(SYSTEM_SOURCES
    ${SOURCE_DIR}/sys/con_log.c
    ${SOURCE_DIR}/sys/sys_autoupdater.c
    ${SOURCE_DIR}/sys/sys_main.c
    ${SYSTEM_PLATFORM_SOURCES}
)

set(SERVER_SOURCES
    ${SOURCE_DIR}/server/sv_client.c
    ${SOURCE_DIR}/server/sv_ccmds.c
    ${SOURCE_DIR}/server/sv_game.c
    ${SOURCE_DIR}/server/sv_init.c
    ${SOURCE_DIR}/server/sv_main.c
    ${SOURCE_DIR}/server/sv_net_chan.c
    ${SOURCE_DIR}/server/sv_snapshot.c
    ${SOURCE_DIR}/server/sv_snd.c
    ${SOURCE_DIR}/server/sv_world.c
)

list(APPEND SYSTEM_SOURCES
    ${SOURCE_DIR}/sys/new/sys_main_new.c
    ${SOURCE_DIR}/sys/win_bounds.cpp
    ${SOURCE_DIR}/sys/win_localization.cpp
    ${SOURCE_DIR}/sys/sys_curl.c
    ${SOURCE_DIR}/sys/sys_update_checker.cpp
)

# Append TIKI sources

list(APPEND COMMON_SOURCES
    ${SOURCE_DIR}/skeletor/bonetable.cpp
    ${SOURCE_DIR}/skeletor/skeletor_imports.cpp
    ${SOURCE_DIR}/skeletor/skeletor_loadanimation.cpp
    ${SOURCE_DIR}/skeletor/skeletor_model_files.cpp
    ${SOURCE_DIR}/skeletor/skeletor_utilities.cpp
    ${SOURCE_DIR}/skeletor/skeletor.cpp
    ${SOURCE_DIR}/skeletor/skeletorbones.cpp
    ${SOURCE_DIR}/skeletor/tokenizer.cpp
	${SOURCE_DIR}/tiki/tiki_anim.cpp
	${SOURCE_DIR}/tiki/tiki_cache.cpp
	${SOURCE_DIR}/tiki/tiki_commands.cpp
	${SOURCE_DIR}/tiki/tiki_files.cpp
	${SOURCE_DIR}/tiki/tiki_frame.cpp
	${SOURCE_DIR}/tiki/tiki_imports.cpp
	${SOURCE_DIR}/tiki/tiki_parse.cpp
	${SOURCE_DIR}/tiki/tiki_skel.cpp
	${SOURCE_DIR}/tiki/tiki_surface.cpp
	${SOURCE_DIR}/tiki/tiki_tag.cpp
	${SOURCE_DIR}/tiki/tiki_utility.cpp
	${SOURCE_DIR}/qcommon/tiki_main.cpp
	${SOURCE_DIR}/qcommon/tiki_script.cpp
)

# Gamespy

list(APPEND SERVER_SOURCES
	${SOURCE_DIR}/gamespy/sv_gamespy.c
	${SOURCE_DIR}/gamespy/sv_gqueryreporting.c
)

include_directories(${SOURCE_DIR}/qcommon)
include_directories(${SOURCE_DIR}/script)
