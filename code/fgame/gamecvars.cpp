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

// gamecvars.cpp: Definitions for any cvars used by the game.
//

#include "gamecvars.h"

cvar_t *developer;

cvar_t *deathmatch;
cvar_t *dmflags;
cvar_t *skill;
cvar_t *fraglimit;
cvar_t *timelimit;
cvar_t *roundlimit;
cvar_t *g_allowjointime;
cvar_t *password;
cvar_t *sv_privatePassword;

cvar_t *filterban;

cvar_t *flood_msgs;
cvar_t *flood_persecond;
cvar_t *flood_waitdelay;

cvar_t *maxclients;
cvar_t *maxentities;
cvar_t *precache;
cvar_t *dedicated;
cvar_t *detail;
cvar_t *com_blood;
cvar_t *whereami;
cvar_t *bosshealth;

cvar_t *sv_maxvelocity;
cvar_t *sv_gravity;
cvar_t *sv_rollspeed;
cvar_t *sv_rollangle;
cvar_t *sv_cheats;
cvar_t *sv_showbboxes;
cvar_t *sv_showtags;

cvar_t *sv_testloc_num;
cvar_t *sv_testloc_secondary;
cvar_t *sv_testloc_radius;
cvar_t *sv_testloc_offset_x;
cvar_t *sv_testloc_offset_y;
cvar_t *sv_testloc_offset_z;
cvar_t *sv_testloc_radius2;
cvar_t *sv_testloc_offset2_x;
cvar_t *sv_testloc_offset2_y;
cvar_t *sv_testloc_offset2_z;

cvar_t *sv_showcameras;
cvar_t *sv_showentnums;
cvar_t *sv_stopspeed;
cvar_t *sv_friction;
cvar_t *sv_waterfriction;
cvar_t *sv_waterspeed;
cvar_t *sv_traceinfo;
cvar_t *sv_drawtrace;
cvar_t *sv_fps;
cvar_t *sv_cinematic;
cvar_t *sv_maplist;
cvar_t *sv_nextmap;
cvar_t *sv_runspeed;
cvar_t *sv_walkspeedmult;
cvar_t *sv_dmspeedmult;
cvar_t *sv_crouchspeedmult;

cvar_t *sv_sprinttime;
cvar_t *sv_sprintmult;
cvar_t *sv_sprinttime_dm;
cvar_t *sv_sprintmult_dm;
cvar_t *sv_sprinton;
cvar_t *sv_invulnerabletime;
cvar_t *sv_team_spawn_interval;

cvar_t *g_showmem;
cvar_t *g_timeents;
cvar_t *g_timescripts;

cvar_t *g_showaxis;
cvar_t *g_showplayerstate;
cvar_t *g_showplayeranim;
cvar_t *g_showawareness;
cvar_t *g_entinfo;
cvar_t *g_showlookat;

cvar_t *g_numdebuglines;
cvar_t *g_numdebuglinedelays;
cvar_t *g_numdebugstrings;

cvar_t *g_spiffyplayer;
cvar_t *g_spiffyvelocity_x;
cvar_t *g_spiffyvelocity_y;
cvar_t *g_spiffyvelocity_z;

cvar_t *g_playermodel;
cvar_t *g_statefile;
cvar_t *g_showbullettrace;
cvar_t *s_debugmusic;
cvar_t *g_showautoaim;
cvar_t *g_debugtargets;
cvar_t *g_debugdamage;
cvar_t *g_logstats;

cvar_t *g_showtokens;
cvar_t *g_showopcodes;
cvar_t *g_scriptcheck;
cvar_t *g_nodecheck;
cvar_t *g_scriptdebug;
cvar_t *g_scripttrace;

cvar_t *g_ai;
cvar_t *g_vehicle;

cvar_t *g_gametype;
cvar_t *g_gametypestring;
cvar_t *g_realismmode;
cvar_t *g_teamdamage;
cvar_t *g_healthdrop;
cvar_t *g_healrate;

cvar_t *g_allowvote;
cvar_t *g_monitor;
cvar_t *g_monitorNum;

cvar_t *g_spawnentities;
cvar_t *g_spawnai;

cvar_t *g_patherror;
cvar_t *g_droppeditemlife;
cvar_t *g_dropclips;
cvar_t *g_animdump;
cvar_t *g_showdamage;
cvar_t *g_ai_notifyradius;
cvar_t *g_ai_noticescale;
cvar_t *g_ai_soundscale;
cvar_t *ai_debug_grenades;

cvar_t *g_warmup;
cvar_t *g_doWarmup;
cvar_t *g_forceready;
cvar_t *g_forcerespawn;
cvar_t *g_maxintermission;

cvar_t *g_smoothClients;

cvar_t *pmove_fixed;
cvar_t *pmove_msec;

cvar_t *g_inactivespectate;
cvar_t *g_inactivekick;

cvar_t *g_teamkillwarn;
cvar_t *g_teamkillkick;
cvar_t *g_teamswitchdelay;

cvar_t *g_viewkick_pitch;
cvar_t *g_viewkick_yaw;
cvar_t *g_viewkick_roll;
cvar_t *g_viewkick_dmmult;

cvar_t *g_drawattackertime;
cvar_t *g_playerdeltamethod;

cvar_t *g_success;
cvar_t *g_failed;

cvar_t *g_gotmedal;
cvar_t *g_medal0;
cvar_t *g_medal1;
cvar_t *g_medal2;
cvar_t *g_medal3;
cvar_t *g_medal4;
cvar_t *g_medal5;
cvar_t *g_medalbt0;
cvar_t *g_medalbt1;
cvar_t *g_medalbt2;
cvar_t *g_medalbt3;
cvar_t *g_medalbt4;
cvar_t *g_medalbt5;
cvar_t *g_eogmedal0;
cvar_t *g_eogmedal1;
cvar_t *g_eogmedal2;
cvar_t *g_m1l1;
cvar_t *g_m1l2;
cvar_t *g_m1l3;
cvar_t *g_m2l1;
cvar_t *g_m2l2;
cvar_t *g_m2l3;
cvar_t *g_m3l1;
cvar_t *g_m3l2;
cvar_t *g_m3l3;
cvar_t *g_m4l1;
cvar_t *g_m4l2;
cvar_t *g_m4l3;
cvar_t *g_m5l1;
cvar_t *g_m5l2;
cvar_t *g_m5l3;
cvar_t *g_m6l1;
cvar_t *g_m6l2;
cvar_t *g_m6l3;
cvar_t *g_t2l1;
cvar_t *g_t3l1;
cvar_t *g_mission;
cvar_t *g_lastsave;

cvar_t *g_forceteamspectate;
cvar_t *g_spectatefollow_forward;
cvar_t *g_spectatefollow_right;
cvar_t *g_spectatefollow_up;
cvar_t *g_spectatefollow_pitch;
cvar_t *g_spectate_allow_full_chat;

cvar_t *g_voiceChat;
cvar_t *g_voiceChatTime;
cvar_t *g_textChatTime;
cvar_t *g_debugsmoke;

cvar_t *g_turret_convergetime;
cvar_t *g_turret_suppresstime;
cvar_t *g_turret_suppresswaittime;
cvar_t *g_turret_spread;
cvar_t *g_maprotation_filter;

cvar_t *g_shownpc;
cvar_t *g_TOW_winstate;
cvar_t *g_chat_expansions;

cvar_t *g_qunits_to_feet;
cvar_t *g_cinematics_off;

cvar_t *g_rifles_for_sweepers;
cvar_t *g_no_seasick;

cvar_t *g_aistats;

//
// Added in OPM
//

cvar_t *sv_netoptimize;

cvar_t *g_obituarylocation;

cvar_t *sv_scriptfiles;

// The maximum number of allocated bot clients
cvar_t *sv_maxbots;
// The number of bots that should be spawned
cvar_t *sv_numbots;
// The minimum number of players that should be present in-game.
//  If the number of real players is below this number,
//  the game will automatically add bots to fill the gap
cvar_t *sv_minPlayers;
// Whether or not the bots use a shared player slots
//  NOTE: Setting this cvar is not recommended
//  because when a client connects and the slot is used by a bot
//  the bot will be relocated to a free entity slot
cvar_t *sv_sharedbots;

cvar_t *g_bot_attack_burst_min_time;
cvar_t *g_bot_attack_burst_random_delay;
cvar_t *g_bot_attack_continuousfire_min_firetime;
cvar_t *g_bot_attack_continuousfire_random_firetime;
cvar_t *g_bot_attack_react_min_delay;
cvar_t *g_bot_attack_react_random_delay;
cvar_t *g_bot_attack_spreadmult;
cvar_t *g_bot_turn_speed;
cvar_t *g_bot_instamsg_chance;
cvar_t *g_bot_instamsg_delay;

cvar_t *g_rankedserver;
cvar_t *g_spectatefollow_firstperson;

cvar_t *cl_running;

// Whether or instant messages are allowed
cvar_t *g_instamsg_allowed;
// Minimum delay, in milliseconds, between instant messages
cvar_t *g_instamsg_minDelay;
// Whether or not text messages are allowed
cvar_t *g_textmsg_allowed;
// Minimum delay, in milliseconds, between messages
cvar_t *g_textmsg_minDelay;

// Whether or not to prevent teams from being unbalanced
cvar_t *g_teambalance;

// Whether or not to use Legacy Navigation
cvar_t *g_navigation_legacy;

void CVAR_Init(void)
{
    int i;

    developer = gi.Cvar_Get("developer", "0", 0);

    precache = gi.Cvar_Get("sv_precache", "1", 0);

    dedicated  = gi.Cvar_Get("dedicated", "1", CVAR_LATCH);
    deathmatch = gi.Cvar_Get("deathmatch", "1", CVAR_USERINFO | CVAR_SERVERINFO | CVAR_LATCH);
    skill      = gi.Cvar_Get("skill", "1", CVAR_USERINFO | CVAR_SERVERINFO | CVAR_LATCH);

    maxclients  = gi.Cvar_Get("sv_maxclients", "1", 0);
    maxentities = gi.Cvar_Get("maxentities", "1024", CVAR_LATCH);

    password           = gi.Cvar_Get("password", "", CVAR_USERINFO);
    sv_privatePassword = gi.Cvar_Get("sv_privatePassword", "", CVAR_TEMP);
    filterban          = gi.Cvar_Get("filterban", "1", 0);

    dmflags           = gi.Cvar_Get("dmflags", "0", CVAR_SERVERINFO);
    fraglimit         = gi.Cvar_Get("fraglimit", "0", CVAR_SERVERINFO);
    timelimit         = gi.Cvar_Get("timelimit", "0", CVAR_SERVERINFO);
    roundlimit        = gi.Cvar_Get("roundlimit", "0", CVAR_SERVERINFO);
    g_allowjointime   = gi.Cvar_Get("g_allowjointime", "30", CVAR_SERVERINFO);
    g_teamswitchdelay = gi.Cvar_Get("g_teamswitchdelay", "15", 0);

    flood_msgs      = gi.Cvar_Get("flood_msgs", "4", 0);
    flood_persecond = gi.Cvar_Get("flood_persecond", "4", 0);
    flood_waitdelay = gi.Cvar_Get("flood_waitdelay", "10", 0);

    detail     = gi.Cvar_Get("detail", "1", CVAR_ARCHIVE);
    com_blood  = gi.Cvar_Get("com_blood", "1", CVAR_ARCHIVE);
    whereami   = gi.Cvar_Get("whereami", "0", 0);
    bosshealth = gi.Cvar_Get("bosshealth", "0", 0);

    sv_rollspeed   = gi.Cvar_Get("sv_rollspeed", "2200", 0);
    sv_rollangle   = gi.Cvar_Get("sv_rollangle", "2", 0);
    sv_maxvelocity = gi.Cvar_Get("sv_maxvelocity", "6000", 0);
    sv_gravity     = gi.Cvar_Get("sv_gravity", "512", 0);

    sv_traceinfo  = gi.Cvar_Get("sv_traceinfo", "0", 0);
    sv_drawtrace  = gi.Cvar_Get("sv_drawtrace", "0", 0);
    sv_showbboxes = gi.Cvar_Get("sv_showbboxes", "0", 0);
    sv_showtags   = gi.Cvar_Get("sv_showtags", "0", 0);

    sv_testloc_num       = gi.Cvar_Get("sv_testloc_num", "0", 0);
    sv_testloc_secondary = gi.Cvar_Get("sv_testloc_secondary", "0", 0);
    sv_testloc_radius    = gi.Cvar_Get("sv_testloc_radius", "1", 0);
    sv_testloc_offset_x  = gi.Cvar_Get("sv_testloc_offset_x", "0", 0);
    sv_testloc_offset_y  = gi.Cvar_Get("sv_testloc_offset_y", "0", 0);
    sv_testloc_offset_z  = gi.Cvar_Get("sv_testloc_offset_z", "0", 0);
    sv_testloc_radius2   = gi.Cvar_Get("sv_testloc_radius2", "1", 0);
    sv_testloc_offset2_x = gi.Cvar_Get("sv_testloc_offset2_x", "0", 0);
    sv_testloc_offset2_y = gi.Cvar_Get("sv_testloc_offset2_y", "0", 0);
    sv_testloc_offset2_z = gi.Cvar_Get("sv_testloc_offset2_z", "0", 0);

    sv_showcameras = gi.Cvar_Get("sv_showcameras", "0", 0);
    sv_showentnums = gi.Cvar_Get("sv_showentnums", "0", 0);

    sv_friction      = gi.Cvar_Get("sv_friction", "4", 0);
    sv_stopspeed     = gi.Cvar_Get("sv_stopspeed", "100", 0);
    sv_waterfriction = gi.Cvar_Get("sv_waterfriction", "1", 0);
    sv_waterspeed    = gi.Cvar_Get("sv_waterspeed", "400", 0);

    sv_cheats    = gi.Cvar_Get("cheats", "0", CVAR_USERINFO | CVAR_SERVERINFO | CVAR_LATCH);
    sv_fps       = gi.Cvar_Get("sv_fps", "20", CVAR_SAVEGAME);
    sv_cinematic = gi.Cvar_Get("sv_cinematic", "0", CVAR_ROM);

    sv_maplist = gi.Cvar_Get("sv_maplist", "", CVAR_ARCHIVE | CVAR_SERVERINFO);
    sv_nextmap = gi.Cvar_Get("nextmap", "", 0);

    sv_runspeed        = gi.Cvar_Get("sv_runspeed", "287", 0); // 250 in mohaa
    sv_walkspeedmult   = gi.Cvar_Get("sv_walkspeedmult", "0.6", 0);
    sv_dmspeedmult     = gi.Cvar_Get("sv_dmspeedmult", "1.1", 0);
    sv_crouchspeedmult = gi.Cvar_Get("sv_crouchspeedmult", "0.6", 0);

    sv_sprinttime    = gi.Cvar_Get("sv_sprinttime", "5.0", 0);
    sv_sprintmult    = gi.Cvar_Get("sv_sprintmult", "1.20", 0);
    sv_sprinttime_dm = gi.Cvar_Get("sv_sprinttime_dm", "5.0", 0);
    sv_sprintmult_dm = gi.Cvar_Get("sv_sprintmult_dm", "1.20", 0);

    if (g_target_game >= target_game_e::TG_MOHTA) {
        sv_sprinton = gi.Cvar_Get("sv_sprinton", "1", 0);
    } else {
        // mohaa doesn't have sprint support
        sv_sprinton = gi.Cvar_Get("sv_sprinton", "0", 0);
    }

    if (!sv_sprinton->integer && sv_runspeed->integer == 287) {
        gi.cvar_set("sv_runspeed", "250");
    }

    if (g_target_game >= target_game_e::TG_MOHTA) {
        sv_invulnerabletime    = gi.Cvar_Get("sv_invulnerabletime", "3.0", CVAR_ARCHIVE | CVAR_SERVERINFO);
        sv_team_spawn_interval = gi.Cvar_Get("sv_team_spawn_interval", "15", CVAR_ARCHIVE | CVAR_SERVERINFO);
    } else {
        sv_invulnerabletime    = gi.Cvar_Get("sv_invulnerabletime", "0", CVAR_ARCHIVE | CVAR_SERVERINFO);
        sv_team_spawn_interval = gi.Cvar_Get("sv_team_spawn_interval", "0", CVAR_ARCHIVE | CVAR_SERVERINFO);
    }

    g_showmem         = gi.Cvar_Get("g_showmem", "0", 0);
    g_timeents        = gi.Cvar_Get("g_timeents", "0", 0);
    g_timescripts     = gi.Cvar_Get("g_timescripts", "0", 0);
    g_showaxis        = gi.Cvar_Get("g_showaxis", "0", 0);
    g_showplayerstate = gi.Cvar_Get("g_showplayerstate", "0", 0);
    g_showplayeranim  = gi.Cvar_Get("g_showplayeranim", "0", 0);
    g_showbullettrace = gi.Cvar_Get("g_showbullettrace", "0", 0);
    g_showawareness   = gi.Cvar_Get("g_showawareness", "0", CVAR_CHEAT);
    g_entinfo         = gi.Cvar_Get("g_entinfo", "0", CVAR_CHEAT);
    g_showlookat      = gi.Cvar_Get("g_showlookat", "0", CVAR_CHEAT);

    g_numdebuglines      = gi.Cvar_Get("g_numdebuglines", "4096", CVAR_LATCH);
    g_numdebuglinedelays = gi.Cvar_Get("g_numdebuglinedelays", "0", 0);
    g_numdebugstrings    = gi.Cvar_Get("g_numdebugstrings", "256", CVAR_LATCH);

    g_spiffyplayer     = gi.Cvar_Get("g_spiffyplayer", "0", CVAR_LATCH);
    g_spiffyvelocity_x = gi.Cvar_Get("g_spiffyvelocity_x", "0", 0);
    g_spiffyvelocity_y = gi.Cvar_Get("g_spiffyvelocity_y", "0", 0);
    g_spiffyvelocity_z = gi.Cvar_Get("g_spiffyvelocity_z", "0", 0);

    g_playermodel = gi.Cvar_Get("g_playermodel", "american_army", CVAR_SAVEGAME);
    g_statefile   = gi.Cvar_Get("g_statefile", "global/mike", 0);

    g_showautoaim  = gi.Cvar_Get("g_showautoaim", "0", 0);
    g_debugtargets = gi.Cvar_Get("g_debugtargets", "0", 0);
    g_debugdamage  = gi.Cvar_Get("g_debugdamage", "0", 0);
    g_logstats     = gi.Cvar_Get("g_logstats", "0", 0);
    g_showtokens   = gi.Cvar_Get("g_showtokens", "0", 0);
    g_showopcodes  = gi.Cvar_Get("g_showopcodes", "0", 0);
    g_scriptcheck  = gi.Cvar_Get("g_scriptcheck", "0", 0);
    g_nodecheck    = gi.Cvar_Get("g_nodecheck", "0", 0);
    g_scriptdebug  = gi.Cvar_Get("g_scriptdebug", "0", 0);
    g_scripttrace  = gi.Cvar_Get("g_scripttrace", "0", 0);

    g_ai      = gi.Cvar_Get("g_ai", "1", 0);
    g_vehicle = gi.Cvar_Get("g_vehicle", "1", 0);

    g_monitor    = gi.Cvar_Get("g_monitor", "", 0);
    g_monitorNum = gi.Cvar_Get("g_monitorNum", "-1", 0);

    g_spawnentities = gi.Cvar_Get("g_spawnentities", "1", 0);
    g_spawnai       = gi.Cvar_Get("g_spawnai", "1", 0);

    g_patherror = gi.Cvar_Get("g_patherror", "1", 0);

    g_droppeditemlife = gi.Cvar_Get("g_droppeditemlife", "30", 0);
    g_dropclips       = gi.Cvar_Get("g_dropclips", "2", 0);

    g_animdump = gi.Cvar_Get("g_animdump", "0", 0);

    g_showdamage = gi.Cvar_Get("g_showdamage", "0", 0);

    g_ai_notifyradius = gi.Cvar_Get("g_ai_notifyradius", "1024", 0);
    g_ai_noticescale  = gi.Cvar_Get("g_ai_noticescale", "1", 0);
    g_ai_soundscale   = gi.Cvar_Get("g_ai_soundscale", "1", 0);
    ai_debug_grenades = gi.Cvar_Get("ai_debug_grenades", "0", CVAR_CHEAT);

    g_gametype       = gi.Cvar_Get("g_gametype", "0", CVAR_USERINFO | CVAR_SERVERINFO | CVAR_LATCH);
    g_gametypestring = gi.Cvar_Get("g_gametypestring", "Free-For-All", CVAR_SERVERINFO);
    g_realismmode    = gi.Cvar_Get("g_realismmode", "0", CVAR_USERINFO | CVAR_SERVERINFO | CVAR_LATCH);
    if (!g_gametype->integer) {
        gi.cvar_set("g_realismmode", "0");
    }

    g_teamdamage = gi.Cvar_Get("g_teamdamage", "0", 0);
    g_healthdrop = gi.Cvar_Get("g_healthdrop", "1", 0);
    if (g_protocol >= protocol_e::PROTOCOL_MOHTA_MIN) {
        g_healrate = gi.Cvar_Get("g_healrate", "10", 0);
    } else {
        //
        // By default, no healrate on 1.11 and below
        //
        g_healrate = gi.Cvar_Get("g_healrate", "0", 0);
    }
    g_allowvote          = gi.Cvar_Get("g_allowvote", "1", CVAR_SERVERINFO);
    g_maprotation_filter = gi.Cvar_Get("g_maprotation_filter", "ffa", 0);
    g_warmup             = gi.Cvar_Get("g_warmup", "20", CVAR_ARCHIVE);
    g_doWarmup           = gi.Cvar_Get("g_doWarmup", "1", 0);

    g_forceready      = gi.Cvar_Get("g_forceready", "1", CVAR_ARCHIVE | CVAR_SERVERINFO);
    g_forcerespawn    = gi.Cvar_Get("g_forcerespawn", "0", CVAR_ARCHIVE | CVAR_SERVERINFO);
    g_maxintermission = gi.Cvar_Get("g_maxintermission", "15", CVAR_ARCHIVE | CVAR_SERVERINFO);

    g_smoothClients = gi.Cvar_Get("g_smoothClients", "1", 0);
    pmove_fixed     = gi.Cvar_Get("pmove_fixed", "0", CVAR_SYSTEMINFO);
    pmove_msec      = gi.Cvar_Get("pmove_msec", "0", CVAR_SYSTEMINFO);

    g_inactivespectate = gi.Cvar_Get("g_inactiveSpectate", "60", CVAR_ARCHIVE);
    g_inactivekick     = gi.Cvar_Get("g_inactivekick", "900", CVAR_ARCHIVE);
    g_teamkillwarn     = gi.Cvar_Get("g_teamkillwarn", "3", CVAR_ARCHIVE);
    g_teamkillkick     = gi.Cvar_Get("g_teamkillkick", "5", CVAR_ARCHIVE);
    g_teamswitchdelay  = gi.Cvar_Get("g_teamswitchdelay", "15", CVAR_ARCHIVE);
    g_shownpc          = gi.Cvar_Get("g_shownpc", "0", 0);

    // Removed in OPM
    //if (developer->integer) {
    //    g_shownpc = gi.Cvar_Get("g_shownpc", "1", 0);
    //} else {
    //    g_shownpc = gi.Cvar_Get("g_shownpc", "0", 0);
    //}

    s_debugmusic = gi.Cvar_Get("s_debugmusic", "0", 0);

    if (g_gametype->integer == GT_SINGLE_PLAYER) {
        gi.cvar_set("deathmatch", "0");
    } else {
        gi.cvar_set("deathmatch", "1");
    }

    if (g_protocol >= protocol_e::PROTOCOL_MOHTA_MIN) {
        if (gi.Cvar_Get("com_target_game", "", 0)->integer == target_game_e::TG_MOHTT) {
            // Set the server type to mohaab
            gi.cvar_set("g_servertype", va("%d", target_game_e::TG_MOHTT));
        }
    }

    switch (g_gametype->integer) {
    default:
    case GT_FFA:
        gi.cvar_set("g_gametypestring", "Free-For-All");
        break;
    case GT_TEAM:
        gi.cvar_set("g_gametypestring", "Team-Match");
        break;
    case GT_TEAM_ROUNDS:
        gi.cvar_set("g_gametypestring", "Round-Based-Match");
        break;
    case GT_OBJECTIVE:
        gi.cvar_set("g_gametypestring", "Objective-Match");
        break;
    case GT_TOW:
        gi.cvar_set("g_gametypestring", "Tug-of-War");
        break;
    case GT_LIBERATION:
        gi.cvar_set("g_gametypestring", "Liberation");
        break;
    case GT_SINGLE_PLAYER:
        gi.cvar_set("g_gametypestring", "Multiplayer");
        break;
    }

    g_viewkick_pitch  = gi.Cvar_Get("g_viewkick_pitch", "0.3", CVAR_CHEAT);
    g_viewkick_yaw    = gi.Cvar_Get("g_viewkick_yaw", "0.3", CVAR_CHEAT);
    g_viewkick_roll   = gi.Cvar_Get("g_viewkick_roll", "0.15", CVAR_CHEAT);
    g_viewkick_dmmult = gi.Cvar_Get("g_viewkick_dmmult", "1.0", CVAR_CHEAT);

    g_drawattackertime  = gi.Cvar_Get("g_drawattackertime", "5", 0);
    g_playerdeltamethod = gi.Cvar_Get("g_playerdeltamethod", "0", 0);

    g_success  = gi.Cvar_Get("g_success", "0", 0);
    g_failed   = gi.Cvar_Get("g_failed", "0", 0);
    g_gotmedal = gi.Cvar_Get("g_gotmedal", "0", 0);

    g_medal0   = gi.Cvar_Get("g_medal0", "0", CVAR_ARCHIVE);
    g_medal1   = gi.Cvar_Get("g_medal1", "0", CVAR_ARCHIVE);
    g_medal2   = gi.Cvar_Get("g_medal2", "0", CVAR_ARCHIVE);
    g_medal3   = gi.Cvar_Get("g_medal3", "0", CVAR_ARCHIVE);
    g_medal4   = gi.Cvar_Get("g_medal4", "0", CVAR_ARCHIVE);
    g_medal5   = gi.Cvar_Get("g_medal5", "0", CVAR_ARCHIVE);
    g_medalbt0 = gi.Cvar_Get("g_medalbt0", "0", CVAR_ARCHIVE);
    g_medalbt1 = gi.Cvar_Get("g_medalbt1", "0", CVAR_ARCHIVE);
    g_medalbt2 = gi.Cvar_Get("g_medalbt2", "0", CVAR_ARCHIVE);
    g_medalbt3 = gi.Cvar_Get("g_medalbt3", "0", CVAR_ARCHIVE);
    g_medalbt4 = gi.Cvar_Get("g_medalbt4", "0", CVAR_ARCHIVE);
    g_medalbt5 = gi.Cvar_Get("g_medalbt5", "0", CVAR_ARCHIVE);

    g_eogmedal0 = gi.Cvar_Get("g_eogmedal0", "0", CVAR_ARCHIVE);
    g_eogmedal1 = gi.Cvar_Get("g_eogmedal1", "0", CVAR_ARCHIVE);
    g_eogmedal2 = gi.Cvar_Get("g_eogmedal2", "0", CVAR_ARCHIVE);

    g_m1l1 = gi.Cvar_Get("g_m1l1", "1", CVAR_ARCHIVE);
    g_m1l2 = gi.Cvar_Get("g_m1l2", "0", CVAR_ARCHIVE);
    g_m1l3 = gi.Cvar_Get("g_m1l3", "0", CVAR_ARCHIVE);
    g_m2l1 = gi.Cvar_Get("g_m2l1", "0", CVAR_ARCHIVE);
    g_m2l2 = gi.Cvar_Get("g_m2l2", "0", CVAR_ARCHIVE);
    g_m2l3 = gi.Cvar_Get("g_m2l3", "0", CVAR_ARCHIVE);
    g_m3l1 = gi.Cvar_Get("g_m3l1", "0", CVAR_ARCHIVE);
    g_m3l2 = gi.Cvar_Get("g_m3l2", "0", CVAR_ARCHIVE);
    g_m3l3 = gi.Cvar_Get("g_m3l3", "0", CVAR_ARCHIVE);
    g_m4l1 = gi.Cvar_Get("g_m4l1", "0", CVAR_ARCHIVE);
    g_m4l2 = gi.Cvar_Get("g_m4l2", "0", CVAR_ARCHIVE);
    g_m4l3 = gi.Cvar_Get("g_m4l3", "0", CVAR_ARCHIVE);
    g_m5l1 = gi.Cvar_Get("g_m5l1", "0", CVAR_ARCHIVE);
    g_m5l2 = gi.Cvar_Get("g_m5l2", "0", CVAR_ARCHIVE);
    g_m5l3 = gi.Cvar_Get("g_m5l3", "0", CVAR_ARCHIVE);
    g_m6l1 = gi.Cvar_Get("g_m6l1", "0", CVAR_ARCHIVE);
    g_m6l2 = gi.Cvar_Get("g_m6l2", "0", CVAR_ARCHIVE);
    g_m6l3 = gi.Cvar_Get("g_m6l3", "0", CVAR_ARCHIVE);
    g_t2l1 = gi.Cvar_Get("g_t2l1", "0", CVAR_ARCHIVE);
    g_t3l1 = gi.Cvar_Get("g_t3l1", "0", CVAR_ARCHIVE);

    g_mission = (gi.Cvar_Get)("g_mission", "0", CVAR_ARCHIVE);

    g_lastsave                 = gi.Cvar_Get("g_lastsave", "", CVAR_ARCHIVE);
    g_forceteamspectate        = gi.Cvar_Get("g_forceteamspectate", "1", 0);
    g_spectatefollow_forward   = gi.Cvar_Get("g_spectatefollow_forward", "-56", 0);
    g_spectatefollow_right     = gi.Cvar_Get("g_spectatefollow_right", "0", 0);
    g_spectatefollow_up        = gi.Cvar_Get("g_spectatefollow_up", "24", 0);
    g_spectatefollow_pitch     = gi.Cvar_Get("g_spectatefollow_pitch", "2", 0);
    g_spectate_allow_full_chat = gi.Cvar_Get("g_spectate_allow_full_chat", "0", 0);

    g_voiceChat     = gi.Cvar_Get("g_voiceChat", "1", 0);
    g_voiceChatTime = gi.Cvar_Get("g_voiceChatTime", "2.5", 0);
    g_textChatTime  = gi.Cvar_Get("g_textChatTime", "1", 0);
    g_debugsmoke    = gi.Cvar_Get("g_debugsmoke", "0", CVAR_CHEAT);

    for (i = 1; i <= 5; i++) {
        gi.Cvar_Get(va("g_obj_alliedtext%d", i), "", CVAR_SERVERINFO);
        gi.Cvar_Get(va("g_obj_axistext%d", i), "", CVAR_SERVERINFO);
    }

    gi.Cvar_Get("g_scoreboardpic", "", CVAR_SERVERINFO);
    gi.Cvar_Get("g_scoreboardpicover", "", CVAR_SERVERINFO);

    if (g_gametype->integer > GT_TEAM) {
        gi.cvar_set("g_tempaxisscore", "0");
        gi.cvar_set("g_tempaxiswinsinrow", "0");
        gi.cvar_set("g_tempalliesscore", "0");
        gi.cvar_set("g_tempallieswinsinrow", "0");
        gi.cvar_set("g_tempmaptime", "0");
        gi.cvar_set("g_scoreboardpicover", "");
    }

    g_turret_convergetime     = gi.Cvar_Get("g_turret_convergetime", "2", 0);
    g_turret_suppresstime     = gi.Cvar_Get("g_turret_suppresstime", "2", 0);
    g_turret_suppresswaittime = gi.Cvar_Get("g_turret_suppresswaittime", "2", 0);
    g_turret_spread           = gi.Cvar_Get("g_turret_spread", "16", 0);
    g_TOW_winstate            = gi.Cvar_Get("g_TOW_winstate", "0", 0);
    g_chat_expansions         = gi.Cvar_Get("g_chat_expansions", "1", 0);
    g_qunits_to_feet          = gi.Cvar_Get("g_qunits_to_feet", "1", 0);
    g_aistats                 = gi.Cvar_Get("g_aistats", "0", CVAR_CHEAT);
    g_cinematics_off          = gi.Cvar_Get("g_cinematics_off", "0", 0);
    g_rifles_for_sweepers     = gi.Cvar_Get("g_rifles_for_sweepers", "0", 0);
    g_no_seasick              = gi.Cvar_Get("g_no_seasick", "0", 0);

    //
    // Added in OPM
    //  Clear the text objectives and the scoreboard pic
    //
    for (i = 1; i <= 5; i++) {
        gi.cvar_set(va("g_obj_alliedtext%d", i), "");
        gi.cvar_set(va("g_obj_axistext%d", i), "");
    }
    gi.cvar_set("g_scoreboardpic", "");

    sv_netoptimize = gi.Cvar_Get("sv_netoptimize", "0", 0);

    if (g_target_game >= target_game_e::TG_MOHTA) {
        g_obituarylocation = gi.Cvar_Get("g_obituarylocation", "0", 0);
    } else {
        // Defaults to 1 on vanilla mohaa
        g_obituarylocation = gi.Cvar_Get("g_obituarylocation", "1", 0);
    }

    sv_scriptfiles = gi.Cvar_Get("sv_scriptfiles", "0", 0);
    sv_maxbots     = gi.Cvar_Get("sv_maxbots", "0", CVAR_LATCH);
    sv_sharedbots  = gi.Cvar_Get("sv_sharedbots", "0", CVAR_LATCH);
    sv_numbots     = gi.Cvar_Get("sv_numbots", "0", 0);
    sv_minPlayers  = gi.Cvar_Get("sv_minPlayers", "0", 0);

    g_bot_attack_burst_min_time                = gi.Cvar_Get("g_bot_attack_burst_min_time", "0.1", 0);
    g_bot_attack_burst_random_delay            = gi.Cvar_Get("g_bot_attack_burst_random_delay", "0.5", 0);
    g_bot_attack_continuousfire_min_firetime    = gi.Cvar_Get("g_bot_attack_continuousfire_min_firetime", "0.5", 0);
    g_bot_attack_continuousfire_random_firetime = gi.Cvar_Get("g_bot_attack_continuousfire_random_firetime", "1.5", 0);
    g_bot_attack_react_min_delay               = gi.Cvar_Get("g_bot_attack_react_min_delay", "0.2", 0);
    g_bot_attack_react_random_delay            = gi.Cvar_Get("g_bot_attack_react_random_delay", "1.2", 0);
    g_bot_attack_spreadmult                    = gi.Cvar_Get("g_bot_attack_spreadmult", "1.0", 0);
    g_bot_turn_speed                           = gi.Cvar_Get("g_bot_turn_speed", "15", 0);
    g_bot_instamsg_chance                      = gi.Cvar_Get("g_bot_instamsg_chance", "5", 0);
    g_bot_instamsg_delay                       = gi.Cvar_Get("g_bot_instamsg_delay", "5.0", 0);

    g_rankedserver               = gi.Cvar_Get("g_rankedserver", "0", 0);
    g_spectatefollow_firstperson = gi.Cvar_Get("g_spectatefollow_firstperson", "0", 0);

    if (maxclients->integer + sv_maxbots->integer > MAX_CLIENTS) {
        unsigned int lowered;

        lowered = MAX_CLIENTS - maxclients->integer;

        gi.cvar_set("sv_maxbots", va("%d", lowered));
        gi.Printf("sv_maxbots reached max clients, lowering the value to %u\n", lowered);
    }

    g_instamsg_allowed  = gi.Cvar_Get("g_instamsg_allowed", "1", 0);
    g_instamsg_minDelay = gi.Cvar_Get("g_instamsg_minDelay", "1000", 0);
    g_textmsg_allowed   = gi.Cvar_Get("g_textmsg_allowed", "1", 0);
    g_textmsg_minDelay  = gi.Cvar_Get("g_textmsg_minDelay", "1000", 0);

    g_teambalance = gi.Cvar_Get("g_teambalance", "0", 0);

    g_navigation_legacy = gi.Cvar_Get("g_navigation_legacy", "0", CVAR_LATCH);

    cl_running = gi.Cvar_Get("cl_running", "", 0);
}
