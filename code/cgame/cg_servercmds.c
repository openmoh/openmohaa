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
	cgi.Cvar_Set("g_gametype", va("%i", cgs.gametype));
	cgs.dmflags = atoi( Info_ValueForKey( info, "dmflags" ) );
	cgs.teamflags = atoi( Info_ValueForKey( info, "teamflags" ) );
	cgs.fraglimit = atoi( Info_ValueForKey( info, "fraglimit" ) );
	cgs.capturelimit = atoi( Info_ValueForKey( info, "capturelimit" ) );
	cgs.timelimit = atoi( Info_ValueForKey( info, "timelimit" ) );
	cgs.maxclients = atoi( Info_ValueForKey( info, "sv_maxclients" ) );
	mapname = Info_ValueForKey( info, "mapname" );
	Com_sprintf( cgs.mapname, sizeof( cgs.mapname ), "maps/%s.bsp", mapname );
	cgi.Cvar_Set( "cg_scoreboardpic", Info_ValueForKey( info,"g_scoreboardpic" ) );
}

/*
==================
CG_ParseWarmup
==================
*/
static void CG_ParseWarmup( void ) {
	const char	*info;
	int			warmup;

	info = CG_ConfigString( CS_WARMUP );

	warmup = atoi( info );
	cg.warmupCount = -1;

	if ( warmup == 0 && cg.warmup ) {

	} else if ( warmup > 0 && cg.warmup <= 0 ) {

		{
//			cgi.S_StartLocalSound( cgs.media.countPrepareSound, CHAN_ANNOUNCER );
		}
	}

	cg.warmup = warmup;
}

/*
================
CG_SetConfigValues

Called on load to set the initial values from configure strings
================
*/
void CG_SetConfigValues( void ) {
	cgs.levelStartTime = atoi( CG_ConfigString( CS_LEVEL_START_TIME ) );
	cg.warmup = atoi( CG_ConfigString( CS_WARMUP ) );
}

/*
================
CG_ConfigStringModified

================
*/
static void CG_ConfigStringModified( void ) {
	const char	*str;
	int		num;
	char buffer[MAX_QPATH];

	num = atoi( CG_Argv( 1 ) );

	// get the gamestate from the client system, which will have the
	// new configstring already integrated
	cgi.GetGameState( &cgs.gameState );

	// look up the individual string that was modified
	str = CG_ConfigString( num );

	// do something with it if necessary
	if ( num == CS_MUSIC ) {
		CG_StartMusic();
	} else if ( num == CS_SERVERINFO ) {
		CG_ParseServerinfo();
	} else if ( num == CS_WARMUP ) {
		CG_ParseWarmup();
	} else if( num >= CS_RAIN_DENSITY && num <= CS_RAIN_NUMSHADERS) {
		CG_RainCSUpdated(num,str);
	} else if ( num == CS_FOGINFO ) {
		sscanf(str, "%d %f %f %f %f", &cg.farplane_cull, &cg.farplane_distance,
			&cg.farplane_color[0], &cg.farplane_color[1], &cg.farplane_color[2]);
	} else if ( num == CS_LEVEL_START_TIME ) {
		cgs.levelStartTime = atoi( str );
	} else if ( num >= CS_MODELS && num < CS_MODELS+MAX_MODELS ) {
		cgs.gameModels[ num-CS_MODELS ] = cgi.R_RegisterModel( str );
		if(str[0] && str[0] != '*') {
			cgs.gameTIKIs[num-CS_MODELS] = cgi.TIKI_RegisterModel( str );
			if(cgs.gameTIKIs[num-CS_MODELS] == 0) {
				CG_Printf("CG_ConfigStringModified: failed to load tiki file %s (%i)\n",str,num-CS_MODELS);
			}
		}
	} else if ( num >= CS_SOUNDS && num < CS_SOUNDS+MAX_SOUNDS ) {
		if ( str[0] != '*' ) {	// player specific sounds don't register here
			Q_strncpyz( buffer, str, sizeof(buffer) );
			if (buffer[strlen( buffer )-1] == '0' || buffer[strlen( buffer )-1] == '1')
				buffer[strlen( buffer )-1] = 0;
			cgs.gameSounds[ num-CS_SOUNDS] = cgi.S_RegisterSound( buffer, qfalse );
		}
	} else if ( num >= CS_PLAYERS && num < CS_PLAYERS+MAX_CLIENTS ) {
		CG_NewClientInfo( num - CS_PLAYERS );
	} else if( num >= CS_WEAPONS && num < CS_TEAMS ) {
		CG_RegisterItemName(num - CS_WEAPONS, str);
	} 	
}


/*
=======================
CG_AddToTeamChat

=======================
*/
static void CG_AddToTeamChat( const char *str ) {
	int len;
	char *p, *ls;
	int lastcolor;
	int chatHeight;

	if (cg_teamChatHeight->integer < TEAMCHAT_HEIGHT) {
		chatHeight = cg_teamChatHeight->integer;
	} else {
		chatHeight = TEAMCHAT_HEIGHT;
	}

	if (chatHeight <= 0 || cg_teamChatTime->integer <= 0) {
		// team chat disabled, dump into normal chat
		cgs.teamChatPos = cgs.teamLastChatPos = 0;
		return;
	}

	len = 0;

	p = cgs.teamChatMsgs[cgs.teamChatPos % chatHeight];
	*p = 0;

	lastcolor = '7';

	ls = NULL;
	while (*str) {
		if (len > TEAMCHAT_WIDTH - 1) {
			if (ls) {
				str -= (p - ls);
				str++;
				p -= (p - ls);
			}
			*p = 0;

			cgs.teamChatMsgTimes[cgs.teamChatPos % chatHeight] = cg.time;

			cgs.teamChatPos++;
			p = cgs.teamChatMsgs[cgs.teamChatPos % chatHeight];
			*p = 0;
			*p++ = Q_COLOR_ESCAPE;
			*p++ = lastcolor;
			len = 0;
			ls = NULL;
		}

		if ( Q_IsColorString( str ) ) {
			*p++ = *str++;
			lastcolor = *str;
			*p++ = *str++;
			continue;
		}
		if (*str == ' ') {
			ls = p;
		}
		*p++ = *str++;
		len++;
	}
	*p = 0;

	cgs.teamChatMsgTimes[cgs.teamChatPos % chatHeight] = cg.time;
	cgs.teamChatPos++;

	if (cgs.teamChatPos - cgs.teamLastChatPos > chatHeight)
		cgs.teamLastChatPos = cgs.teamChatPos - chatHeight;
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
	if ( cg_showmiss->integer ) {
		CG_Printf( "CG_MapRestart\n" );
	}

	CG_InitLocalEntities();
	CG_InitMarkPolys();
	CG_ClearParticles ();

	// make sure the "3 frags left" warnings play again
	cg.fraglimitWarnings = 0;

	cg.timelimitWarnings = 0;

	cg.mapRestart = qtrue;

	CG_StartMusic();

	cgi.S_ClearLoopingSounds(qtrue);

	// we really should clear more parts of cg here and stop sounds

	// play the "fight" sound if this is a restart without warmup
	if ( cg.warmup == 0 /* && cgs.gametype == GT_TOURNAMENT */) {
//		cgi.S_StartLocalSound( cgs.media.countFightSound, CHAN_ANNOUNCER );
		CG_CenterPrint( "FIGHT!", 120, GIANTCHAR_WIDTH*2 );
	}

	cgi.Cvar_Set("cg_thirdPerson", "0");
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
	{ "wait", 1 },
	{ "locationprint", 3 }, // locationprint <x> <y> <text ... >
	{ "add", 2 }, // add <variable> <amount>
	// su44: these are used on "-(V)ethod-Clan" server ( 8.9.16.34:12203 )
	// for some kind of anticheat
	{ "set", 2 }, // set <variable> <value>
	{ "setu", 2 }, // setu <variable> <value>
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
	int			i;
	char		*ptr;
	const char	*ptr2;

	ptr = ( char * )cmd;
	ptr2 = COM_Parse(&ptr);
	for ( i = 0 ; i < sizeof( stufftextBlock ) / sizeof( stufftextBlock[0] ) ; i++ ) {
		if ( !Q_stricmp( ptr2, stufftextBlock[i] ) ) {
			return;
		}
	}
	// wombat: some servers use ST commands for anti-cheat measures
	// we don't use the whitelist right now...
	Com_Printf( "^3Executing stufftext command: \"%s\"\n", cmd ); 
	cgi.SendConsoleCommand( cmd );
	return;

	for ( i = 0 ; i < sizeof( stufftextSafe ) / sizeof( stufftextSafe[0] ) ; i++ ) {
		if ( !Q_stricmp( ptr2, stufftextSafe[i].cmd ) ) {
			cgi.SendConsoleCommand( cmd );
			return;
		}
	}
	Com_Printf( "Unlisted stufftext command: \"%s\"\n", cmd ); 
}

/*
=================
CG_AddGameMessage

To be printed below the compass
=================
*/
void CG_AddGameMessage( const char *cmd, serverMessageType_t smt ) {
	ubersound_t *click;

	click = CG_GetUbersound( "click" );
	switch ( smt ) {
		case SMT_YELLOW:
		case SMT_WHITE:
			if ( cg.gameMessageTime == 0 )
				cg.gameMessageTime = cg.time;
			cg.gameMessageTypes[cg.gameMessagePtr2] = smt;
			Q_strncpyz( cg.gameMessages[cg.gameMessagePtr2], cmd, sizeof(cg.gameMessages[cg.gameMessagePtr2]) );
			CG_Printf( "Game Message: %s", cmd );
			cgi.S_StartLocalSound( click->sfxHandle, click->channel );
			cg.gameMessagePtr2 = (cg.gameMessagePtr2+1) % MAX_GAMEMESSAGES;
			if ( cg.gameMessagePtr2 == cg.gameMessagePtr1 ) {
				cg.gameMessagePtr1 = (cg.gameMessagePtr1+1) % MAX_GAMEMESSAGES;
				cg.gameMessageTime = cg.time;
			}
			break;
		case SMT_CHAT:
			CG_Printf( "Chat: %s", cmd );
			cgi.S_StartLocalSound( click->sfxHandle, click->channel );
		case SMT_DEATH:
			if ( cg.chatDeathMessageTime == 0 )
				cg.chatDeathMessageTime = cg.time;
			cg.chatDeathMessageTypes[cg.chatDeathMessagePtr2] = smt;
			Q_strncpyz( cg.chatDeathMessages[cg.chatDeathMessagePtr2], cmd, sizeof(cg.chatDeathMessages[cg.chatDeathMessagePtr2]) );
			if ( smt == SMT_DEATH )
				CG_Printf( "Death: %s", cmd );
			cg.chatDeathMessagePtr2 = (cg.chatDeathMessagePtr2+1) % MAX_CHATDEATHMESSAGES;
			if (cg.chatDeathMessagePtr2 == cg.chatDeathMessagePtr1 ) {
				cg.chatDeathMessagePtr1 = (cg.chatDeathMessagePtr1+1) % MAX_CHATDEATHMESSAGES;
				cg.chatDeathMessageTime = cg.time;
			}
			break;
		case SMT_UNKNOWN:
			CG_Printf( "Unknown Message: %s", cmd );
			break;
	}
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

	if ( !strcmp( cmd, "cp" ) ) {
		CG_CenterPrint( CG_Argv(1), SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH );
		return;
	}

	if ( !strcmp( cmd, "cs" ) ) {
		CG_ConfigStringModified();
		return;
	}

	if ( !strcmp( cmd, "print" ) ) {
		// TODO: print to screen, too
		const char *ptr;
		serverMessageType_t smt;

		ptr = CG_Argv(1);
		smt = (serverMessageType_t)*ptr;

		if ( smt < SMT_MAX )
			CG_AddGameMessage( ptr + 1, smt );
		else
			CG_Printf( "%s", ptr );

		return;
	}

	// su44: chats are handled differently in MoH
//	if ( !strcmp( cmd, "chat" ) ) {
//		if ( !cg_teamChatsOnly->integer ) {
////			cgi.S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
//			Q_strncpyz( text, CG_Argv(1), MAX_SAY_TEXT );
//			CG_RemoveChatEscapeChar( text );
//			CG_Printf( "%s\n", text );
//		}
//		return;
//	}
//
//	if ( !strcmp( cmd, "tchat" ) ) {
////		cgi.S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
//		Q_strncpyz( text, CG_Argv(1), MAX_SAY_TEXT );
//		CG_RemoveChatEscapeChar( text );
//		CG_AddToTeamChat( text );
//		CG_Printf( "%s\n", text );
//		return;
//	}

	if ( !strcmp( cmd, "scores" ) ) {
		//CG_ParseScores();
		Q_strncpyz( cg.aScore, cgi.Args(), sizeof( cg.aScore ) );
		return;
	}

	//if ( !strcmp( cmd, "map_restart" ) ) {
	//	CG_MapRestart();
	//	return;
	//}

	// clientLevelShot is sent before taking a special screenshot for
	// the menu system during development
	if ( !strcmp( cmd, "clientLevelShot" ) ) {
		cg.levelShot = qtrue;
		return;
	}

	// MoHAA's stopwatch
	if ( !strcmp( cmd, "stopwatch" ) ) {
		// TODO		
		return;
	}

	if ( !strcmp( cmd, "svlag" ) ) {
		// TODO
		return;
	}

	// su44: send just before server map change
	if ( !strcmp( cmd, "fadesound" ) ) {
		// TODO
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
		if ( cgi.GetServerCommand( ++cgs.serverCommandSequence ) ) {
			CG_ServerCommand();
		}
	}
}
