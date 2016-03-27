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
cvar_t *sv_reborn;

cvar_t *deathmatch;
cvar_t *dmflags;
cvar_t *skill;
cvar_t *fraglimit;
cvar_t *timelimit;
cvar_t *roundlimit;
cvar_t *g_allowjointime;
cvar_t *g_teamswitchdelay;

cvar_t *password;
cvar_t *sv_privatePassword;
cvar_t *filterban;

cvar_t *flood_msgs;
cvar_t *flood_persecond;
cvar_t *flood_waitdelay;

cvar_t *maxclients;
cvar_t *maxbots;
cvar_t *maxentities;
cvar_t *nomonsters;
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
cvar_t *sv_walkspeed;
cvar_t *sv_dmspeedmult;
cvar_t *sv_crouchspeedmult;

cvar_t *g_showmem;
cvar_t *g_timeents;

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
cvar_t *g_rankedserver;
cvar_t *g_teamdamage;

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
cvar_t *g_lastsave;

cvar_t *g_forceteamspectate;
cvar_t *g_spectatefollow_forward;
cvar_t *g_spectatefollow_right;
cvar_t *g_spectatefollow_up;
cvar_t *g_spectatefollow_pitch;
cvar_t *g_spectatefollow_firstperson;
cvar_t *g_spectate_allow_full_chat;

cvar_t *g_voiceChat;

cvar_t *sv_scriptfiles;

void CVAR_Init( void )
{
	developer				= gi.Cvar_Get( "developer",					"0",				0 );
	sv_reborn				= gi.Cvar_Get( "sv_reborn",					"0",				CVAR_LATCH|CVAR_SERVERINFO );

	precache				= gi.Cvar_Get( "sv_precache",				"1",				0 );

	dedicated				= gi.Cvar_Get( "dedicated",					"1",				CVAR_LATCH );
	deathmatch				= gi.Cvar_Get( "deathmatch",				"1",				CVAR_USERINFO|CVAR_SERVERINFO|CVAR_LATCH );
	skill					= gi.Cvar_Get( "skill",						"1",				CVAR_USERINFO|CVAR_SERVERINFO|CVAR_LATCH );

	maxclients				= gi.Cvar_Get( "sv_maxclients",				"1",				0 );
	maxbots					= gi.Cvar_Get( "sv_maxbots",				"2",				0 );
	maxentities				= gi.Cvar_Get( "maxentities",				"1024",				CVAR_LATCH );

	password				= gi.Cvar_Get( "password",					"",					CVAR_USERINFO );
	sv_privatePassword		= gi.Cvar_Get( "sv_privatePassword",		"",					CVAR_TEMP );
	filterban				= gi.Cvar_Get( "filterban",					"1",				0 );

	dmflags					= gi.Cvar_Get( "dmflags",					"0",				CVAR_SERVERINFO );
	fraglimit				= gi.Cvar_Get( "fraglimit",					"0",				CVAR_SERVERINFO );
	timelimit				= gi.Cvar_Get( "timelimit",					"0",				CVAR_SERVERINFO );
	roundlimit				= gi.Cvar_Get( "roundlimit",				"0",				CVAR_SERVERINFO );
	g_allowjointime			= gi.Cvar_Get( "g_allowjointime",			"30",				CVAR_SERVERINFO );
	g_teamswitchdelay		= gi.Cvar_Get( "g_teamswitchdelay",			"15",				0 );

	nomonsters				= gi.Cvar_Get( "nomonsters",				"0",				CVAR_SERVERINFO );

	flood_msgs				= gi.Cvar_Get( "flood_msgs",				"4",				0 );
	flood_persecond			= gi.Cvar_Get( "flood_persecond",			"4",				0 );
	flood_waitdelay			= gi.Cvar_Get( "flood_waitdelay",			"10",				0 );

	detail					= gi.Cvar_Get( "detail",					"1",				CVAR_ARCHIVE );
	com_blood				= gi.Cvar_Get( "com_blood",					"1",				CVAR_ARCHIVE );
	whereami				= gi.Cvar_Get( "whereami",					"0",				0 );
	bosshealth				= gi.Cvar_Get( "bosshealth",				"0",				0 );

	sv_rollspeed			= gi.Cvar_Get( "sv_rollspeed",				"2200",				0 );
	sv_rollangle			= gi.Cvar_Get( "sv_rollangle",				"2",				0 );
	sv_maxvelocity			= gi.Cvar_Get( "sv_maxvelocity", 			"6000",				0 );
	sv_gravity				= gi.Cvar_Get( "sv_gravity", 				"512",				0 );

	sv_traceinfo			= gi.Cvar_Get( "sv_traceinfo", 				"0",				0 );
	sv_drawtrace			= gi.Cvar_Get( "sv_drawtrace", 				"0",				0 );
	sv_showbboxes			= gi.Cvar_Get( "sv_showbboxes", 			"0",				0 );

	sv_testloc_num			= gi.Cvar_Get( "sv_testloc_num", 			"0",				0 );
	sv_testloc_secondary	= gi.Cvar_Get( "sv_testloc_secondary", 		"0",				0 );
	sv_testloc_radius		= gi.Cvar_Get( "sv_testloc_radius", 		"1",				0 );
	sv_testloc_offset_x		= gi.Cvar_Get( "sv_testloc_offset_x", 		"0",				0 );
	sv_testloc_offset_y		= gi.Cvar_Get( "sv_testloc_offset_y", 		"0",				0 );
	sv_testloc_offset_z		= gi.Cvar_Get( "sv_testloc_offset_z", 		"0",				0 );
	sv_testloc_radius2		= gi.Cvar_Get( "sv_testloc_radius2", 		"1",				0 );
	sv_testloc_offset2_x	= gi.Cvar_Get( "sv_testloc_offset2_x", 		"0",				0 );
	sv_testloc_offset2_y	= gi.Cvar_Get( "sv_testloc_offset2_y", 		"0",				0 );
	sv_testloc_offset2_z	= gi.Cvar_Get( "sv_testloc_offset2_z", 		"0",				0 );

	sv_showcameras			= gi.Cvar_Get( "sv_showcameras", 			"0",				0 );
	sv_showentnums			= gi.Cvar_Get( "sv_showentnums", 			"0",				0 );

	sv_friction				= gi.Cvar_Get( "sv_friction", 				"4",				CVAR_SERVERINFO );
	sv_stopspeed			= gi.Cvar_Get( "sv_stopspeed", 				"100",				CVAR_SERVERINFO );
	sv_waterfriction		= gi.Cvar_Get( "sv_waterfriction", 			"1",				CVAR_SERVERINFO );
	sv_waterspeed			= gi.Cvar_Get( "sv_waterspeed", 			"400",				CVAR_SERVERINFO );

	sv_cheats				= gi.Cvar_Get( "cheats", 					"0",				CVAR_USERINFO|CVAR_SERVERINFO|CVAR_LATCH );
	sv_fps					= gi.Cvar_Get( "sv_fps", 					"20",				CVAR_SERVERINFO|CVAR_SAVEGAME );
	sv_cinematic			= gi.Cvar_Get( "sv_cinematic", 				"0",				CVAR_SYSTEMINFO|CVAR_ROM );

	sv_maplist				= gi.Cvar_Get( "sv_maplist", 				"",					CVAR_ARCHIVE|CVAR_SERVERINFO );
	sv_nextmap				= gi.Cvar_Get( "nextmap", 					"",					0 );

	sv_runspeed				= gi.Cvar_Get( "sv_runspeed", 				"250",				CVAR_SERVERINFO );
	sv_walkspeed			= gi.Cvar_Get( "sv_walkspeed", 				"150",				CVAR_SERVERINFO );
	sv_dmspeedmult			= gi.Cvar_Get( "sv_dmspeedmult", 			"1.1",				CVAR_SERVERINFO );
	sv_crouchspeedmult		= gi.Cvar_Get( "sv_crouchspeedmult", 		"0.6",				CVAR_SERVERINFO );

	g_showmem				= gi.Cvar_Get( "g_showmem", 				"0",				0 );
	g_timeents				= gi.Cvar_Get( "g_timeents", 				"0",				0 );
	g_showaxis				= gi.Cvar_Get( "g_showaxis", 				"0",				0 );
	g_showplayerstate		= gi.Cvar_Get( "g_showplayerstate", 		"0",				0 );
	g_showplayeranim		= gi.Cvar_Get( "g_showplayeranim", 			"0",				0 );
	g_showbullettrace		= gi.Cvar_Get( "g_showbullettrace", 		"0",				0 );
	g_showawareness			= gi.Cvar_Get( "g_showawareness", 			"0",				CVAR_CHEAT );
	g_entinfo				= gi.Cvar_Get( "g_entinfo", 				"0",				CVAR_CHEAT );
	g_showlookat			= gi.Cvar_Get( "g_showlookat", 				"0",				CVAR_CHEAT );

	g_numdebuglines			= gi.Cvar_Get( "g_numdebuglines", 			"4096",				CVAR_LATCH );
	g_numdebuglinedelays	= gi.Cvar_Get( "g_numdebuglinedelays", 		"0",				0 );
	g_numdebugstrings		= gi.Cvar_Get( "g_numdebugstrings", 		"256",				CVAR_LATCH );

	g_spiffyplayer			= gi.Cvar_Get( "g_spiffyplayer", 			"0",				CVAR_LATCH );
	g_spiffyvelocity_x		= gi.Cvar_Get( "g_spiffyvelocity_x", 		"0",				0 );
	g_spiffyvelocity_y		= gi.Cvar_Get( "g_spiffyvelocity_y", 		"0",				0 );
	g_spiffyvelocity_z		= gi.Cvar_Get( "g_spiffyvelocity_z", 		"0",				0 );

	g_playermodel			= gi.Cvar_Get( "g_playermodel", 			"american_army",	CVAR_SAVEGAME );
	g_statefile				= gi.Cvar_Get( "g_statefile", 				"global/mike",		0 );

	g_showautoaim			= gi.Cvar_Get( "g_showautoaim", 			"0",				0 );
	g_debugtargets			= gi.Cvar_Get( "g_debugtargets", 			"0",				0 );
	g_debugdamage			= gi.Cvar_Get( "g_debugdamage", 			"0",				0 );
	g_logstats				= gi.Cvar_Get( "g_logstats", 				"0",				0 );
	g_showtokens			= gi.Cvar_Get( "g_showtokens", 				"0",				0 );
	g_showopcodes			= gi.Cvar_Get( "g_showopcodes", 			"0",				0 );
	g_scriptcheck			= gi.Cvar_Get( "g_scriptcheck", 			"0",				0 );
	g_nodecheck				= gi.Cvar_Get( "g_nodecheck", 				"0",				0 );
	g_scriptdebug			= gi.Cvar_Get( "g_scriptdebug", 			"0",				0 );
	g_scripttrace			= gi.Cvar_Get( "g_scripttrace", 			"0",				0 );

	g_ai					= gi.Cvar_Get( "g_ai", 						"1",				0 );
	g_vehicle				= gi.Cvar_Get( "g_vehicle", 				"1",				0 );

	g_monitor				= gi.Cvar_Get( "g_monitor", 				"",					0 );
	g_monitorNum			= gi.Cvar_Get( "g_monitorNum", 				"-1",				0 );

	g_spawnentities			= gi.Cvar_Get( "g_spawnentities", 			"1",				0 );
	g_spawnai				= gi.Cvar_Get( "g_spawnai", 				"1",				0 );

	g_patherror				= gi.Cvar_Get( "g_patherror", 				"1",				0 );

	g_droppeditemlife		= gi.Cvar_Get( "g_droppeditemlife", 		"30",				0 );
	g_dropclips				= gi.Cvar_Get( "g_dropclips", 				"2",				0 );

	g_animdump				= gi.Cvar_Get( "g_animdump", 				"0",				0 );

	g_showdamage			= gi.Cvar_Get( "g_showdamage", 				"0",				0 );

	g_ai_notifyradius		= gi.Cvar_Get( "g_ai_notifyradius", 		"1024",				0 );
	g_ai_noticescale		= gi.Cvar_Get( "g_ai_noticescale", 			"1",				0 );
	g_ai_soundscale			= gi.Cvar_Get( "g_ai_soundscale", 			"1",				0 );
	ai_debug_grenades		= gi.Cvar_Get( "ai_debug_grenades", 		"0",				CVAR_CHEAT );

	g_gametype				= gi.Cvar_Get( "g_gametype", 				"0",				CVAR_USERINFO|CVAR_SERVERINFO|CVAR_LATCH );
	g_gametypestring		= gi.Cvar_Get( "g_gametypestring", 			"Free-For-All",		CVAR_SERVERINFO );
	g_rankedserver			= gi.Cvar_Get( "g_rankedserver", 			"0",				0 );
	g_teamdamage			= gi.Cvar_Get( "g_teamdamage", 				"0",				0 );
	g_allowvote				= gi.Cvar_Get( "g_allowvote", 				"1",				0 );
	g_warmup				= gi.Cvar_Get( "g_warmup", 					"20",				CVAR_ARCHIVE );
	g_doWarmup				= gi.Cvar_Get( "g_doWarmup", 				"1",				0 );

	g_forceready			= gi.Cvar_Get( "g_forceready", 				"1",				CVAR_ARCHIVE|CVAR_SERVERINFO );
	g_forcerespawn			= gi.Cvar_Get( "g_forcerespawn", 			"0",				CVAR_ARCHIVE|CVAR_SERVERINFO );
	g_maxintermission		= gi.Cvar_Get( "g_maxintermission", 		"15",				CVAR_ARCHIVE|CVAR_SERVERINFO );

	g_smoothClients			= gi.Cvar_Get( "g_smoothClients", 			"1",				0 );
	pmove_fixed				= gi.Cvar_Get( "pmove_fixed", 				"0",				CVAR_SYSTEMINFO );
	pmove_msec				= gi.Cvar_Get( "pmove_msec", 				"0",				CVAR_SYSTEMINFO );

	g_inactivespectate		= gi.Cvar_Get( "g_inactiveSpectate", 		"60",				CVAR_ARCHIVE );
	g_inactivekick			= gi.Cvar_Get( "g_inactivekick", 			"900",				CVAR_ARCHIVE );

	s_debugmusic			= gi.Cvar_Get( "s_debugmusic", 				"0",				0 );

	sv_scriptfiles			= gi.Cvar_Get( "sv_scriptfiles", 			"0",				0 );

	switch( g_gametype->integer )
	{
	default:
	case GT_FFA:
		if( g_gametype->integer >= GT_MAX_GAME_TYPE )
		{
			gi.Cvar_Set( "g_gametypestring", "Multiplayer" );
		}
		else
		{
			gi.Cvar_Set( "g_gametypestring", "Free-For-All" );
		}
		break;

	case GT_TEAM:
		gi.Cvar_Set( "g_gametypestring", "Team-Match" );
		break;

	case GT_TEAM_ROUNDS:
		gi.Cvar_Set( "g_gametypestring", "Round-Based-Match" );
		break;

	case GT_OBJECTIVE:
		gi.Cvar_Set( "g_gametypestring", "Objective-Match" );
		break;

	case GT_SINGLE_PLAYER:
		gi.Cvar_Set( "g_gametypestring", "0" );
		break;
	}

	g_viewkick_pitch				= gi.Cvar_Get( "g_viewkick_pitch",				"0.3",			CVAR_CHEAT );
	g_viewkick_yaw					= gi.Cvar_Get( "g_viewkick_yaw",				"0.3",			CVAR_CHEAT );
	g_viewkick_roll					= gi.Cvar_Get( "g_viewkick_roll",				"0.15",			CVAR_CHEAT );
	g_viewkick_dmmult				= gi.Cvar_Get( "g_viewkick_dmmult",				"1.0",			CVAR_CHEAT );

	g_drawattackertime				= gi.Cvar_Get( "g_drawattackertime",			"5",			0 );
	g_playerdeltamethod				= gi.Cvar_Get( "g_playerdeltamethod",			"0",			0 );

	g_success						= gi.Cvar_Get( "g_success",						"0",			0 );
	g_failed						= gi.Cvar_Get( "g_failed",						"0",			0 );
	g_gotmedal						= gi.Cvar_Get( "g_gotmedal",					"0",			0 );

	g_medal0						= gi.Cvar_Get( "g_medal0",						"0",			CVAR_ARCHIVE );
	g_medal1						= gi.Cvar_Get( "g_medal1",						"0",			CVAR_ARCHIVE );
	g_medal2						= gi.Cvar_Get( "g_medal2",						"0",			CVAR_ARCHIVE );
	g_medal3						= gi.Cvar_Get( "g_medal3",						"0",			CVAR_ARCHIVE );
	g_medal4						= gi.Cvar_Get( "g_medal4",						"0",			CVAR_ARCHIVE );
	g_medal5						= gi.Cvar_Get( "g_medal5",						"0",			CVAR_ARCHIVE );

	g_eogmedal0						= gi.Cvar_Get( "g_eogmedal0",					"0",			CVAR_ARCHIVE );
	g_eogmedal1						= gi.Cvar_Get( "g_eogmedal1",					"0",			CVAR_ARCHIVE );
	g_eogmedal2						= gi.Cvar_Get( "g_eogmedal2",					"0",			CVAR_ARCHIVE );

	g_m1l1							= gi.Cvar_Get( "g_m1l1",						"1",			CVAR_ARCHIVE );
	g_m1l2							= gi.Cvar_Get( "g_m1l2",						"0",			CVAR_ARCHIVE );
	g_m1l3							= gi.Cvar_Get( "g_m1l3",						"0",			CVAR_ARCHIVE );
	g_m2l1							= gi.Cvar_Get( "g_m2l1",						"0",			CVAR_ARCHIVE );
	g_m2l2							= gi.Cvar_Get( "g_m2l2",						"0",			CVAR_ARCHIVE );
	g_m2l3							= gi.Cvar_Get( "g_m2l3",						"0",			CVAR_ARCHIVE );
	g_m3l1							= gi.Cvar_Get( "g_m3l1",						"0",			CVAR_ARCHIVE );
	g_m3l2							= gi.Cvar_Get( "g_m3l2",						"0",			CVAR_ARCHIVE );
	g_m3l3							= gi.Cvar_Get( "g_m3l3",						"0",			CVAR_ARCHIVE );
	g_m4l1							= gi.Cvar_Get( "g_m4l1",						"0",			CVAR_ARCHIVE );
	g_m4l2							= gi.Cvar_Get( "g_m4l2",						"0",			CVAR_ARCHIVE );
	g_m4l3							= gi.Cvar_Get( "g_m4l3",						"0",			CVAR_ARCHIVE );
	g_m5l1							= gi.Cvar_Get( "g_m5l1",						"0",			CVAR_ARCHIVE );
	g_m5l2							= gi.Cvar_Get( "g_m5l2",						"0",			CVAR_ARCHIVE );
	g_m5l3							= gi.Cvar_Get( "g_m5l3",						"0",			CVAR_ARCHIVE );
	g_m6l1							= gi.Cvar_Get( "g_m6l1",						"0",			CVAR_ARCHIVE );
	g_m6l2							= gi.Cvar_Get( "g_m6l2",						"0",			CVAR_ARCHIVE );
	g_m6l3							= gi.Cvar_Get( "g_m6l3",						"0",			CVAR_ARCHIVE );

	g_lastsave						= gi.Cvar_Get( "g_lastsave",					"",				CVAR_ARCHIVE );
	g_forceteamspectate				= gi.Cvar_Get( "g_forceteamspectate",			"1",			0 );
	g_spectatefollow_forward		= gi.Cvar_Get( "g_spectatefollow_forward",		"-56",			0 );
	g_spectatefollow_right			= gi.Cvar_Get( "g_spectatefollow_right",		"0",			0 );
	g_spectatefollow_up				= gi.Cvar_Get( "g_spectatefollow_up",			"24",			0 );
	g_spectatefollow_pitch			= gi.Cvar_Get( "g_spectatefollow_pitch",		"2",			0 );
	g_spectatefollow_firstperson	= gi.Cvar_Get( "g_spectatefollow_firstperson",	"0",			0 );
	g_spectate_allow_full_chat		= gi.Cvar_Get( "g_spectate_allow_full_chat",	"0",			0 );

	g_voiceChat						= gi.Cvar_Get( "g_voiceChat",					"1",			0 );

	gi.Cvar_Get( "g_obj_alliedtext1",			"",			CVAR_SERVERINFO );
	gi.Cvar_Get( "g_obj_alliedtext2",			"",			CVAR_SERVERINFO );
	gi.Cvar_Get( "g_obj_alliedtext3",			"",			CVAR_SERVERINFO );
	gi.Cvar_Get( "g_obj_axistext1",				"",			CVAR_SERVERINFO );
	gi.Cvar_Get( "g_obj_axistext2",				"",			CVAR_SERVERINFO );
	gi.Cvar_Get( "g_obj_axistext3",				"",			CVAR_SERVERINFO );
	gi.Cvar_Get( "g_scoreboardpic",				"",			CVAR_SERVERINFO );
	gi.Cvar_Get( "g_scoreboardpicover",			"",			CVAR_SERVERINFO );

	if( g_gametype->integer > GT_TEAM )
	{
		gi.Cvar_Set( "g_tempaxisscore",				"0" );
		gi.Cvar_Set( "g_tempaxiswinsinrow",			"0" );
		gi.Cvar_Set( "g_tempalliesscore",			"0" );
		gi.Cvar_Set( "g_tempallieswinsinrow",		"0" );
		gi.Cvar_Set( "g_tempmaptime",				"0" );
		gi.Cvar_Set( "g_scoreboardpicover",			"" );
	}
}
