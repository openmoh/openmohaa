/*
===========================================================================
Copyright (C) 2015 the OpenMoHAA team

This file is part of OpenMoHAA source code.

OpenMoHAA source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

OpenMoHAA source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with OpenMoHAA source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

// gamecvars.h: Game variables

#pragma once

#include "g_local.h"

#ifdef __cplusplus
extern "C" {
#endif

extern cvar_t *developer;

extern cvar_t *deathmatch;
extern cvar_t *dmflags;
extern cvar_t *skill;
extern cvar_t *fraglimit;
extern cvar_t *timelimit;
extern cvar_t *roundlimit;
extern cvar_t *g_allowjointime;
extern cvar_t *password;
extern cvar_t *sv_privatePassword;

extern cvar_t *filterban;

extern cvar_t *flood_msgs;
extern cvar_t *flood_persecond;
extern cvar_t *flood_waitdelay;

extern cvar_t *maxclients;
extern cvar_t *maxentities;
extern cvar_t *precache;
extern cvar_t *dedicated;
extern cvar_t *detail;
extern cvar_t *com_blood;
extern cvar_t *whereami;
extern cvar_t *bosshealth;

extern cvar_t *sv_maxvelocity;
extern cvar_t *sv_gravity;
extern cvar_t *sv_rollspeed;
extern cvar_t *sv_rollangle;
extern cvar_t *sv_cheats;
extern cvar_t *sv_showbboxes;
extern cvar_t *sv_showtags;

extern cvar_t *sv_testloc_num;
extern cvar_t *sv_testloc_secondary;
extern cvar_t *sv_testloc_radius;
extern cvar_t *sv_testloc_offset_x;
extern cvar_t *sv_testloc_offset_y;
extern cvar_t *sv_testloc_offset_z;
extern cvar_t *sv_testloc_radius2;
extern cvar_t *sv_testloc_offset2_x;
extern cvar_t *sv_testloc_offset2_y;
extern cvar_t *sv_testloc_offset2_z;

extern cvar_t *sv_showcameras;
extern cvar_t *sv_showentnums;
extern cvar_t *sv_stopspeed;
extern cvar_t *sv_friction;
extern cvar_t *sv_waterfriction;
extern cvar_t *sv_waterspeed;
extern cvar_t *sv_traceinfo;
extern cvar_t *sv_drawtrace;
extern cvar_t *sv_fps;
extern cvar_t *sv_cinematic;
extern cvar_t *sv_maplist;
extern cvar_t *sv_nextmap;
extern cvar_t *sv_runspeed;
extern cvar_t *sv_walkspeedmult;
extern cvar_t *sv_dmspeedmult;
extern cvar_t *sv_crouchspeedmult;

extern cvar_t *sv_sprinttime;
extern cvar_t *sv_sprintmult;
extern cvar_t *sv_sprinttime_dm;
extern cvar_t *sv_sprintmult_dm;
extern cvar_t *sv_sprinton;
extern cvar_t *sv_invulnerabletime;
extern cvar_t *sv_team_spawn_interval;

extern cvar_t *g_showmem;
extern cvar_t *g_timeents;
extern cvar_t *g_timescripts;

extern cvar_t *g_showaxis;
extern cvar_t *g_showplayerstate;
extern cvar_t *g_showplayeranim;
extern cvar_t *g_showawareness;
extern cvar_t *g_entinfo;
extern cvar_t *g_showlookat;

extern cvar_t *g_numdebuglines;
extern cvar_t *g_numdebuglinedelays;
extern cvar_t *g_numdebugstrings;

extern cvar_t *g_spiffyplayer;
extern cvar_t *g_spiffyvelocity_x;
extern cvar_t *g_spiffyvelocity_y;
extern cvar_t *g_spiffyvelocity_z;

extern cvar_t *g_playermodel;
extern cvar_t *g_statefile;
extern cvar_t *g_showbullettrace;
extern cvar_t *s_debugmusic;
extern cvar_t *g_showautoaim;
extern cvar_t *g_debugtargets;
extern cvar_t *g_debugdamage;
extern cvar_t *g_logstats;

extern cvar_t *g_showtokens;
extern cvar_t *g_showopcodes;
extern cvar_t *g_scriptcheck;
extern cvar_t *g_nodecheck;
extern cvar_t *g_scriptdebug;
extern cvar_t *g_scripttrace;

extern cvar_t *g_ai;
extern cvar_t *g_vehicle;

extern cvar_t *g_gametype;
extern cvar_t *g_gametypestring;
extern cvar_t *g_realismmode;
extern cvar_t *g_teamdamage;
extern cvar_t *g_healthdrop;
extern cvar_t *g_healrate;

extern cvar_t *g_allowvote;
extern cvar_t *g_monitor;
extern cvar_t *g_monitorNum;

extern cvar_t *g_spawnentities;
extern cvar_t *g_spawnai;

extern cvar_t *g_patherror;
extern cvar_t *g_droppeditemlife;
extern cvar_t *g_dropclips;
extern cvar_t *g_animdump;
extern cvar_t *g_showdamage;
extern cvar_t *g_ai_notifyradius;
extern cvar_t *g_ai_noticescale;
extern cvar_t *g_ai_soundscale;
extern cvar_t *ai_debug_grenades;

extern cvar_t *g_warmup;
extern cvar_t *g_doWarmup;
extern cvar_t *g_forceready;
extern cvar_t *g_forcerespawn;
extern cvar_t *g_maxintermission;

extern cvar_t *g_smoothClients;

extern cvar_t *pmove_fixed;
extern cvar_t *pmove_msec;

extern cvar_t *g_inactivespectate;
extern cvar_t *g_inactivekick;

extern cvar_t *g_teamkillwarn;
extern cvar_t *g_teamkillkick;
extern cvar_t *g_teamswitchdelay;

extern cvar_t *g_viewkick_pitch;
extern cvar_t *g_viewkick_yaw;
extern cvar_t *g_viewkick_roll;
extern cvar_t *g_viewkick_dmmult;

extern cvar_t *g_drawattackertime;
extern cvar_t *g_playerdeltamethod;

extern cvar_t *g_success;
extern cvar_t *g_failed;

extern cvar_t *g_gotmedal;
extern cvar_t *g_medal0;
extern cvar_t *g_medal1;
extern cvar_t *g_medal2;
extern cvar_t *g_medal3;
extern cvar_t *g_medal4;
extern cvar_t *g_medal5;
extern cvar_t *g_medalbt0;
extern cvar_t *g_medalbt1;
extern cvar_t *g_medalbt2;
extern cvar_t *g_medalbt3;
extern cvar_t *g_medalbt4;
extern cvar_t *g_medalbt5;
extern cvar_t *g_eogmedal0;
extern cvar_t *g_eogmedal1;
extern cvar_t *g_eogmedal2;
extern cvar_t *g_m1l1;
extern cvar_t *g_m1l2;
extern cvar_t *g_m1l3;
extern cvar_t *g_m2l1;
extern cvar_t *g_m2l2;
extern cvar_t *g_m2l3;
extern cvar_t *g_m3l1;
extern cvar_t *g_m3l2;
extern cvar_t *g_m3l3;
extern cvar_t *g_m4l1;
extern cvar_t *g_m4l2;
extern cvar_t *g_m4l3;
extern cvar_t *g_m5l1;
extern cvar_t *g_m5l2;
extern cvar_t *g_m5l3;
extern cvar_t *g_m6l1;
extern cvar_t *g_m6l2;
extern cvar_t *g_m6l3;
extern cvar_t *g_t2l1;
extern cvar_t *g_t3l1;
extern cvar_t *g_mission;
extern cvar_t *g_lastsave;

extern cvar_t *g_forceteamspectate;
extern cvar_t *g_spectatefollow_forward;
extern cvar_t *g_spectatefollow_right;
extern cvar_t *g_spectatefollow_up;
extern cvar_t *g_spectatefollow_pitch;
extern cvar_t *g_spectate_allow_full_chat;

extern cvar_t *g_voiceChat;
extern cvar_t *g_voiceChatTime;
extern cvar_t *g_textChatTime;
extern cvar_t *g_debugsmoke;

extern cvar_t *g_turret_convergetime;
extern cvar_t *g_turret_suppresstime;
extern cvar_t *g_turret_suppresswaittime;
extern cvar_t *g_turret_spread;
extern cvar_t *g_maprotation_filter;

extern cvar_t *g_shownpc;
extern cvar_t *g_TOW_winstate;
extern cvar_t *g_chat_expansions;

extern cvar_t *g_qunits_to_feet;
extern cvar_t *g_cinematics_off;

extern cvar_t *g_rifles_for_sweepers;
extern cvar_t *g_no_seasick;

extern cvar_t *g_aistats;

extern cvar_t *sv_netoptimize;

extern cvar_t *g_obituarylocation;

extern cvar_t *sv_scriptfiles;

extern cvar_t *sv_maxbots;
extern cvar_t *sv_numbots;
extern cvar_t *sv_minPlayers;
extern cvar_t *sv_sharedbots;

/**
 * @brief Minimum time to pause (bursting).
 */
extern cvar_t *g_bot_attack_burst_min_time;
/**
 * @brief Random time added to pause (bursting).
 */
extern cvar_t *g_bot_attack_burst_random_delay;
/**
 * @brief Minimum duration of continuous firing.
 */
extern cvar_t *g_bot_attack_continuousfire_min_firetime;
/**
 * @brief Random time added to the continuous firing duration.
 */
extern cvar_t *g_bot_attack_continuousfire_random_firetime;
/**
 * @brief The minimum time before the bot starts reacting.
 */
extern cvar_t *g_bot_attack_react_min_delay;
/**
 * @brief A random value added with the reaction delay.
 */
extern cvar_t *g_bot_attack_react_random_delay;
/**
 * @brief The accuracy.
 */
extern cvar_t *g_bot_attack_spreadmult;
/**
 * @brief The degrees per seconds when the bot turns.
 */
extern cvar_t *g_bot_turn_speed;
/**
 * @brief A value that determines the chance of the bot sending an instant message on kill.
 * 0 = disable
 * The higher the value, the lower the frequency of instant messages.
 */
extern cvar_t *g_bot_instamsg_chance;
/**
 * @brief The delay at which the bot can send an instant message again.
 */
extern cvar_t *g_bot_instamsg_delay;

extern cvar_t *g_rankedserver;
extern cvar_t *g_spectatefollow_firstperson;

extern cvar_t *cl_running;

extern cvar_t *g_instamsg_allowed;
extern cvar_t *g_instamsg_minDelay;
extern cvar_t *g_textmsg_allowed;
extern cvar_t *g_textmsg_minDelay;

extern cvar_t *g_teambalance;

extern cvar_t *g_navigation_legacy;

void CVAR_Init(void);

#ifdef __cplusplus
}
#endif
