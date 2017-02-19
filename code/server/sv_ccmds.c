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

#include "../client/client.h"
#include "../client/snd_local.h"
#include "server.h"

/*
===============================================================================

OPERATOR CONSOLE ONLY COMMANDS

These commands can only be entered from stdin or by a remote operator datagram
===============================================================================
*/


/*
==================
SV_GetPlayerByHandle

Returns the player with player id or name from Cmd_Argv(1)
==================
*/
static client_t *SV_GetPlayerByHandle( void ) {
	client_t	*cl;
	int			i;
	char		*s;
	char		cleanName[64];

	// make sure server is running
	if ( !com_sv_running->integer ) {
		return NULL;
	}

	if ( Cmd_Argc() < 2 ) {
		Com_Printf( "No player specified.\n" );
		return NULL;
	}

	s = Cmd_Argv(1);

	// Check whether this is a numeric player handle
	for(i = 0; s[i] >= '0' && s[i] <= '9'; i++);
	
	if(!s[i])
	{
		int plid = atoi(s);

		// Check for numeric playerid match
		if(plid >= 0 && plid < sv_maxclients->integer)
		{
			cl = &svs.clients[plid];
			
			if(cl->state)
				return cl;
		}
	}

	// check for a name match
	for ( i=0, cl=svs.clients ; i < sv_maxclients->integer ; i++,cl++ ) {
		if ( !cl->state ) {
			continue;
		}
		if ( !Q_stricmp( cl->name, s ) ) {
			return cl;
		}

		Q_strncpyz( cleanName, cl->name, sizeof(cleanName) );
		Q_CleanStr( cleanName );
		if ( !Q_stricmp( cleanName, s ) ) {
			return cl;
		}
	}

	Com_Printf( "Player %s is not on the server\n", s );

	return NULL;
}

/*
==================
SV_GetPlayerByNum

Returns the player with idnum from Cmd_Argv(1)
==================
*/
static client_t *SV_GetPlayerByNum( void ) {
	client_t	*cl;
	int			i;
	int			idnum;
	char		*s;

	// make sure server is running
	if ( !com_sv_running->integer ) {
		return NULL;
	}

	if ( Cmd_Argc() < 2 ) {
		Com_Printf( "No player specified.\n" );
		return NULL;
	}

	s = Cmd_Argv(1);

	for (i = 0; s[i]; i++) {
		if (s[i] < '0' || s[i] > '9') {
			Com_Printf( "Bad slot number: %s\n", s);
			return NULL;
		}
	}
	idnum = atoi( s );
	if ( idnum < 0 || idnum >= sv_maxclients->integer ) {
		Com_Printf( "Bad client slot: %i\n", idnum );
		return NULL;
	}

	cl = &svs.clients[idnum];
	if ( !cl->state ) {
		Com_Printf( "Client %i is not active\n", idnum );
		return NULL;
	}
	return cl;
}

//=========================================================


/*
==================
SV_Map_f

Restart the server on a different map
==================
*/
static void SV_Map_f( void ) {
	const char	*cmd;
	char		expanded[ MAX_QPATH ];
	char		map[ MAX_QPATH ];
	char		mapname[ MAX_QPATH ];
	const char	*spawnpos;

	if( Cmd_Argc() != 2 ) {
		Com_Printf( "USAGE: map <map>\n" );
		return;
	}

	if( developer->integer && *fs_mapdir->string )
	{
		Com_sprintf( mapname, sizeof( mapname ), "%s/%s", fs_mapdir->string, Cmd_Argv( 1 ) );
	}
	else
	{
		Com_sprintf( mapname, sizeof( mapname ), "%s", Cmd_Argv( 1 ) );
	}

	Com_BackslashToSlash( mapname );

	spawnpos = strchr( mapname, '$' );
	if( spawnpos )
	{
		Q_strncpyz( map, mapname, spawnpos - mapname + 1 );
	}
	else
	{
		strcpy( map, mapname );
	}

	// make sure the level exists before trying to change, so that
	// a typo at the server console won't end the game
	Com_sprintf( expanded, sizeof( expanded ), "maps/%s.bsp", map );
	if( FS_ReadFile( expanded, NULL ) == -1 ) {
		Com_Printf( "Can't find map %s\n", expanded );
		return;
	}

	cmd = Cmd_Argv( 0 );

	if( !Q_stricmpn( cmd, "sp", 2 ) )
	{
		Cvar_Set( "g_gametype", "0" );
		Cvar_Set( "sv_maxclients", "1" );
	}

	Cvar_Get( "g_gametype", "0", CVAR_LATCH | CVAR_SERVERINFO );
	Cvar_Get( "sv_maxclients", "1", CVAR_LATCH | CVAR_SERVERINFO );

	if( !strnicmp( map, "dm/", 3 ) && g_gametype->integer == GT_OBJECTIVE )
	{
		Com_Printf( "Can't load regular dm map in objective game type\n" );
		return;
	}

	if( !strnicmp( map, "obj/", 4 ) && g_gametype->integer != GT_OBJECTIVE )
	{
		Com_Printf( "Can't load objective map in non-objective game type\n" );
		return;
	}

	if( !developer->integer )
	{
		if( svs.iNumClients == 1 )
		{
			Cvar_Set( "cheats", "1" );
		}
		else if( strstr( cmd, "devmap" ) )
		{
			Cvar_Set( "cheats", " 1" );
		}
		else
		{
			Cvar_Set( "cheats", "0" );
		}
	}

	Cvar_SaveGameRestart_f();

	// start up the map
	SV_SpawnServer( mapname, qfalse, qfalse, qfalse );

	if( !g_gametype->integer ) {
		svs.autosave = qtrue;
	}
}

/*
==================
SV_GameMap_f

Restart the server on a different map
==================
*/
static void SV_GameMap_f( void ) {
	const char	*map;
	qboolean	bTransition;

	if( Cmd_Argc() != 2 ) {
		Com_Printf( "USAGE: gamemap <map>\n" );
		return;
	}

	map = Cmd_Argv( 1 );
	Com_DPrintf( "SV_GameMap(%s)\n", map );

	strcpy( svs.gameName, "current" );
	Cvar_SaveGameRestart_f();

	bTransition = sv.state == SS_GAME;

	// save persistant data
	if( bTransition ) {
		SV_ArchivePersistantFile( qfalse );
	}

	SV_SpawnServer( map, qfalse, qfalse, bTransition );

	if( !g_gametype->integer ) {
		svs.autosave = qtrue;
	}
}

/*
================
SV_MapRestart_f

Completely restarts a level, but doesn't send a new gamestate to the clients.
This allows fair starts with variable load times.
================
*/
static void SV_MapRestart_f( void ) {
	if( com_frameTime == sv.serverId ) {
		return;
	}

	if( !com_sv_running->integer ) {
		Com_Printf( "Server is not running.\n" );
		return;
	}

	Cvar_SaveGameRestart_f();
	SV_SpawnServer( svs.rawServerName, qfalse, qtrue, qfalse );
}

//===============================================================

/*
==================
SV_Kick_f

Kick a user off of the server  FIXME: move to game
==================
*/
static void SV_Kick_f( void ) {
	client_t	*cl;
	int			i;

	// make sure server is running
	if ( !com_sv_running->integer ) {
		Com_Printf( "Server is not running.\n" );
		return;
	}

	if ( Cmd_Argc() != 2 ) {
		Com_Printf ("Usage: kick <player name>\nkick all = kick everyone\nkick allbots = kick all bots\n");
		return;
	}

	cl = SV_GetPlayerByHandle();
	if ( !cl ) {
		if ( !Q_stricmp(Cmd_Argv(1), "all") ) {
			for ( i=0, cl=svs.clients ; i < sv_maxclients->integer ; i++,cl++ ) {
				if ( !cl->state ) {
					continue;
				}
				if( cl->netchan.remoteAddress.type == NA_LOOPBACK ) {
					continue;
				}
				SV_DropClient( cl, "was kicked" );
				cl->lastPacketTime = svs.time;	// in case there is a funny zombie
			}
		}
		else if ( !Q_stricmp(Cmd_Argv(1), "allbots") ) {
			for ( i=0, cl=svs.clients ; i < sv_maxclients->integer ; i++,cl++ ) {
				if ( !cl->state ) {
					continue;
				}
				if( cl->netchan.remoteAddress.type != NA_BOT ) {
					continue;
				}
				SV_DropClient( cl, "was kicked" );
				cl->lastPacketTime = svs.time;	// in case there is a funny zombie
			}
		}
		return;
	}
	if( cl->netchan.remoteAddress.type == NA_LOOPBACK ) {
		SV_SendServerCommand(NULL, "print \"%s\"", "Cannot kick host player\n");
		return;
	}

	SV_DropClient( cl, "was kicked" );
	cl->lastPacketTime = svs.time;	// in case there is a funny zombie
}

/*
==================
SV_Ban_f

Ban a user from being able to play on this server through the auth
server
==================
*/
static void SV_Ban_f( void ) {
	client_t	*cl;

	// make sure server is running
	if ( !com_sv_running->integer ) {
		Com_Printf( "Server is not running.\n" );
		return;
	}

	if ( Cmd_Argc() != 2 ) {
		Com_Printf ("Usage: banUser <player name>\n");
		return;
	}

	cl = SV_GetPlayerByHandle();

	if (!cl) {
		return;
	}

	if( cl->netchan.remoteAddress.type == NA_LOOPBACK ) {
		SV_SendServerCommand(NULL, "print \"%s\"", "Cannot kick host player\n");
		return;
	}

	// look up the authorize server's IP
	if ( !svs.authorizeAddress.ip[0] && svs.authorizeAddress.type != NA_BAD ) {
		Com_Printf( "Resolving %s\n", AUTHORIZE_SERVER_NAME );
		if ( !NET_StringToAdr( AUTHORIZE_SERVER_NAME, &svs.authorizeAddress ) ) {
			Com_Printf( "Couldn't resolve address\n" );
			return;
		}
		svs.authorizeAddress.port = BigShort( PORT_AUTHORIZE );
		Com_Printf( "%s resolved to %i.%i.%i.%i:%i\n", AUTHORIZE_SERVER_NAME,
			svs.authorizeAddress.ip[0], svs.authorizeAddress.ip[1],
			svs.authorizeAddress.ip[2], svs.authorizeAddress.ip[3],
			BigShort( svs.authorizeAddress.port ) );
	}

	// otherwise send their ip to the authorize server
	if ( svs.authorizeAddress.type != NA_BAD ) {
		NET_OutOfBandPrint( NS_SERVER, svs.authorizeAddress,
			"banUser %i.%i.%i.%i", cl->netchan.remoteAddress.ip[0], cl->netchan.remoteAddress.ip[1], 
								   cl->netchan.remoteAddress.ip[2], cl->netchan.remoteAddress.ip[3] );
		Com_Printf("%s was banned from coming back\n", cl->name);
	}
}

/*
==================
SV_BanNum_f

Ban a user from being able to play on this server through the auth
server
==================
*/
static void SV_BanNum_f( void ) {
	client_t	*cl;

	// make sure server is running
	if ( !com_sv_running->integer ) {
		Com_Printf( "Server is not running.\n" );
		return;
	}

	if ( Cmd_Argc() != 2 ) {
		Com_Printf ("Usage: banClient <client number>\n");
		return;
	}

	cl = SV_GetPlayerByNum();
	if ( !cl ) {
		return;
	}
	if( cl->netchan.remoteAddress.type == NA_LOOPBACK ) {
		SV_SendServerCommand(NULL, "print \"%s\"", "Cannot kick host player\n");
		return;
	}

	// look up the authorize server's IP
	if ( !svs.authorizeAddress.ip[0] && svs.authorizeAddress.type != NA_BAD ) {
		Com_Printf( "Resolving %s\n", AUTHORIZE_SERVER_NAME );
		if ( !NET_StringToAdr( AUTHORIZE_SERVER_NAME, &svs.authorizeAddress ) ) {
			Com_Printf( "Couldn't resolve address\n" );
			return;
		}
		svs.authorizeAddress.port = BigShort( PORT_AUTHORIZE );
		Com_Printf( "%s resolved to %i.%i.%i.%i:%i\n", AUTHORIZE_SERVER_NAME,
			svs.authorizeAddress.ip[0], svs.authorizeAddress.ip[1],
			svs.authorizeAddress.ip[2], svs.authorizeAddress.ip[3],
			BigShort( svs.authorizeAddress.port ) );
	}

	// otherwise send their ip to the authorize server
	if ( svs.authorizeAddress.type != NA_BAD ) {
		NET_OutOfBandPrint( NS_SERVER, svs.authorizeAddress,
			"banUser %i.%i.%i.%i", cl->netchan.remoteAddress.ip[0], cl->netchan.remoteAddress.ip[1], 
								   cl->netchan.remoteAddress.ip[2], cl->netchan.remoteAddress.ip[3] );
		Com_Printf("%s was banned from coming back\n", cl->name);
	}
}

/*
==================
SV_KickNum_f

Kick a user off of the server  FIXME: move to game
==================
*/
static void SV_KickNum_f( void ) {
	client_t	*cl;

	// make sure server is running
	if ( !com_sv_running->integer ) {
		Com_Printf( "Server is not running.\n" );
		return;
	}

	if ( Cmd_Argc() != 2 ) {
		Com_Printf ("Usage: kicknum <client number>\n");
		return;
	}

	cl = SV_GetPlayerByNum();
	if ( !cl ) {
		return;
	}
	if( cl->netchan.remoteAddress.type == NA_LOOPBACK ) {
		SV_SendServerCommand(NULL, "print \"%s\"", "Cannot kick host player\n");
		return;
	}

	SV_DropClient( cl, "was kicked" );
	cl->lastPacketTime = svs.time;	// in case there is a funny zombie
}

/*
================
SV_Status_f
================
*/
static void SV_Status_f( void ) {
	int			i;
	size_t		j, l;
	client_t	*cl;
	playerState_t	*ps;
	const char		*s;
	int			ping;

	// make sure server is running
	if ( !com_sv_running->integer ) {
		Com_Printf( "Server is not running.\n" );
		return;
	}

	Com_Printf ("map: %s\n", sv_mapname->string );

	Com_Printf ("num score ping name            lastmsg address               qport rate\n");
	Com_Printf ("--- ----- ---- --------------- ------- --------------------- ----- -----\n");
	for (i=0,cl=svs.clients ; i < sv_maxclients->integer ; i++,cl++)
	{
		if (!cl->state)
			continue;
		Com_Printf ("%3i ", i);
		ps = SV_GameClientNum( i );
		// su44: ps->persistant is not avaible in mohaa
		//Com_Printf ("%5i ", ps->persistant[PERS_SCORE]);
		Com_Printf ("%5i ", 0);

		if (cl->state == CS_CONNECTED)
			Com_Printf ("CNCT ");
		else if (cl->state == CS_ZOMBIE)
			Com_Printf ("ZMBI ");
		else
		{
			ping = cl->ping < 9999 ? cl->ping : 9999;
			Com_Printf ("%4i ", ping);
		}

		Com_Printf ("%s", cl->name);
    // TTimo adding a ^7 to reset the color
    // NOTE: colored names in status breaks the padding (WONTFIX)
    Com_Printf ("^7");
		l = 16 - strlen(cl->name);
		for (j=0 ; j<l ; j++)
			Com_Printf (" ");

		Com_Printf ("%7i ", svs.time - cl->lastPacketTime );

		s = NET_AdrToString( cl->netchan.remoteAddress );
		Com_Printf ("%s", s);
		l = 22 - strlen(s);
		for (j=0 ; j<l ; j++)
			Com_Printf (" ");
		
		Com_Printf ("%5i", cl->netchan.qport);

		Com_Printf (" %5i", cl->rate);

		Com_Printf ("\n");
	}
	Com_Printf ("\n");
}

/*
==================
SV_ConSay_f
==================
*/
static void SV_ConSay_f(void) {
	char	*p;
	char	text[1024];

	// make sure server is running
	if ( !com_sv_running->integer ) {
		Com_Printf( "Server is not running.\n" );
		return;
	}

	if ( Cmd_Argc () < 2 ) {
		return;
	}

	strcpy (text, "console: ");
	p = Cmd_Args();

	if ( *p == '"' ) {
		p++;
		p[strlen(p)-1] = 0;
	}

	strcat(text, p);

	SV_SendServerCommand(NULL, "chat \"%s\n\"", text);
}


/*
==================
SV_Heartbeat_f

Also called by SV_DropClient, SV_DirectConnect, and SV_SpawnServer
==================
*/
void SV_Heartbeat_f( void ) {
	svs.nextHeartbeatTime = -9999999;
}


/*
===========
SV_Serverinfo_f

Examine the serverinfo string
===========
*/
static void SV_Serverinfo_f( void ) {
	Com_Printf ("Server info settings:\n");
	Info_Print ( Cvar_InfoString( CVAR_SERVERINFO ) );
}


/*
===========
SV_Systeminfo_f

Examine or change the serverinfo string
===========
*/
static void SV_Systeminfo_f( void ) {
	Com_Printf ("System info settings:\n");
	Info_Print ( Cvar_InfoString( CVAR_SYSTEMINFO ) );
}


/*
===========
SV_DumpUser_f

Examine all a users info strings FIXME: move to game
===========
*/
static void SV_DumpUser_f( void ) {
	client_t	*cl;

	// make sure server is running
	if ( !com_sv_running->integer ) {
		Com_Printf( "Server is not running.\n" );
		return;
	}

	if ( Cmd_Argc() != 2 ) {
		Com_Printf ("Usage: info <userid>\n");
		return;
	}

	cl = SV_GetPlayerByHandle();
	if ( !cl ) {
		return;
	}

	Com_Printf( "userinfo\n" );
	Com_Printf( "--------\n" );
	Info_Print( cl->userinfo );
}


/*
=================
SV_KillServer
=================
*/
static void SV_KillServer_f( void ) {
	SV_Shutdown( "killserver" );
}

/*
=================
SV_EasyMode_f
=================
*/
void SV_EasyMode_f( void )
{
	Cvar_Set( "g_maxplayerhealth", "1500" );
	Cvar_Set( "skill", "0" );
	Com_Printf( "You are now setup for easy mode.\n" );
}

/*
=================
SV_MediumMode_f
=================
*/
void SV_MediumMode_f( void )
{
	Cvar_Set( "g_maxplayerhealth", "750" );
	Cvar_Set( "skill", "1" );
	Com_Printf( "You are now setup for medium mode.\n" );
}

/*
=================
SV_HardMode_f
=================
*/
void SV_HardMode_f( void )
{
	Cvar_Set( "g_maxplayerhealth", "250" );
	Cvar_Set( "skill", "2" );
	Com_Printf( "You are now setup for hard mode.\n" );
}

/*
=================
SV_LoadLastGame_f
=================
*/
void SV_LoadLastGame_f( void )
{
	Cbuf_AddText( va( "loadgame %s\n", Cvar_Get( "g_lastsave", "", 0 )->string ) );
}

#if 0

/*
=================
SV_TIKI_f
=================
*/
static void SV_TIKI_f( void ) {
	char		*fname;
	fname = Cmd_Argv(1);
	if(TIKI_RegisterModel(fname)==0)
	{
		char tmp[128];
		strcpy(tmp,"models/");
		strcat(tmp,fname);
		TIKI_RegisterModel(tmp);
	}
}

/*
=================
SV_TIKI_DumpBones_f
=================
*/
static void SV_TIKI_DumpBones_f( void ) {
	char		*fname;
	tiki_t		*tiki;
	int i;
	int filter = -1;

	fname = Cmd_Argv(1);
	tiki = TIKI_RegisterModel(fname);
	if(tiki==0)
	{
		char tmp[128];
		strcpy(tmp,"models/");
		strcat(tmp,fname);
		tiki = TIKI_RegisterModel(tmp);
		if(!tiki)
			return;
	}
	if(Cmd_Argc() > 2) {
		filter = atoi(Cmd_Argv(2));
		if(filter < 0 || filter > 6) {
			filter = -1;
		}
	}
	for(i = 0; i < tiki->numBones; i++) {
		// *((int*)tiki->bones[i]) is a boneType
		if(filter != -1 && *((int*)tiki->bones[i]) != filter)
			continue;
		Com_Printf("Bone %i of %i ",i,tiki->numBones);
		TIKI_PrintBoneInfo(tiki,i);
	}
}

#endif

//===========================================================

/*
==================
SV_AddOperatorCommands
==================
*/
void SV_AddOperatorCommands( void ) {
	static qboolean	initialized;

	if( initialized ) {
		return;
	}
	initialized = qtrue;

	Cmd_AddCommand( "heartbeat", SV_Heartbeat_f );
	Cmd_AddCommand( "kick", SV_Kick_f );
	Cmd_AddCommand( "clientkick", SV_KickNum_f );
	Cmd_AddCommand( "banUser", SV_Ban_f );
	Cmd_AddCommand( "banClient", SV_BanNum_f );
	Cmd_AddCommand( "status", SV_Status_f );
	Cmd_AddCommand( "serverinfo", SV_Serverinfo_f );
	Cmd_AddCommand( "systeminfo", SV_Systeminfo_f );
	Cmd_AddCommand( "dumpuser", SV_DumpUser_f );
	Cmd_AddCommand( "restart", SV_MapRestart_f );
	Cmd_AddCommand( "sectorlist", SV_SectorList_f );
	Cmd_AddCommand( "spmap", SV_Map_f );
	Cmd_AddCommand( "spdevmap", SV_Map_f );
	Cmd_AddCommand( "map", SV_Map_f );
	Cmd_AddCommand( "devmap", SV_Map_f );
	Cmd_AddCommand( "gamemap", SV_GameMap_f );

	Cmd_AddCommand( "killserver", SV_KillServer_f );
	Cmd_AddCommand( "savegame", SV_Savegame_f );
	Cmd_AddCommand( "autosavegame", SV_Autosavegame_f );
	Cmd_AddCommand( "loadgame", SV_Loadgame_f );
	Cmd_AddCommand( "loadlastgame", SV_LoadLastGame_f );
	Cmd_AddCommand( "difficultyEasy", SV_EasyMode_f );
	Cmd_AddCommand( "difficultyMedium", SV_MediumMode_f );
	Cmd_AddCommand( "difficultyHard", SV_HardMode_f );

	if( com_dedicated->integer ) {
		Cmd_AddCommand( "say", SV_ConSay_f );
	}

	if( developer->integer ) {
		SV_MediumMode_f();
	} else {
		SV_EasyMode_f();
	}
}

/*
==================
SV_RemoveOperatorCommands
==================
*/
void SV_RemoveOperatorCommands( void ) {
#if 0
	// removing these won't let the server start again
	Cmd_RemoveCommand ("heartbeat");
	Cmd_RemoveCommand ("kick");
	Cmd_RemoveCommand ("banUser");
	Cmd_RemoveCommand ("banClient");
	Cmd_RemoveCommand ("status");
	Cmd_RemoveCommand ("serverinfo");
	Cmd_RemoveCommand ("systeminfo");
	Cmd_RemoveCommand ("dumpuser");
	Cmd_RemoveCommand ("map_restart");
	Cmd_RemoveCommand ("sectorlist");
	Cmd_RemoveCommand ("say");
#endif
}

/*
==================
SV_ArchiveHudDrawElements
==================
*/
void SV_ArchiveHudDrawElements( qboolean loading )
{
	int i;

	for( i = 0; i < MAX_HUDDRAW_ELEMENTS; i++ )
	{
		ge->ArchiveString( cls.HudDrawElements[ i ].shaderName );
		ge->ArchiveInteger( &cls.HudDrawElements[ i ].iX );
		ge->ArchiveInteger( &cls.HudDrawElements[ i ].iY );
		ge->ArchiveInteger( &cls.HudDrawElements[ i ].iWidth );
		ge->ArchiveInteger( &cls.HudDrawElements[ i ].iVerticalAlign );
		ge->ArchiveFloat( &cls.HudDrawElements[ i ].vColor[ 0 ] );
		ge->ArchiveFloat( &cls.HudDrawElements[ i ].vColor[ 1 ] );
		ge->ArchiveFloat( &cls.HudDrawElements[ i ].vColor[ 2 ] );
		ge->ArchiveFloat( &cls.HudDrawElements[ i ].vColor[ 3 ] );
		ge->ArchiveInteger( &cls.HudDrawElements[ i ].iHorizontalAlign );
		ge->ArchiveInteger( &cls.HudDrawElements[ i ].iVerticalAlign );
		ge->ArchiveInteger( &cls.HudDrawElements[ i ].bVirtualScreen );
		ge->ArchiveString( cls.HudDrawElements[ i ].fontName );
		ge->ArchiveString( cls.HudDrawElements[ i ].string );
	}

	if( loading ) {
		if( cge ) {
			cge->CG_RefreshHudDrawElements();
		}
	}
}

/*
==================
SV_HudDrawShader
==================
*/
void SV_HudDrawShader( int iInfo, char *name )
{
	strcpy( cls.HudDrawElements[ iInfo ].shaderName, name );
	cls.HudDrawElements[ iInfo ].string[ 0 ] = 0;
	cls.HudDrawElements[ iInfo ].pFont = NULL;
	cls.HudDrawElements[ iInfo ].fontName[ 0 ] = 0;

	if( cge ) {
		cge->CG_HudDrawShader( iInfo );
	}
}

/*
==================
SV_HudDrawAlign
==================
*/
void SV_HudDrawAlign( int iInfo, int iHorizontalAlign, int iVerticalAlign )
{
	cls.HudDrawElements[ iInfo ].iHorizontalAlign = iHorizontalAlign;
	cls.HudDrawElements[ iInfo ].iVerticalAlign = iVerticalAlign;
}

/*
==================
SV_HudDrawRect
==================
*/
void SV_HudDrawRect( int iInfo, int iX, int iY, int iWidth, int iHeight )
{
	cls.HudDrawElements[ iInfo ].iX = iX;
	cls.HudDrawElements[ iInfo ].iY = iY;
	cls.HudDrawElements[ iInfo ].iWidth = iWidth;
	cls.HudDrawElements[ iInfo ].iHeight = iHeight;
}

/*
==================
SV_HudDrawVirtualSize
==================
*/
void SV_HudDrawVirtualSize( int iInfo, qboolean bVirtualScreen )
{
	cls.HudDrawElements[ iInfo ].bVirtualScreen = bVirtualScreen;
}

/*
==================
SV_HudDrawColor
==================
*/
void SV_HudDrawColor( int iInfo, vec3_t vColor )
{
	VectorCopy( vColor, cls.HudDrawElements[ iInfo ].vColor );
}

/*
==================
SV_HudDrawAlpha
==================
*/
void SV_HudDrawAlpha( int iInfo, float alpha )
{
	cls.HudDrawElements[ iInfo ].vColor[ 3 ] = alpha;
}

/*
==================
SV_HudDrawString
==================
*/
void SV_HudDrawString( int iInfo, const char *string )
{
	cls.HudDrawElements[ iInfo ].hShader = 0;
	strcpy( cls.HudDrawElements[ iInfo ].string, string );
}

/*
==================
SV_HudDrawFont
==================
*/
void SV_HudDrawFont( int iInfo, const char *name )
{
	strcpy( cls.HudDrawElements[ iInfo ].fontName, name );
	cls.HudDrawElements[ iInfo ].hShader = 0;
	cls.HudDrawElements[ iInfo ].shaderName[ 0 ] = 0;

	if( cge ) {
		cge->CG_HudDrawFont( iInfo );
	}
}

/*
==================
SV_ArchiveViewModelAnimation
==================
*/
void SV_ArchiveViewModelAnimation( qboolean loading )
{
#ifdef CLIENT
	int i;

	for( i = 0; i < MAX_FRAMEINFOS; i++ )
	{
		ge->ArchiveInteger( &cls.anim.vmFrameInfo[ i ].index );
		ge->ArchiveFloat( &cls.anim.vmFrameInfo[ i ].time );
		ge->ArchiveFloat( &cls.anim.vmFrameInfo[ i ].weight );
	}

	ge->ArchiveInteger( &cls.anim.lastVMAnim );
	ge->ArchiveInteger( &cls.anim.lastVMAnimChanged );
	ge->ArchiveInteger( &cls.anim.currentVMAnimSlot );
	ge->ArchiveInteger( &cls.anim.currentVMDuration );
	ge->ArchiveInteger( &cls.anim.crossBlending );
	ge->ArchiveInteger( &cls.anim.lastEquippedWeaponStat );
	ge->ArchiveString( cls.anim.lastActiveItem );
	ge->ArchiveInteger( &cls.anim.lastAnimPrefixIndex );
	ge->ArchiveFloat( &cls.anim.currentVMPosOffset[ 0 ] );
	ge->ArchiveFloat( &cls.anim.currentVMPosOffset[ 1 ] );
	ge->ArchiveFloat( &cls.anim.currentVMPosOffset[ 2 ] );
#endif
}

/*
==================
SV_ArchiveStopwatch
==================
*/
void SV_ArchiveStopwatch( qboolean loading )
{
#ifdef CLIENT
	ge->ArchiveSvsTime( &cls.stopwatch.iStartTime );
	ge->ArchiveSvsTime( &cls.stopwatch.iEndTime );
#endif
}

/*
==================
SV_ArchivePersistantFile
==================
*/
void SV_ArchivePersistantFile( qboolean loading )
{
	const char *name;

	Com_DPrintf( "SV_ArchivePersistantFile()\n" );
	name = Com_GetArchiveFileName( svs.gameName, "spv" );
	ge->ArchivePersistant( name, loading );
}

/*
==================
SV_ArchiveLevel
==================
*/
void SV_ArchiveLevel( qboolean loading )
{
	SV_ArchiveHudDrawElements( loading );
	SV_ArchiveViewModelAnimation( loading );
	SV_ArchiveStopwatch( loading );
}

/*
==================
SV_ArchiveLevelFile
==================
*/
qboolean SV_ArchiveLevelFile( qboolean loading, qboolean autosave )
{
	const char *name;
	qboolean success;
	fileHandle_t f;
	savegamestruct_t save;
#ifdef CLIENT
	soundsystemsavegame_t SSsave;
#endif

	Com_DPrintf( "SV_ArchiveLevelFile()\n" );
	name = Com_GetArchiveFileName( svs.gameName, "sav" );
	if( loading )
	{
		success = ge->ReadLevel( name );
		if( success )
		{
			name = Com_GetArchiveFileName( svs.gameName, "ssv" );
			FS_FOpenFileRead( name, &f, qfalse, qtrue );
			if( f )
			{
				FS_Read( &save, sizeof( savegamestruct_t ), f );
				if( save.version != 3 )
				{
					FS_FCloseFile( f );
					return 0;
				}

#ifdef CLIENT
				FS_Read( &SSsave, sizeof( soundsystemsavegame_t ), f );
#endif
				CM_ReadPortalState( f );
				FS_FCloseFile( f );
			}
		}
	}
	else
	{
		ge->WriteLevel( name, autosave );
		success = qtrue;
	}

	return success;
}

/*
==================
S_Save
==================
*/
void S_Save( fileHandle_t f )
{
#ifdef CLIENT
	soundsystemsavegame_t save;
	S_SaveData( &save );
	FS_Write( &save, sizeof( soundsystemsavegame_t ), f );
#endif
}

/*
==================
S_Load
==================
*/
void S_Load( fileHandle_t f )
{
	FS_Read( &svs, sizeof( serverStatic_t ), f );
#ifdef CLIENT
	// FIXME...
	//S_InitBase( &svs.soundSystem );
#endif
}

/*
==================
SV_ArchiveServerFile
==================
*/
qboolean SV_ArchiveServerFile( qboolean loading, qboolean autosave )
{
	fileHandle_t f;
	const char *name;
	char comment[ 64 ];
	savegamestruct_t save;
	char cmdString[ 256 ];
	time_t aclock;

	Com_DPrintf( "SV_ArchiveServerFile(%s)\n", autosave ? "true" : "false" );
	name = Com_GetArchiveFileName( svs.gameName, "ssv" );

	if( !loading )
	{
		f = FS_FOpenFileWrite( name );
		if( !f )
		{
			Com_Printf( "Couldn't write %s\n", name );
			return qfalse;
		}

		time( &aclock );

		if( autosave )
		{
			Com_sprintf( comment, sizeof( comment ), "%s - Starting", sv.configstrings[ CS_MESSAGE ] );
		}
		else if( sv.configstrings[ CS_SAVENAME ] && *sv.configstrings[ CS_SAVENAME ] )
		{
			Com_sprintf( comment, sizeof( comment ), "%s - %s", sv.configstrings[ CS_MESSAGE ], sv.configstrings[ CS_SAVENAME ] );
		}
		else
		{
			Com_sprintf( comment, sizeof( comment ), "%s", sv.configstrings[ CS_MESSAGE ] );
		}

		if( strstr( name, "quick.ssv" ) ) {
			Com_sprintf( save.comment, sizeof( save.comment ), "QuickSave - %s", comment );
		} else {
			strcpy( save.comment, name );
		}

		SV_SetConfigstring( CS_SAVENAME, "" );

		save.version = 3;
		save.time = aclock;
		strncpy( save.mapName, svs.mapName, sizeof( save.mapName ) );
		strncpy( save.saveName, svs.gameName, sizeof( save.saveName ) );
		save.mapTime = svs.time - svs.startTime;

		FS_Write( &save, sizeof( savegamestruct_t ), f );
		S_Save( f );
		CM_WritePortalState( f );
		FS_FCloseFile( f );

		name = Com_GetArchiveFileName( svs.gameName, "tga" );
		Com_sprintf( cmdString, sizeof( cmdString ), "saveshot %s 256 256\n", name );
		Cbuf_ExecuteText( svs.autosave != qfalse, name );
	}
	else
	{
		FS_FOpenFileRead( name, &f, qfalse, qtrue );
		if( !f )
		{
			Com_Printf( "Couldn't read %s\n", name );
			return qfalse;
		}

		FS_Read( &save, sizeof( savegamestruct_t ), f );
		if( save.version != 3 )
		{
			Com_Printf( "Invalid or Old Server SaveGame Version\n", name );
			return qfalse;
		}

#ifdef CLIENT
		S_StopAllSounds( qtrue );
#endif
		S_Load( f );
		strncpy( svs.mapName, save.mapName, sizeof( svs.mapName ) );
		svs.mapTime = save.mapTime;
		svs.areabits_warning_time = 0;
		strcpy( svs.tm_filename, save.tm_filename );
		svs.tm_loopcount = save.tm_loopcount;
		svs.tm_offset = save.tm_offset;
		FS_FCloseFile( f );
	}

	return qtrue;
}

/*
==================
SV_Loadgame_f
==================
*/
void SV_Loadgame_f( void )
{
	int length;
	const char *name;
	const char *archive_name;
	qboolean bStartedGame;

	if( com_cl_running && com_cl_running->integer &&
#ifdef CLIENT
		cls.state != CA_DISCONNECTED && cg_gametype->integer ||
#endif
		com_sv_running && com_sv_running->integer && g_gametype->integer )
	{
		Com_Printf( "Can't loadgame in a multiplayer game\n" );
		return;
	}

	Cvar_Set( "g_gametype", "0" );
	Cvar_Set( "sv_maxclients", "1" );
	Cvar_Get( "g_gametype", "0", CVAR_LATCH | CVAR_SERVERINFO );
	Cvar_Get( "sv_maxclients", "0", CVAR_LATCH | CVAR_SERVERINFO );

	if( developer->integer )
	{
		if( svs.iNumClients == 1 )
		{
			Cvar_Set( "cheats", "1" );
		}
		else
		{
			Cvar_Set( "cheats", "0" );
		}
	}

	if( sv.state == SS_LOADING || sv.state == SS_LOADING2 )
	{
		Com_Printf( "Can't load game when loading\n" );
		return;
	}

	if( Cmd_Argc() != 2 )
	{
		Com_Printf( "USAGE: loadgame <name>\n" );
		return;
	}

	name = Cmd_Argv( 1 );
	Com_Printf( "Loading game...%s\n", name );
	if( strstr( name, ".." ) || strchr( name, '/' ) || strchr( name, '\\' ) ) {
		Com_Printf( "Bad savedir.\n" );
	}

	strcpy( svs.gameName, name );
	archive_name = Com_GetArchiveFileName( name, "sav" );
	Cvar_SaveGameRestart_f();

	length = FS_ReadFileEx( archive_name, NULL, qtrue );
	if( length == -1 )
	{
		Com_Printf( "Savegame not found.\n" );
		return;
	}

	if( !ge )
	{
		SV_InitGameProgs();
		bStartedGame = qtrue;
	}

	if( ge->LevelArchiveValid( archive_name ) && SV_ArchiveServerFile( qtrue, qfalse ) )
	{
		SV_SpawnServer( svs.mapName, qtrue, qfalse, qfalse );
		svs.soundsNeedLoad = qtrue;
	}
	else if( bStartedGame )
	{
		SV_ShutdownGameProgs();
	}
}

/*
==================
SV_SavegameFilename
==================
*/
void SV_SavegameFilename( const char *name, char *fileName, int length )
{
	int num;
	const char *fname;
	int lastNumber;
	int len;
	int a;
	int b;
	int c;

	for( num = 0; num < 10000; num++ )
	{
		a = num / 1000;
		b = num % 1000 / 100;
		c = num % 1000 % 100 / 10;
		lastNumber = num % 1000 % 100 % 10;
		Com_sprintf( fileName, length, "%s%i%i%i%i", name, a, b, c, lastNumber );
		fname = Com_GetArchiveFileName( fileName, "ssv" );
		len = FS_ReadFileEx( fname, NULL, qtrue );
		if( len <= 0 ) {
			break;
		}
	}
}

/*
==================
SV_AllowSaveGame
==================
*/
qboolean SV_AllowSaveGame( void )
{
	if( !com_sv_running || !com_sv_running->integer )
	{
		Com_DPrintf( "You must be in a game with a server to save.\n" );
	}
	else if( !com_cl_running || !com_cl_running->integer )
	{
		Com_DPrintf( "You must be in a game with a client to save.\n" );
	}
	else if( sv.state != SS_GAME )
	{
		Com_DPrintf( "You must be in game to save.\n" );
	}
#ifdef CLIENT
	else if( cls.state != CA_DISCONNECTED && cg_gametype->integer )
	{
		Com_DPrintf( "Can't savegame in a multiplayer game\n" );
	}
#endif
	else if( g_gametype->integer )
	{
		Com_DPrintf( "Can't savegame in a multiplayer game\n" );
	}
	else if( !svs.clients || svs.clients->gentity == NULL || svs.clients->gentity->client == NULL || svs.clients->gentity->client->ps.stats[ 0 ] )
	{
		Com_DPrintf( "Can't savegame when dead\n" );
	}
	else if( sv.state == SS_LOADING || sv.state == SS_LOADING2 )
	{
		Com_DPrintf( "Can't save game when loading\n" );
	}

	return qtrue;
}

/*
==================
SV_DoSaveGame
==================
*/
static qboolean bSavegame;

qboolean SV_DoSaveGame()
{
	if( bSavegame )
	{
		if( SV_AllowSaveGame() ) {
			return qtrue;
		}
		bSavegame = qfalse;
	}
	return qfalse;
}

/*
==================
SV_SaveGame
==================
*/
void SV_SaveGame( const char *gamename, qboolean autosave )
{
	char *ptr;
	char name[ 64 ];
	char mname[ 64 ];

	if( !SV_AllowSaveGame() ) {
		return;
	}

	if( gamename )
	{
		strcpy( name, gamename );
	}
	else
	{
		strcpy( mname, svs.mapName );
		for( ptr = strchr( mname, '/' ); ptr != NULL; ptr = strchr( mname, '/' ) )
		{
			*ptr = '_';
		}
		SV_SavegameFilename( mname, name, sizeof( name ) );
	}

	if( strstr( name, ".." ) || strchr( name, '/' ) || strchr( name, '\\' ) )
	{
		Com_DPrintf( "Bad savedir.\n" );
		return;
	}

	Com_Printf( "Saving to %s", name );
	if( autosave ) {
		Com_DPrintf( " (autosave)...\n" );
	} else {
		Com_DPrintf( "...\n" );
	}

	strcpy( svs.gameName, name );
	SV_ArchiveLevelFile( qfalse, autosave );
	SV_ArchiveServerFile( qfalse, autosave );
	Com_Printf( "Done.\n" );
	strcpy( svs.gameName, "current" );

#ifdef CLIENT
	IN_MouseCancel();
#endif
}

/*
==================
SV_Savegame_f
==================
*/
static char savegame_name[ 64 ];

void SV_Savegame_f( void )
{
	char *s;

	if( !SV_AllowSaveGame() ) {
		return;
	}

	if( Cmd_Argc() == 2 )
	{

		s = Cmd_Argv( 1 );
		if( strlen( s ) >= sizeof( savegame_name ) ) {
			return;
		}

		strcpy( savegame_name, s );
	}
	else
	{
		savegame_name[ 0 ] = 0;
	}

	bSavegame = qtrue;
}

/*
==================
SV_CheckSaveGame
==================
*/
void SV_CheckSaveGame( void )
{
	if( !SV_DoSaveGame() ) {
		return;
	}

	if( cl.serverTime >= svs.time )
	{
		bSavegame = qfalse;
		SV_SaveGame( savegame_name[ 0 ] ? savegame_name : NULL, qfalse );
#ifdef CLIENT
		UI_SetupFiles();
#endif
	}
}

void SV_Autosavegame_f( void )
{
	SV_SaveGame( NULL, qtrue );
}
