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
// cg_servercmds.c -- reliably sequenced text commands sent by the server
// these are processed at snapshot transition time, so there will definately
// be a valid snapshot this frame

#include "cg_local.h"

/*
================
CG_ParseServerinfo

This is called explicitly when the gamestate is first received,
and whenever the server updates any serverinfo flagged cvars
================
*/
void CG_ParseServerinfo( void ) {
	const char	*info;
	char	*mapname;

	info = CG_ConfigString( CS_SERVERINFO );
	cgs.gametype = atoi( Info_ValueForKey( info, "g_gametype" ) );
	trap_Cvar_Set("g_gametype", va("%i", cgs.gametype));
	cgs.dmflags = atoi( Info_ValueForKey( info, "dmflags" ) );
	cgs.timelimit = atoi( Info_ValueForKey( info, "timelimit" ) );
	cgs.maxclients = atoi( Info_ValueForKey( info, "sv_maxclients" ) );
	mapname = Info_ValueForKey( info, "mapname" );
	Com_sprintf( cgs.mapname, sizeof( cgs.mapname ), "maps/%s.bsp", mapname );
}

/*
================
CG_SetConfigValues

Called on load to set the initial values from configure strings
================
*/
void CG_SetConfigValues( void ) {

	cgs.levelStartTime = atoi( CG_ConfigString( CS_LEVEL_START_TIME ) );
}

/*
=====================
CG_ShaderStateChanged
=====================
*/
void CG_ShaderStateChanged(void) {
	char originalShader[MAX_QPATH];
	char newShader[MAX_QPATH];
	char timeOffset[16];
	const char *o;
	char *n,*t;

	o = CG_ConfigString( CS_SHADERSTATE );
	while (o && *o) {
		n = strstr(o, "=");
		if (n && *n) {
			strncpy(originalShader, o, n-o);
			originalShader[n-o] = 0;
			n++;
			t = strstr(n, ":");
			if (t && *t) {
				strncpy(newShader, n, t-n);
				newShader[t-n] = 0;
			} else {
				break;
			}
			t++;
			o = strstr(t, "@");
			if (o) {
				strncpy(timeOffset, t, o-t);
				timeOffset[o-t] = 0;
				o++;
				trap_R_RemapShader( originalShader, newShader, timeOffset );
			}
		} else {
			break;
		}
	}
}

/*
================
CG_ConfigStringModified

================
*/
static void CG_ConfigStringModified( void ) {
	const char	*str;
	int		num;

	num = atoi( CG_Argv( 1 ) );

	// get the gamestate from the client system, which will have the
	// new configstring already integrated
	trap_GetGameState( &cgs.gameState );

	// look up the individual string that was modified
	str = CG_ConfigString( num );

	// do something with it if necessary
	if ( num == CS_MUSIC ) {
//		CG_StartMusic( );
	} else if ( num == CS_SERVERINFO ) {
		CG_ParseServerinfo();
	} else if ( num == CS_LEVEL_START_TIME ) {
		cgs.levelStartTime = atoi( str );
	} else if ( num >= CS_MODELS && num < CS_MODELS+MAX_MODELS ) {
		cgs.gameModels[ num-CS_MODELS ] = trap_R_RegisterModel( str );
		if(str[0] && str[0] != '*') {
			cgs.gameTIKIs[num-CS_MODELS] = trap_TIKI_RegisterModel( str );
			if(cgs.gameTIKIs[num-CS_MODELS] == 0) {
				CG_Printf("CG_ConfigStringModified: failed to load tiki file %s (%i)\n",str,num-CS_MODELS);
			}
		}
	} else if ( num >= CS_SOUNDS && num < CS_SOUNDS+MAX_SOUNDS ) {
		if ( str[0] != '*' ) {	// player specific sounds don't register here
			cgs.gameSounds[ num-CS_SOUNDS] = trap_S_RegisterSound( str, qfalse );
		}
	} else if ( num >= CS_PLAYERS && num < CS_PLAYERS+MAX_CLIENTS ) {

	} else if ( num >= CS_WEAPONS && num < CS_UNKNOWN) {

	}
	else if ( num == CS_SHADERSTATE ) {
		CG_ShaderStateChanged();
	}
		
}

/*
===============
CG_MapRestart

The server has issued a map_restart, so the next snapshot
is completely new and should not be interpolated to.

A tournement restart will clear everything, but doesn't
require a reload of all the media
===============
*/
static void CG_MapRestart( void ) {

//	CG_InitLocalEntities();
//	CG_InitMarkPolys();
//	CG_ClearParticles ();

	cg.mapRestart = qtrue;

	trap_S_ClearLoopingSounds(qtrue);

	// we really should clear more parts of cg here and stop sounds
}

/*
=================
CG_RemoveChatEscapeChar
=================
*/
static void CG_RemoveChatEscapeChar( char *text ) {
	int i, l;

	l = 0;
	for ( i = 0; text[i]; i++ ) {
		if (text[i] == '\x19')
			continue;
		text[l++] = text[i];
	}
	text[l] = '\0';
}

// Wombat: Lists of blocked and allowed stufftext commands
const char *stufftextBlock[] = {
	"quit"
};

typedef struct {
	char	*cmd;
	int		args;
} stufftextCommand_t;

stufftextCommand_t stufftextSafe[] = {
	{ "pushmenu_teamselect", 0 },
	{ "pushmenu_weaponselect", 0 },
	{ "wait", 1 }
};

/*
=================
CG_StufftextCommand

Wombat: MOHAA servers may send certain server commands
that contain console commands supposed to be executed
on the client's console.
I don't like the idea that a remote machine can execute
*anything*, so i think i will implement a filter to pass only
sane commands...
=================
*/
void CG_StufftextCommand( const char *cmd ) {
	int		i;

	for ( i = 0 ; i < sizeof( stufftextBlock ) / sizeof( stufftextBlock[0] ) ; i++ ) {
		if ( !Q_stricmp( cmd, stufftextBlock[i] ) ) {
			return;
		}
	}
	for ( i = 0 ; i < sizeof( stufftextSafe ) / sizeof( stufftextSafe[0] ) ; i++ ) {
		if ( !Q_stricmp( cmd, stufftextSafe[i].cmd ) ) {
			trap_SendConsoleCommand( cmd );
			return;
		}
	}
	Com_Printf( "Unlisted stufftext command: \"%s\"\n", cmd ); 
}

void	CG_Stopwatch_f( void ) {
	
}
void	CG_ServerLag_f( void ) {
	
}

/*
=================
CG_ServerCommand

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================
*/
static void CG_ServerCommand( void ) {
	const char	*cmd;
	char		text[MAX_SAY_TEXT];
	char		stufftextBuffer[512];
	const char *ptr;
	char		letter;
	int			i;
	int			lastpos;

	cmd = CG_Argv(0);

	if ( !cmd[0] ) {
		// server claimed the command
		return;
	}

	if ( !strcmp( cmd, "cp" ) ) {
		CG_CenterPrint( CG_Argv(1), SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH );
		return;
	}

	if ( !strcmp( cmd, "cs" ) ) {
		CG_ConfigStringModified();
		return;
	}
	


	if ( !strcmp( cmd, "print" ) ) {
		CG_Printf( "%s", CG_Argv(1) );
		return;
	}

	// wombat: hudprint is something specific to MOHAA
	if ( !strcmp( cmd, "hudprint" ) ) {
		CG_Printf( "HUD_Print %s", CG_Argv(1) );
		return;
	}
	
	// wombat: stufftext is something specific to MOHAA
	if ( !strcmp( cmd, "stufftext" ) ) {
		ptr = CG_Argv(1);
		for ( i=0,lastpos=0; i<sizeof(stufftextBuffer);i++ ) {
			letter =  *(ptr + i);
			if ( i == sizeof(stufftextBuffer) && letter != 0 ) {
				Com_Printf( "WARNING: Max. stufftext command length (%i) exceeded.\n", sizeof(stufftextBuffer) );
				return;
			}			
			if ( letter == ';' ) {
				letter = 0;
				stufftextBuffer[i] =letter;
				CG_StufftextCommand( &stufftextBuffer[lastpos] );
				lastpos = i+1;
			}
			else if ( letter == 0 ) {
				stufftextBuffer[i] =letter;
				CG_StufftextCommand( &stufftextBuffer[lastpos] );
				return;
			}
			else stufftextBuffer[i] =letter;
		}
		return;
	}
	
	if ( !strcmp( cmd, "stopwatch" ) ) {
		CG_Stopwatch_f();
		return;
	}
	if ( !strcmp( cmd, "svlag" ) ) {
		CG_ServerLag_f();
		return;
	}

	if ( !strcmp( cmd, "chat" ) ) {
		return;
	}

	if ( !strcmp( cmd, "tchat" ) ) {
		return;
	}

	if ( !strcmp( cmd, "tinfo" ) ) {
		return;
	}

	if ( !strcmp( cmd, "map_restart" ) ) {
		CG_MapRestart();
		return;
	}

	if ( Q_stricmp (cmd, "remapShader") == 0 )
	{
		if (trap_Argc() == 4)
		{
			char shader1[MAX_QPATH];
			char shader2[MAX_QPATH];
			char shader3[MAX_QPATH];

			Q_strncpyz(shader1, CG_Argv(1), sizeof(shader1));
			Q_strncpyz(shader2, CG_Argv(2), sizeof(shader2));
			Q_strncpyz(shader3, CG_Argv(3), sizeof(shader3));

			trap_R_RemapShader(shader1, shader2, shader3);
		}
		
		return;
	}

	CG_Printf( "Unknown client game command: %s\n", cmd );
}


/*
====================
CG_ExecuteNewServerCommands

Execute all of the server commands that were received along
with this this snapshot.
====================
*/
void CG_ExecuteNewServerCommands( int latestSequence ) {
	while ( cgs.serverCommandSequence < latestSequence ) {
		if ( trap_GetServerCommand( ++cgs.serverCommandSequence ) ) {
			CG_ServerCommand();
		}
	}
}
