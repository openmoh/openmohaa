/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
// cg_main.c -- initialization and primary entry point for cgame
#include "cg_local.h"

clientGameImport_t		cgi;
clientGameExport_t		cge;

int forceModelModificationCount = -1;

void CG_Init( int serverMessageNum, int serverCommandSequence, int clientNum, int randomSeed );
void CG_Shutdown( void );

#if 0
/*
================
vmMain

This is the only way control passes into the module.
This must be the very first function compiled into the .q3vm file
================
*/
intptr_t vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11  ) {

	switch ( command ) {
	case CG_INIT:
		CG_Init( arg0, arg1, arg2, arg3 );
		return 0;
	case CG_SHUTDOWN:
		CG_Shutdown();
		return 0;
	case CG_CONSOLE_COMMAND:
		return CG_ConsoleCommand();
	case CG_DRAW_ACTIVE_FRAME:
		CG_DrawActiveFrame( arg0, arg1, arg2 );
		return 0;
	case CG_DRAW_2D:
		CG_Draw2D( arg0, arg1, arg2 );
		return 0;
	case CG_CROSSHAIR_PLAYER:
		return CG_CrosshairPlayer();
	case CG_LAST_ATTACKER:
		return CG_LastAttacker();
	case CG_KEY_EVENT:
		CG_KeyEvent(arg0, arg1);
		return 0;
	case CG_MOUSE_EVENT:
		CG_MouseEvent(arg0, arg1);
		return 0;
	case CG_EVENT_HANDLING:
		CG_EventHandling(arg0);
		return 0;
	case CG_CENTERPRINT:
		CG_CenterPrint( (const char*)arg0, 200, 10 );
		return 0;
	case CG_LOCATIONPRINT:
		CG_LocationPrint( (const char*)arg0, arg1, arg2, 10 );
		return 0;
	case CG_PARSEMSG:
		CG_ParseCGMessage();
		return 0;
	default:
		CG_Error( "vmMain: unknown command %i", command );
		break;
	}
	return -1;
}
#endif

cg_t				cg;
cgs_t				cgs;
centity_t			cg_entities[ MAX_GENTITIES ];

cvar_t	*cg_railTrailTime;
cvar_t	*cg_centertime;
cvar_t	*cg_locationtime;
cvar_t	*cg_runpitch;
cvar_t	*cg_runroll;
cvar_t	*cg_bobup;
cvar_t	*cg_bobpitch;
cvar_t	*cg_bobroll;
cvar_t	*cg_swingSpeed;
cvar_t	*cg_shadows;
cvar_t	*cg_gibs;
cvar_t	*cg_drawTimer;
cvar_t	*cg_drawFPS;
cvar_t	*cg_drawSnapshot;
cvar_t	*cg_draw3dIcons;
cvar_t	*cg_drawIcons;
cvar_t	*cg_drawAmmoWarning;
cvar_t	*cg_drawCrosshair;
cvar_t	*cg_drawCrosshairNames;
cvar_t	*cg_drawRewards;
cvar_t	*cg_crosshairSize;
cvar_t	*cg_crosshairX;
cvar_t	*cg_crosshairY;
cvar_t	*cg_crosshairHealth;
cvar_t	*cg_draw2D;
cvar_t	*cg_drawStatus;
cvar_t	*cg_animSpeed;
cvar_t	*cg_debugAnim;
cvar_t	*cg_debugPosition;
cvar_t	*cg_debugEvents;
cvar_t	*cg_errorDecay;
cvar_t	*cg_nopredict;
cvar_t	*cg_noPlayerAnims;
cvar_t	*cg_showmiss;
cvar_t	*cg_footsteps;
cvar_t	*cg_addMarks;
cvar_t	*cg_brassTime;
cvar_t	*cg_viewsize;
cvar_t	*cg_drawGun;
cvar_t	*cg_gun_frame;
cvar_t	*cg_gun_x;
cvar_t	*cg_gun_y;
cvar_t	*cg_gun_z;
cvar_t	*cg_tracerChance;
cvar_t	*cg_tracerWidth;
cvar_t	*cg_tracerLength;
cvar_t	*cg_autoswitch;
cvar_t	*cg_ignore;
cvar_t	*cg_simpleItems;
cvar_t	*cg_fov;
cvar_t	*cg_zoomFov;
cvar_t	*cg_thirdPerson;
cvar_t	*cg_thirdPersonRange;
cvar_t	*cg_thirdPersonAngle;
cvar_t	*cg_stereoSeparation;
cvar_t	*cg_lagometer;
cvar_t	*cg_drawAttacker;
cvar_t	*cg_synchronousClients;
cvar_t	*cg_teamChatTime;
cvar_t	*cg_teamChatHeight;
cvar_t	*cg_stats;
cvar_t	*cg_buildScript;
cvar_t	*cg_forceModel;
cvar_t	*cg_paused;
cvar_t	*cg_blood;
cvar_t	*cg_predictItems;
cvar_t	*cg_deferPlayers;
cvar_t	*cg_drawTeamOverlay;
cvar_t	*cg_teamOverlayUserinfo;
cvar_t	*cg_drawFriend;
cvar_t	*cg_teamChatsOnly;
cvar_t	*cg_noVoiceChats;
cvar_t	*cg_noVoiceText;
cvar_t	*cg_hudFiles;
cvar_t	*cg_scorePlum;
cvar_t	*cg_smoothClients;
cvar_t	*pmove_fixed;
cvar_t	*cg_pmove_fixed;
cvar_t	*pmove_msec;
cvar_t	*cg_pmove_msec;
cvar_t	*cg_cameraMode;
cvar_t	*cg_cameraOrbit;
cvar_t	*cg_cameraOrbitDelay;
cvar_t	*cg_timescaleFadeEnd;
cvar_t	*cg_timescaleFadeSpeed;
cvar_t	*cg_timescale;
cvar_t	*cg_smallFont;
cvar_t	*cg_bigFont;
cvar_t	*cg_noTaunt;
cvar_t	*cg_noProjectileTrail;
cvar_t	*cg_trueLightning;

cvar_t	*vm_offset_max;
cvar_t	*vm_offset_speed;
cvar_t	*vm_sway_front;
cvar_t	*vm_sway_side;
cvar_t	*vm_sway_up;
cvar_t	*vm_offset_air_front;
cvar_t	*vm_offset_air_side;
cvar_t	*vm_offset_air_up;
cvar_t	*vm_offset_crouch_front;
cvar_t	*vm_offset_crouch_side;
cvar_t	*vm_offset_crouch_up;
cvar_t	*vm_offset_rocketcrouch_front;
cvar_t	*vm_offset_rocketcrouch_side;
cvar_t	*vm_offset_rocketcrouch_up;
cvar_t	*vm_offset_shotguncrouch_front;
cvar_t	*vm_offset_shotguncrouch_side;
cvar_t	*vm_offset_shotguncrouch_up;
cvar_t	*vm_offset_vel_base;
cvar_t	*vm_offset_vel_front;
cvar_t	*vm_offset_vel_side;
cvar_t	*vm_offset_vel_up;
cvar_t	*vm_offset_upvel;
cvar_t	*vm_lean_lower;
cvar_t	*cg_debugCGMessages;

typedef struct {
	cvar_t		**cvar;
	char		*cvarName;
	char		*defaultString;
	int			cvarFlags;
} cvarTable_t;

static cvarTable_t cvarTable[] = {
	{ &cg_ignore, "cg_ignore", "0", 0 },	// used for debugging
	{ &cg_autoswitch, "cg_autoswitch", "1", CVAR_ARCHIVE },
	{ &cg_drawGun, "cg_drawGun", "1", CVAR_ARCHIVE },
	{ &cg_zoomFov, "cg_zoomfov", "22.5", CVAR_ARCHIVE },
	{ &cg_fov, "cg_fov", "90", CVAR_ARCHIVE },
	{ &cg_viewsize, "cg_viewsize", "100", CVAR_ARCHIVE },
	{ &cg_stereoSeparation, "cg_stereoSeparation", "0.4", CVAR_ARCHIVE  },
	{ &cg_shadows, "cg_shadows", "1", CVAR_ARCHIVE  },
	{ &cg_gibs, "cg_gibs", "1", CVAR_ARCHIVE  },
	{ &cg_draw2D, "cg_draw2D", "1", CVAR_ARCHIVE  },
	{ &cg_drawStatus, "cg_drawStatus", "1", CVAR_ARCHIVE  },
	{ &cg_drawTimer, "cg_drawTimer", "0", CVAR_ARCHIVE  },
	{ &cg_drawFPS, "cg_drawFPS", "0", CVAR_ARCHIVE  },
	{ &cg_drawSnapshot, "cg_drawSnapshot", "0", CVAR_ARCHIVE  },
	{ &cg_draw3dIcons, "cg_draw3dIcons", "1", CVAR_ARCHIVE  },
	{ &cg_drawIcons, "cg_drawIcons", "1", CVAR_ARCHIVE  },
	{ &cg_drawAmmoWarning, "cg_drawAmmoWarning", "1", CVAR_ARCHIVE  },
	{ &cg_drawAttacker, "cg_drawAttacker", "1", CVAR_ARCHIVE  },
	{ &cg_drawCrosshair, "cg_drawCrosshair", "1", CVAR_ARCHIVE },
	{ &cg_drawCrosshairNames, "cg_drawCrosshairNames", "1", CVAR_ARCHIVE },
	{ &cg_drawRewards, "cg_drawRewards", "1", CVAR_ARCHIVE },
	{ &cg_crosshairSize, "cg_crosshairSize", "24", CVAR_ARCHIVE },
	{ &cg_crosshairHealth, "cg_crosshairHealth", "1", CVAR_ARCHIVE },
	{ &cg_crosshairX, "cg_crosshairX", "0", CVAR_ARCHIVE },
	{ &cg_crosshairY, "cg_crosshairY", "0", CVAR_ARCHIVE },
	{ &cg_brassTime, "cg_brassTime", "2500", CVAR_ARCHIVE },
	{ &cg_simpleItems, "cg_simpleItems", "0", CVAR_ARCHIVE },
	{ &cg_addMarks, "cg_marks", "1", CVAR_ARCHIVE },
	{ &cg_lagometer, "cg_lagometer", "1", CVAR_ARCHIVE },
	{ &cg_railTrailTime, "cg_railTrailTime", "400", CVAR_ARCHIVE  },
	{ &cg_gun_x, "cg_gunX", "0", CVAR_CHEAT | CVAR_ARCHIVE  },
	{ &cg_gun_y, "cg_gunY", "0", CVAR_CHEAT | CVAR_ARCHIVE  },
	{ &cg_gun_z, "cg_gunZ", "0", CVAR_CHEAT | CVAR_ARCHIVE },
	{ &cg_centertime, "cg_centertime", "3", CVAR_CHEAT },
	{ &cg_locationtime, "cg_locationtime", "3", CVAR_CHEAT },
	{ &cg_runpitch, "cg_runpitch", "0.002", CVAR_ARCHIVE},
	{ &cg_runroll, "cg_runroll", "0.005", CVAR_ARCHIVE },
	{ &cg_bobup , "cg_bobup", "0.005", CVAR_CHEAT },
	{ &cg_bobpitch, "cg_bobpitch", "0.002", CVAR_ARCHIVE },
	{ &cg_bobroll, "cg_bobroll", "0.002", CVAR_ARCHIVE },
	{ &cg_swingSpeed, "cg_swingSpeed", "0.3", CVAR_CHEAT },
	{ &cg_animSpeed, "cg_animspeed", "1", CVAR_CHEAT },
	{ &cg_debugAnim, "cg_debuganim", "0", CVAR_CHEAT },
	{ &cg_debugPosition, "cg_debugposition", "0", CVAR_CHEAT },
	{ &cg_debugEvents, "cg_debugevents", "0", CVAR_CHEAT },
	{ &cg_errorDecay, "cg_errordecay", "100", 0 },
	{ &cg_nopredict, "cg_nopredict", "0", 0 },
	{ &cg_noPlayerAnims, "cg_noplayeranims", "0", CVAR_CHEAT },
	{ &cg_showmiss, "cg_showmiss", "0", 0 },
	{ &cg_footsteps, "cg_footsteps", "1", CVAR_CHEAT },
	{ &cg_tracerChance, "cg_tracerchance", "0.4", CVAR_CHEAT },
	{ &cg_tracerWidth, "cg_tracerwidth", "1", CVAR_CHEAT },
	{ &cg_tracerLength, "cg_tracerlength", "100", CVAR_CHEAT },
	{ &cg_thirdPersonRange, "cg_thirdPersonRange", "40", CVAR_CHEAT },
	{ &cg_thirdPersonAngle, "cg_thirdPersonAngle", "0", CVAR_CHEAT },
	{ &cg_thirdPerson, "cg_thirdPerson", "0", 0 },
	{ &cg_teamChatTime, "cg_teamChatTime", "3000", CVAR_ARCHIVE  },
	{ &cg_teamChatHeight, "cg_teamChatHeight", "0", CVAR_ARCHIVE  },
	{ &cg_forceModel, "cg_forceModel", "0", CVAR_ARCHIVE  },
	{ &cg_predictItems, "cg_predictItems", "1", CVAR_ARCHIVE },
	{ &cg_deferPlayers, "cg_deferPlayers", "1", CVAR_ARCHIVE },
	{ &cg_drawTeamOverlay, "cg_drawTeamOverlay", "0", CVAR_ARCHIVE },
	{ &cg_teamOverlayUserinfo, "teamoverlay", "0", CVAR_ROM | CVAR_USERINFO },
	{ &cg_stats, "cg_stats", "0", 0 },
	{ &cg_drawFriend, "cg_drawFriend", "1", CVAR_ARCHIVE },
	{ &cg_teamChatsOnly, "cg_teamChatsOnly", "0", CVAR_ARCHIVE },
	{ &cg_noVoiceChats, "cg_noVoiceChats", "0", CVAR_ARCHIVE },
	{ &cg_noVoiceText, "cg_noVoiceText", "0", CVAR_ARCHIVE },
	// the following variables are created in other parts of the system,
	// but we also reference them here
	{ &cg_buildScript, "com_buildScript", "0", 0 },	// force loading of all possible data amd error on failures
	{ &cg_paused, "cl_paused", "0", CVAR_ROM },
	{ &cg_blood, "com_blood", "1", CVAR_ARCHIVE },
	{ &cg_synchronousClients, "g_synchronousClients", "0", 0 },	// communicated by systeminfo

	{ &cg_cameraOrbit, "cg_cameraOrbit", "0", CVAR_CHEAT},
	{ &cg_cameraOrbitDelay, "cg_cameraOrbitDelay", "50", CVAR_ARCHIVE},
	{ &cg_timescaleFadeEnd, "cg_timescaleFadeEnd", "1", 0},
	{ &cg_timescaleFadeSpeed, "cg_timescaleFadeSpeed", "0", 0},
	{ &cg_timescale, "timescale", "1", 0},
	{ &cg_scorePlum, "cg_scorePlums", "1", CVAR_USERINFO | CVAR_ARCHIVE},
	{ &cg_smoothClients, "cg_smoothClients", "0", CVAR_USERINFO | CVAR_ARCHIVE},
	{ &cg_cameraMode, "com_cameraMode", "0", CVAR_CHEAT},

	{ &pmove_fixed, "pmove_fixed", "0", 0},
	{ &pmove_msec, "pmove_msec", "8", 0},
	{ &cg_noTaunt, "cg_noTaunt", "0", CVAR_ARCHIVE},
	{ &cg_noProjectileTrail, "cg_noProjectileTrail", "0", CVAR_ARCHIVE},
	{ &cg_smallFont, "ui_smallFont", "0.25", CVAR_ARCHIVE},
	{ &cg_bigFont, "ui_bigFont", "0.4", CVAR_ARCHIVE},
	{ &cg_trueLightning, "cg_trueLightning", "0.0", CVAR_ARCHIVE},
//	{ &cg_pmove_fixed, "cg_pmove_fixed", "0", CVAR_USERINFO | CVAR_ARCHIVE },

	// su44: MoHAA viewmodel-related cvars
	{ &vm_offset_max, "vm_offset_max", "8.0", 0},
	{ &vm_offset_speed, "vm_offset_speed", "8.0", 0},
	{ &vm_sway_front, "vm_sway_front", "0.1", 0},
	{ &vm_sway_side, "vm_sway_side", "0.005", 0},
	{ &vm_sway_up, "vm_sway_up", "0.003", 0},
	{ &vm_offset_air_front, "vm_offset_air_front", "-3.0", 0},
	{ &vm_offset_air_side, "vm_offset_air_side", "1.5", 0},
	{ &vm_offset_air_up, "vm_offset_air_up", "-6.0", 0},
	{ &vm_offset_crouch_front, "vm_offset_crouch_front", "-0.5", 0},
	{ &vm_offset_crouch_side, "vm_offset_crouch_side", "2.25", 0},
	{ &vm_offset_crouch_up, "vm_offset_crouch_up", "0.2", 0},
	{ &vm_offset_rocketcrouch_front, "vm_offset_rocketcrouch_front", "0", 0},
	{ &vm_offset_rocketcrouch_side, "vm_offset_rocketcrouch_side", "0", 0},
	{ &vm_offset_rocketcrouch_up, "vm_offset_rocketcrouch_up", "0", 0},
	{ &vm_offset_shotguncrouch_front, "vm_offset_shotguncrouch_front", "-1", 0},
	{ &vm_offset_shotguncrouch_side, "vm_offset_shotguncrouch_side", "-1.1", 0},
	{ &vm_offset_shotguncrouch_up, "vm_offset_shotguncrouch_up", "-1.1", 0},
	{ &vm_offset_vel_base, "vm_offset_vel_base", "100", 0},
	{ &vm_offset_vel_front, "vm_offset_vel_front", "-2.0", 0},
	{ &vm_offset_vel_side, "vm_offset_vel_side", "1.5", 0},
	{ &vm_offset_vel_up, "vm_offset_vel_up", "-4.0", 0},
	{ &vm_offset_upvel, "vm_offset_upvel", "0.0025", 0},
	{ &vm_lean_lower, "vm_lean_lower", "0.1", 0},
	{ &cg_debugCGMessages, "cg_debugCGMessages", "0", 0},
};

static int  cvarTableSize = sizeof( cvarTable ) / sizeof( cvarTable[0] );

/*
=================
CG_RegisterCvars
=================
*/
void CG_RegisterCvars( void ) {
	int			i;
	cvarTable_t	*cv;
	cvar_t		*sv_running;

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
		*cv->cvar = cgi.Cvar_Get( cv->cvarName, cv->defaultString, cv->cvarFlags );
	}

	sv_running = cgi.Cvar_Get( "sv_running", "", 0 );

	// see if we are also running the server on this machine
	cgs.localServer = sv_running->integer;

	forceModelModificationCount = cg_forceModel->modificationCount;

	cgi.Cvar_Get( "model", DEFAULT_MODEL, CVAR_USERINFO | CVAR_ARCHIVE );
}

/*
===================
CG_ForceModelChange
===================
*/
static void CG_ForceModelChange( void ) {
	int		i;

	for (i=0 ; i<MAX_CLIENTS ; i++) {
		const char		*clientInfo;

		clientInfo = CG_ConfigString( CS_PLAYERS+i );
		if ( !clientInfo[0] ) {
			continue;
		}
		CG_NewClientInfo( i );
	}
}

/*
=================
CG_UpdateCvars
=================
*/
void CG_UpdateCvars( void ) {
	// check for modications here

	// If team overlay is on, ask for updates from the server.  If its off,
	// let the server know so we don't receive it
	if ( drawTeamOverlayModificationCount != cg_drawTeamOverlay->modificationCount ) {
		drawTeamOverlayModificationCount = cg_drawTeamOverlay->modificationCount;

		if ( cg_drawTeamOverlay->integer > 0 ) {
			cgi.Cvar_Set( "teamoverlay", "1" );
		} else {
			cgi.Cvar_Set( "teamoverlay", "0" );
		}
	}

	// if force model changed
	if ( forceModelModificationCount != cg_forceModel->modificationCount ) {
		forceModelModificationCount = cg_forceModel->modificationCount;
		CG_ForceModelChange();
	}
}

int CG_CrosshairPlayer( void ) {
	if(cg.snap == 0)
		return -1;
	return cg.snap->ps.stats[STAT_INFOCLIENT];
}

int CG_LastAttacker( void ) {
	if ( !cg.attackerTime ) {
		return -1;
	}
	return cg.snap->ps.stats[STAT_ATTACKERCLIENT];
}

void QDECL CG_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	cgi.Printf( text );
}

void QDECL CG_Error( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	cgi.Error( ERR_DROP, text );
}

void QDECL Com_Error( int level, const char *error, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	vsprintf (text, error, argptr);
	va_end (argptr);

	CG_Error( "%s", text);
}

void QDECL Com_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	CG_Printf ("%s", text);
}

/*
================
CG_Argv
================
*/
const char *CG_Argv( int arg ) {
	return cgi.Argv( arg );
}


//========================================================================

/*
=================
CG_RegisterSounds

called during a precache command
=================
*/
static void CG_RegisterSounds( void ) {
	int		i;
	const char	*soundName;
	char	buffer[MAX_STRING_CHARS];

	for ( i = 1 ; i < MAX_SOUNDS ; i++ ) {
		soundName = CG_ConfigString( CS_SOUNDS+i );
		if ( !soundName[0] ) {
			break;
		}
		if ( soundName[0] == '*' ) {
			continue;	// custom sound
		}
		Q_strncpyz( buffer, soundName, sizeof(buffer) );
		if (buffer[strlen( buffer )-1] == '0' || buffer[strlen( buffer )-1] == '1')
			buffer[strlen( buffer )-1] = 0;
		cgs.gameSounds[i] = cgi.S_RegisterSound( buffer, qfalse );
	}
}


//===================================================================================


/*
=================
CG_RegisterGraphics

This function may execute for a couple of minutes with a slow disk.
=================
*/
static void CG_RegisterGraphics( void ) {
	int			i;
	const char	*str;

	// clear any references to old media
	memset( &cg.refdef, 0, sizeof( cg.refdef ) );
	cgi.R_ClearScene();

	CG_LoadingString( cgs.mapname );

	cgi.R_LoadWorldMap( cgs.mapname );

	// precache status bar pics
	CG_LoadingString( "game media" );


	cgs.media.crosshairShader[0] = cgi.R_RegisterShader( "gfx/2d/BLANK" );
	cgs.media.crosshairShader[1] = cgi.R_RegisterShader( "textures/hud/crosshair" );
	cgs.media.crosshairShader[2] = cgi.R_RegisterShader( "gfx/2d/crosshair" );

	cgs.media.backTileShader = cgi.R_RegisterShader( "gfx/2d/backtile" );

	cgs.media.shadowMarkShader = cgi.R_RegisterShader( "markShadow" );
	cgs.media.wakeMarkShader = cgi.R_RegisterShader( "wake" );

	// su44: MoHAA zoom overlays
	cgs.media.zoomOverlayShader = cgi.R_RegisterShader("textures/hud/zoomoverlay");
	cgs.media.kar98TopOverlayShader = cgi.R_RegisterShader("textures/hud/kartop.tga");
	cgs.media.kar98BottomOverlayShader = cgi.R_RegisterShader("textures/hud/karbottom.tga");
	cgs.media.binocularsOverlayShader = cgi.R_RegisterShader("textures/hud/binocularsoverlay");

	// register the inline models
	cgs.numInlineModels = cgi.CM_NumInlineModels();
	for ( i = 1 ; i < cgs.numInlineModels ; i++ ) {
		char	name[10];
		vec3_t			mins, maxs;
		int				j;

		Com_sprintf( name, sizeof(name), "*%i", i );
		cgs.inlineDrawModel[i] = cgi.R_RegisterModel( name );
		cgi.R_ModelBounds( cgs.inlineDrawModel[i], mins, maxs );
		for ( j = 0 ; j < 3 ; j++ ) {
			cgs.inlineModelMidpoints[i][j] = mins[j] + 0.5 * ( maxs[j] - mins[j] );
		}
	}

	// register all the server specified models
	for (i=1 ; i<MAX_MODELS ; i++) {
		const char		*modelName;

		modelName = CG_ConfigString( CS_MODELS+i );
		if ( !modelName[0] ) {
			break;
		}
		cgs.gameModels[i] = cgi.R_RegisterModel( modelName );
		if( modelName[0] != '*')	{
			cgs.gameTIKIs[i] = cgi.TIKI_RegisterModel( modelName );
			if(cgs.gameTIKIs[i] == 0) {
				CG_Printf("CG_RegisterGraphics: failed to load  tiki file  %s (%i)\n",modelName,i);
			}
		}
	}
	for (i=0; i<64; i++) {
		const char		*itemName;
		itemName = CG_ConfigString( CS_WEAPONS+i );
		if(itemName[0]) {
			CG_RegisterItemName(i, itemName);
		}
	}
	for( i = CS_RAIN_DENSITY; i != (CS_RAIN_NUMSHADERS+1); i++) {
		str = CG_ConfigString( i );
		CG_RainCSUpdated(i,str);
	}
	str = CG_ConfigString( CS_FOGINFO );
	sscanf(str, "%d %f %f %f %f", &cg.farplane_cull, &cg.farplane_distance,
		&cg.farplane_color[0], &cg.farplane_color[1], &cg.farplane_color[2]);

	CG_ClearParticles ();
}

/*
===================
CG_RegisterClients
===================
*/
static void CG_RegisterClients( void ) {
	int		i;

	CG_NewClientInfo(cg.clientNum);

	for (i=0 ; i<MAX_CLIENTS ; i++) {
		const char		*clientInfo;

		if (cg.clientNum == i) {
			continue;
		}

		clientInfo = CG_ConfigString( CS_PLAYERS+i );
		if ( !clientInfo[0]) {
			continue;
		}
		CG_NewClientInfo( i );
	}
}

//===========================================================================

/*
=================
CG_ConfigString
=================
*/
const char *CG_ConfigString( int index ) {
	if ( index < 0 || index >= MAX_CONFIGSTRINGS ) {
		CG_Error( "CG_ConfigString: bad index: %i", index );
	}
	return cgs.gameState.stringData + cgs.gameState.stringOffsets[ index ];
}

//==================================================================

/*
======================
CG_StartMusic

======================
*/
void CG_StartMusic( void ) {

	cgi.S_StartBackgroundTrack( CG_LoadMusic(CG_ConfigString(CS_MUSIC)),"" );
}

/*
=================
CG_Init

Called after every level change or subsystem restart
Will perform callbacks to make the loading info screen update.
=================
*/
void CG_Init( int serverMessageNum, int serverCommandSequence, int clientNum, int randomSeed ) {
	const char	*s;

	// clear everything
	memset( &cgs, 0, sizeof( cgs ) );
	memset( &cg, 0, sizeof( cg ) );
	memset( cg_entities, 0, sizeof(cg_entities) );

	srand( randomSeed );

	cg.clientNum = clientNum;

	cgs.processedSnapshotNum = serverMessageNum;
	cgs.serverCommandSequence = serverCommandSequence;

	cgi.R_RegisterFont( "facfont-20", 0, &cgs.media.facfont );
	cgi.R_RegisterFont( "verdana-14", 0, &cgs.media.verdana );

	// load a few needed things before we do any screen updates
//	cgs.media.charsetShader		= cgi.R_RegisterShader( "gfx/2d/bigchars" );
	cgs.media.whiteShader		= cgi.R_RegisterShader( "*white" );
//	cgs.media.charsetProp		= cgi.R_RegisterShaderNoMip( "menu/art/font1_prop.tga" );
//	cgs.media.charsetPropGlow	= cgi.R_RegisterShaderNoMip( "menu/art/font1_prop_glo.tga" );
//	cgs.media.charsetPropB		= cgi.R_RegisterShaderNoMip( "menu/art/font2_prop.tga" );
	cgs.media.blackShader = cgi.R_RegisterShaderNoMip( "textures/mohmenu/black.tga" );

	CG_RegisterCvars();

	CG_InitConsoleCommands();

	// get the rendering configuration from the client system
	cgi.GetGlconfig( &cgs.glconfig );
	cgs.screenXScale = cgs.glconfig.vidWidth / 640.0;
	cgs.screenYScale = cgs.glconfig.vidHeight / 480.0;

	// get the gamestate from the client system
	cgi.GetGameState( &cgs.gameState );

	// check version
	s = CG_ConfigString( CS_GAME_VERSION );
	if ( strcmp( s, GAME_VERSION ) ) {
		CG_Error( "Client/Server game mismatch: %s/%s", GAME_VERSION, s );
	}

	s = CG_ConfigString( CS_LEVEL_START_TIME );
	cgs.levelStartTime = atoi( s );

	CG_ParseServerinfo();

	// load the new map
	CG_LoadingString( "collision map" );

	cgi.CM_LoadMap( cgs.mapname );

	CG_LoadingString( "sounds" );

	CG_RegisterSounds();
	CG_LoadUbersound();

	CG_LoadingString( "graphics" );

	CG_RegisterGraphics();

	CG_LoadingString( "clients" );

	CG_RegisterClients();		// if low on memory, some clients will be deferred

	CG_InitLocalEntities();

	CG_InitMarkPolys();

	CG_InitEventSystem();

	CG_InitBeams();

	CG_InitRainEffect();

	// remove the last loading update
	cg.infoScreenText[0] = 0;

	// Make sure we have update values (scores)
	CG_SetConfigValues();

	CG_StartMusic();

	CG_LoadingString( "" );

	cgi.S_ClearLoopingSounds( qtrue );
}

/*
=================
CG_Shutdown

Called before every level change or subsystem restart
=================
*/
void CG_Shutdown( void ) {
	// some mods may need to do cleanup work here,
	// like closing files or archiving session data
}

/*
================
GetCGameAPI

Gets client game imports and returns client game exports
================
*/
clientGameExport_t* GetCGameAPI( clientGameImport_t *import )
{
	cgi = *import;

	cge.CG_Init				= CG_Init;
	cge.CG_Shutdown			= CG_Shutdown;

	cge.CG_ConsoleCommand	= CG_ConsoleCommand;

	cge.CG_DrawActiveFrame	= CG_DrawActiveFrame;
	cge.CG_Draw2D			= CG_Draw2D;

	cge.CG_CrosshairPlayer	= CG_CrosshairPlayer;
	cge.CG_LastAttacker		= CG_LastAttacker;

	cge.CG_KeyEvent			= CG_KeyEvent;
	cge.CG_MouseEvent		= CG_MouseEvent;
	cge.CG_EventHandling	= CG_EventHandling;

	cge.CG_CenterPrint		= CG_CenterPrint;
	cge.CG_LocationPrint	= CG_LocationPrint;

	cge.CG_ParseCGMessage	= CG_ParseCGMessage;

	return &cge;
}

/*
==================
CG_EventHandling
==================
 type 0 - no event handling
      1 - team menu
      2 - hud editor

*/

void CG_EventHandling(int type) {

}

void CG_KeyEvent(int key, qboolean down) {

}

void CG_MouseEvent(int x, int y) {

}


