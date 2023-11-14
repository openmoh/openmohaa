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

#include "server.h"

#ifndef DEDICATED
#  include "../client/client.h"
#  include "../client/snd_local.h"
#  include "../uilib/ui_public.h"
#endif

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

	if( !Q_stricmpn( map, "dm/", 3 ) && g_gametype->integer == GT_OBJECTIVE )
	{
		Com_Printf( "Can't load regular dm map in objective game type\n" );
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

    //
    // in 2.0 and above, ignore training / mohaa mission maps
    //
	// FIXME: is this really necessary?
	/*
	if (com_target_game->integer != TG_MOH && g_gametype->integer == GT_SINGLE_PLAYER) {
		if (mapname[0] == 'm' && mapname[1] - '1' <= 5) {
			return;
		}

		if (!Q_stricmp(mapname, "training")) {
			return;
		} else if (!Q_stricmp(mapname, "training.bsp")) {
			return;
		} else if (!Q_stricmp(mapname, "void")) {
			return;
		} else if (!Q_stricmp(mapname, "void.bsp")) {
			return;
		}
	}
	*/

	// start up the map
	SV_SpawnServer( mapname, qfalse, qfalse, qfalse );

	if( g_gametype->integer == GT_SINGLE_PLAYER ) {
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
SV_KickAll_f

Kick all users off of the server
==================
*/
static void SV_KickAll_f( void ) {
	client_t *cl;
	int i;

	// make sure server is running
	if( !com_sv_running->integer ) {
		Com_Printf( "Server is not running.\n" );
		return;
	}

	for( i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++ ) {
		if( !cl->state ) {
			continue;
		}

		if( cl->netchan.remoteAddress.type == NA_LOOPBACK ) {
			continue;
		}

		SV_DropClient( cl, "was kicked" );
		cl->lastPacketTime = svs.time; // in case there is a funny zombie
	}
}

/*
==================
SV_KickNum_f

Kick a user off of the server
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
		Com_Printf ("Usage: %s <client number>\n", Cmd_Argv(0));
		return;
	}

	cl = SV_GetPlayerByNum();
	if ( !cl ) {
		return;
	}
	if( cl->netchan.remoteAddress.type == NA_LOOPBACK ) {
		Com_Printf("Cannot kick host player\n");
		return;
	}

	SV_DropClient( cl, "was kicked" );
	cl->lastPacketTime = svs.time;	// in case there is a funny zombie
}

#ifndef STANDALONE
// these functions require the auth server which of course is not available anymore for stand-alone games.

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
		Com_Printf("Cannot kick host player\n");
		return;
	}

	// look up the authorize server's IP
	if ( !svs.authorizeAddress.ip[0] && svs.authorizeAddress.type != NA_BAD ) {
		Com_Printf( "Resolving %s\n", AUTHORIZE_SERVER_NAME );
		if ( !NET_StringToAdr( AUTHORIZE_SERVER_NAME, &svs.authorizeAddress, NA_IP ) ) {
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
		Com_Printf("Cannot kick host player\n");
		return;
	}

	// look up the authorize server's IP
	if ( !svs.authorizeAddress.ip[0] && svs.authorizeAddress.type != NA_BAD ) {
		Com_Printf( "Resolving %s\n", AUTHORIZE_SERVER_NAME );
		if ( !NET_StringToAdr( AUTHORIZE_SERVER_NAME, &svs.authorizeAddress, NA_IP ) ) {
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
#endif

/*
==================
SV_RehashBans_f

Load saved bans from file.
==================
*/
static void SV_RehashBans_f(void)
{
	int index, filelen;
	fileHandle_t readfrom;
	char *textbuf, *curpos, *maskpos, *newlinepos, *endpos;
	char filepath[MAX_QPATH];
	
	// make sure server is running
	if ( !com_sv_running->integer ) {
		return;
	}
	
	serverBansCount = 0;
	
	if(!sv_banFile->string || !*sv_banFile->string)
		return;

	Com_sprintf(filepath, sizeof(filepath), "%s/%s", FS_GetCurrentGameDir(), sv_banFile->string);

	if((filelen = FS_SV_FOpenFileRead(filepath, &readfrom)) >= 0)
	{
		if(filelen < 2)
		{
			// Don't bother if file is too short.
			FS_FCloseFile(readfrom);
			return;
		}

		curpos = textbuf = Z_Malloc(filelen);
		
		filelen = FS_Read(textbuf, filelen, readfrom);
		FS_FCloseFile(readfrom);
		
		endpos = textbuf + filelen;
		
		for(index = 0; index < SERVER_MAXBANS && curpos + 2 < endpos; index++)
		{
			// find the end of the address string
			for(maskpos = curpos + 2; maskpos < endpos && *maskpos != ' '; maskpos++);
			
			if(maskpos + 1 >= endpos)
				break;

			*maskpos = '\0';
			maskpos++;
			
			// find the end of the subnet specifier
			for(newlinepos = maskpos; newlinepos < endpos && *newlinepos != '\n'; newlinepos++);
			
			if(newlinepos >= endpos)
				break;
			
			*newlinepos = '\0';
			
			if(NET_StringToAdr(curpos + 2, &serverBans[index].ip, NA_UNSPEC))
			{
				serverBans[index].isexception = (curpos[0] != '0');
				serverBans[index].subnet = atoi(maskpos);
				
				if(serverBans[index].ip.type == NA_IP &&
				   (serverBans[index].subnet < 1 || serverBans[index].subnet > 32))
				{
					serverBans[index].subnet = 32;
				}
				else if(serverBans[index].ip.type == NA_IP6 &&
					(serverBans[index].subnet < 1 || serverBans[index].subnet > 128))
				{
					serverBans[index].subnet = 128;
				}
			}
			
			curpos = newlinepos + 1;
		}
			
		serverBansCount = index;
		
		Z_Free(textbuf);
	}
}

/*
==================
SV_WriteBans

Save bans to file.
==================
*/
static void SV_WriteBans(void)
{
	int index;
	fileHandle_t writeto;
	char filepath[MAX_QPATH];
	
	if(!sv_banFile->string || !*sv_banFile->string)
		return;
	
	Com_sprintf(filepath, sizeof(filepath), "%s/%s", FS_GetCurrentGameDir(), sv_banFile->string);

	if((writeto = FS_SV_FOpenFileWrite(filepath)))
	{
		char writebuf[128];
		serverBan_t *curban;
		
		for(index = 0; index < serverBansCount; index++)
		{
			curban = &serverBans[index];
			
			Com_sprintf(writebuf, sizeof(writebuf), "%d %s %d\n",
				    curban->isexception, NET_AdrToString(curban->ip), curban->subnet);
			FS_Write(writebuf, strlen(writebuf), writeto);
		}

		FS_FCloseFile(writeto);
	}
}

/*
==================
SV_DelBanEntryFromList

Remove a ban or an exception from the list.
==================
*/

static qboolean SV_DelBanEntryFromList(int index)
{
	if(index == serverBansCount - 1)
		serverBansCount--;
	else if(index < ARRAY_LEN(serverBans) - 1)
	{
		memmove(serverBans + index, serverBans + index + 1, (serverBansCount - index - 1) * sizeof(*serverBans));
		serverBansCount--;
	}
	else
		return qtrue;

	return qfalse;
}

/*
==================
SV_ParseCIDRNotation

Parse a CIDR notation type string and return a netadr_t and suffix by reference
==================
*/

static qboolean SV_ParseCIDRNotation(netadr_t *dest, int *mask, char *adrstr)
{
	char *suffix;
	
	suffix = strchr(adrstr, '/');
	if(suffix)
	{
		*suffix = '\0';
		suffix++;
	}

	if(!NET_StringToAdr(adrstr, dest, NA_UNSPEC))
		return qtrue;

	if(suffix)
	{
		*mask = atoi(suffix);
		
		if(dest->type == NA_IP)
		{
			if(*mask < 1 || *mask > 32)
				*mask = 32;
		}
		else
		{
			if(*mask < 1 || *mask > 128)
				*mask = 128;
		}
	}
	else if(dest->type == NA_IP)
		*mask = 32;
	else
		*mask = 128;
	
	return qfalse;
}

/*
==================
SV_AddBanToList

Ban a user from being able to play on this server based on his ip address.
==================
*/

static void SV_AddBanToList(qboolean isexception)
{
	char *banstring;
	char addy2[NET_ADDRSTRMAXLEN];
	netadr_t ip;
	int index, argc, mask;
	serverBan_t *curban;

	// make sure server is running
	if ( !com_sv_running->integer ) {
		Com_Printf( "Server is not running.\n" );
		return;
	}

	argc = Cmd_Argc();
	
	if(argc < 2 || argc > 3)
	{
		Com_Printf ("Usage: %s (ip[/subnet] | clientnum [subnet])\n", Cmd_Argv(0));
		return;
	}

	if(serverBansCount >= ARRAY_LEN(serverBans))
	{
		Com_Printf ("Error: Maximum number of bans/exceptions exceeded.\n");
		return;
	}

	banstring = Cmd_Argv(1);
	
	if(strchr(banstring, '.') || strchr(banstring, ':'))
	{
		// This is an ip address, not a client num.
		
		if(SV_ParseCIDRNotation(&ip, &mask, banstring))
		{
			Com_Printf("Error: Invalid address %s\n", banstring);
			return;
		}
	}
	else
	{
		client_t *cl;
		
		// client num.
		
		cl = SV_GetPlayerByNum();

		if(!cl)
		{
			Com_Printf("Error: Playernum %s does not exist.\n", Cmd_Argv(1));
			return;
		}
		
		ip = cl->netchan.remoteAddress;
		
		if(argc == 3)
		{
			mask = atoi(Cmd_Argv(2));
			
			if(ip.type == NA_IP)
			{
				if(mask < 1 || mask > 32)
					mask = 32;
			}
			else
			{
				if(mask < 1 || mask > 128)
					mask = 128;
			}
		}
		else
			mask = (ip.type == NA_IP6) ? 128 : 32;
	}

	if(ip.type != NA_IP && ip.type != NA_IP6)
	{
		Com_Printf("Error: Can ban players connected via the internet only.\n");
		return;
	}

	// first check whether a conflicting ban exists that would supersede the new one.
	for(index = 0; index < serverBansCount; index++)
	{
		curban = &serverBans[index];
		
		if(curban->subnet <= mask)
		{
			if((curban->isexception || !isexception) && NET_CompareBaseAdrMask(curban->ip, ip, curban->subnet))
			{
				Q_strncpyz(addy2, NET_AdrToString(ip), sizeof(addy2));
				
				Com_Printf("Error: %s %s/%d supersedes %s %s/%d\n", curban->isexception ? "Exception" : "Ban",
					   NET_AdrToString(curban->ip), curban->subnet,
					   isexception ? "exception" : "ban", addy2, mask);
				return;
			}
		}
		if(curban->subnet >= mask)
		{
			if(!curban->isexception && isexception && NET_CompareBaseAdrMask(curban->ip, ip, mask))
			{
				Q_strncpyz(addy2, NET_AdrToString(curban->ip), sizeof(addy2));
			
				Com_Printf("Error: %s %s/%d supersedes already existing %s %s/%d\n", isexception ? "Exception" : "Ban",
					   NET_AdrToString(ip), mask,
					   curban->isexception ? "exception" : "ban", addy2, curban->subnet);
				return;
			}
		}
	}

	// now delete bans that are superseded by the new one
	index = 0;
	while(index < serverBansCount)
	{
		curban = &serverBans[index];
		
		if(curban->subnet > mask && (!curban->isexception || isexception) && NET_CompareBaseAdrMask(curban->ip, ip, mask))
			SV_DelBanEntryFromList(index);
		else
			index++;
	}

	serverBans[serverBansCount].ip = ip;
	serverBans[serverBansCount].subnet = mask;
	serverBans[serverBansCount].isexception = isexception;
	
	serverBansCount++;
	
	SV_WriteBans();

	Com_Printf("Added %s: %s/%d\n", isexception ? "ban exception" : "ban",
		   NET_AdrToString(ip), mask);
}

/*
==================
SV_DelBanFromList

Remove a ban or an exception from the list.
==================
*/

static void SV_DelBanFromList(qboolean isexception)
{
	int index, count = 0, todel, mask;
	netadr_t ip;
	char *banstring;
	
	// make sure server is running
	if ( !com_sv_running->integer ) {
		Com_Printf( "Server is not running.\n" );
		return;
	}
	
	if(Cmd_Argc() != 2)
	{
		Com_Printf ("Usage: %s (ip[/subnet] | num)\n", Cmd_Argv(0));
		return;
	}

	banstring = Cmd_Argv(1);
	
	if(strchr(banstring, '.') || strchr(banstring, ':'))
	{
		serverBan_t *curban;
		
		if(SV_ParseCIDRNotation(&ip, &mask, banstring))
		{
			Com_Printf("Error: Invalid address %s\n", banstring);
			return;
		}
		
		index = 0;
		
		while(index < serverBansCount)
		{
			curban = &serverBans[index];
			
			if(curban->isexception == isexception		&&
			   curban->subnet >= mask 			&&
			   NET_CompareBaseAdrMask(curban->ip, ip, mask))
			{
				Com_Printf("Deleting %s %s/%d\n",
					   isexception ? "exception" : "ban",
					   NET_AdrToString(curban->ip), curban->subnet);
					   
				SV_DelBanEntryFromList(index);
			}
			else
				index++;
		}
	}
	else
	{
		todel = atoi(Cmd_Argv(1));

		if(todel < 1 || todel > serverBansCount)
		{
			Com_Printf("Error: Invalid ban number given\n");
			return;
		}
	
		for(index = 0; index < serverBansCount; index++)
		{
			if(serverBans[index].isexception == isexception)
			{
				count++;
			
				if(count == todel)
				{
					Com_Printf("Deleting %s %s/%d\n",
					   isexception ? "exception" : "ban",
					   NET_AdrToString(serverBans[index].ip), serverBans[index].subnet);

					SV_DelBanEntryFromList(index);

					break;
				}
			}
		}
	}
	
	SV_WriteBans();
}


/*
==================
SV_ListBans_f

List all bans and exceptions on console
==================
*/

static void SV_ListBans_f(void)
{
	int index, count;
	serverBan_t *ban;

	// make sure server is running
	if ( !com_sv_running->integer ) {
		Com_Printf( "Server is not running.\n" );
		return;
	}
	
	// List all bans
	for(index = count = 0; index < serverBansCount; index++)
	{
		ban = &serverBans[index];
		if(!ban->isexception)
		{
			count++;

			Com_Printf("Ban #%d: %s/%d\n", count,
				    NET_AdrToString(ban->ip), ban->subnet);
		}
	}
	// List all exceptions
	for(index = count = 0; index < serverBansCount; index++)
	{
		ban = &serverBans[index];
		if(ban->isexception)
		{
			count++;

			Com_Printf("Except #%d: %s/%d\n", count,
				    NET_AdrToString(ban->ip), ban->subnet);
		}
	}
}

/*
==================
SV_FlushBans_f

Delete all bans and exceptions.
==================
*/

static void SV_FlushBans_f(void)
{
	// make sure server is running
	if ( !com_sv_running->integer ) {
		Com_Printf( "Server is not running.\n" );
		return;
	}

	serverBansCount = 0;
	
	// empty the ban file.
	SV_WriteBans();
	
	Com_Printf("All bans and exceptions have been deleted.\n");
}

static void SV_BanAddr_f(void)
{
	SV_AddBanToList(qfalse);
}

static void SV_ExceptAddr_f(void)
{
	SV_AddBanToList(qtrue);
}

static void SV_BanDel_f(void)
{
	SV_DelBanFromList(qfalse);
}

static void SV_ExceptDel_f(void)
{
	SV_DelBanFromList(qtrue);
}

/*
** SV_Strlen -- skips color escape codes
*/
static int SV_Strlen( const char *str ) {
	const char *s = str;
	int count = 0;

	while ( *s ) {
		if ( Q_IsColorString( s ) ) {
			s += 2;
		} else {
			count++;
			s++;
		}
	}

	return count;
}

/*
================
SV_Status_f
================
*/
static void SV_Status_f(void) {
    int			i;
    size_t		j, l;
    client_t* cl;
    playerState_t* ps;
    const char* s;
    int			ping;

    // make sure server is running
    if (!com_sv_running->integer) {
        Com_Printf("Server is not running.\n");
        return;
    }

    Com_Printf("map: %s\n", sv_mapname->string);

    Com_Printf("num score ping name            lastmsg address                                  qport rate \n");
    Com_Printf("--- ----- ---- --------------- ------- ---------------------------------------  ----- -----\n");
    for (i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++)
    {
        if (!cl->state)
            continue;
        Com_Printf("%3i ", i);
        ps = SV_GameClientNum(i);
        // su44: ps->persistant is not avaible in mohaa
        //Com_Printf ("%5i ", ps->persistant[PERS_SCORE]);
        Com_Printf("%5i ", ps->stats[STAT_KILLS]);

        if (cl->state == CS_CONNECTED)
            Com_Printf("CNCT ");
        else if (cl->state == CS_ZOMBIE)
            Com_Printf("ZMBI ");
        else
        {
            ping = cl->ping < 9999 ? cl->ping : 9999;
            Com_Printf("%4i ", ping);
        }

        Com_Printf("%s", cl->name);
        // TTimo adding a ^7 to reset the color
        // NOTE: colored names in status breaks the padding (WONTFIX)
        Com_Printf("^7");
        l = SV_Strlen(cl->name);
		if (l <= 16) {
			l = 16 - l;
			for (j = 0; j < l; j++)
				Com_Printf(" ");
		}

        Com_Printf("%7i ", svs.time - cl->lastPacketTime);

        s = NET_AdrToString(cl->netchan.remoteAddress);
        Com_Printf("%s", s);
        l = 39 - strlen(s);
        for (j = 0; j < l; j++)
            Com_Printf(" ");

        Com_Printf("%5i", cl->netchan.qport);

        Com_Printf(" %5i", cl->rate);

        Com_Printf("\n");
    }
    Com_Printf("\n");
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

	Com_Printf("%s\n", text);
	SV_SendServerCommand(NULL, "print \"" HUD_MESSAGE_CHAT_WHITE "%s\n\"", text);
}

/*
==================
SV_ConTell_f
==================
*/
static void SV_ConTell_f(void) {
	char	*p;
	char	text[1024];
	client_t	*cl;

	// make sure server is running
	if ( !com_sv_running->integer ) {
		Com_Printf( "Server is not running.\n" );
		return;
	}

	if ( Cmd_Argc() < 3 ) {
		Com_Printf ("Usage: tell <client number> <text>\n");
		return;
	}

	cl = SV_GetPlayerByNum();
	if ( !cl ) {
		return;
	}

	strcpy (text, "(private) console: ");
	p = Cmd_ArgsFrom(2);

	if ( *p == '"' ) {
		p++;
		p[strlen(p)-1] = 0;
	}

	strcat(text, p);

	Com_Printf("%s\n", text);
	SV_SendServerCommand(cl, "print \"" HUD_MESSAGE_CHAT_WHITE "%s\n\"", text);
}


/*
==================
SV_ConSayto_f
==================
*/
static void SV_ConSayto_f(void) {
	char		*p;
	char		text[1024];
	client_t	*cl;
	char		*rawname;
	char		name[MAX_NAME_LENGTH];
	char		cleanName[MAX_NAME_LENGTH];
	client_t	*saytocl;
	int			i;

	// make sure server is running
	if ( !com_sv_running->integer ) {
		Com_Printf( "Server is not running.\n" );
		return;
	}

	if ( Cmd_Argc() < 3 ) {
		Com_Printf ("Usage: sayto <player name> <text>\n");
		return;
	}

	rawname = Cmd_Argv(1);
	
	//allowing special characters in the console 
	//with hex strings for player names
	Com_FieldStringToPlayerName( name, MAX_NAME_LENGTH, rawname );

	saytocl = NULL;
	for ( i=0, cl=svs.clients ; i < sv_maxclients->integer ; i++,cl++ ) {
		if ( !cl->state ) {
			continue;
		}
		Q_strncpyz( cleanName, cl->name, sizeof(cleanName) );
		Q_CleanStr( cleanName );

		if ( !Q_stricmp( cleanName, name ) ) {
			saytocl = cl;
			break;
		}
	}
	if( !saytocl )
	{
		Com_Printf ("No such player name: %s.\n", name);
		return;
	}

	strcpy (text, "(private) console: ");
	p = Cmd_ArgsFrom(2);

	if ( *p == '"' ) {
		p++;
		p[strlen(p)-1] = 0;
	}

	strcat(text, p);

    Com_Printf("%s\n", text);
    SV_SendServerCommand(saytocl, "print \"" HUD_MESSAGE_CHAT_WHITE "%s\n\"", text);
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



//===========================================================

/*
==================
SV_CompleteMapName
==================
*/
static void SV_CompleteMapName( char *args, int argNum ) {
	if( argNum == 2 ) {
		Field_CompleteFilename( "maps", "bsp", qtrue, qfalse );
	}
}

/*
==================
SV_CompletePlayerName
==================
*/
static void SV_CompletePlayerName( char *args, int argNum ) {
	if( argNum == 2 ) {
		char		names[MAX_CLIENTS][MAX_NAME_LENGTH];
		const char	*namesPtr[MAX_CLIENTS];
		client_t	*cl;
		int			i;
		int			nameCount;
		int			clientCount;

		nameCount = 0;
		clientCount = sv_maxclients->integer;

		for ( i=0, cl=svs.clients ; i < clientCount; i++,cl++ ) {
			if ( !cl->state ) {
				continue;
			}
			if( i >= MAX_CLIENTS ) {
				break;
			}
			Q_strncpyz( names[nameCount], cl->name, sizeof(names[nameCount]) );
			Q_CleanStr( names[nameCount] );

			namesPtr[nameCount] = names[nameCount];

			nameCount++;
		}
		qsort( (void*)namesPtr, nameCount, sizeof( namesPtr[0] ), Com_strCompare );

		Field_CompletePlayerName( namesPtr, nameCount );
	}
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
void SV_AddOperatorCommands(void) {
    static qboolean	initialized;

    if (initialized) {
        return;
    }
    initialized = qtrue;

    Cmd_AddCommand("heartbeat", SV_Heartbeat_f);
    Cmd_AddCommand("kick", SV_Kick_f);
#ifndef STANDALONE
    if (!com_standalone->integer)
    {
        Cmd_AddCommand("banUser", SV_Ban_f);
        Cmd_AddCommand("banClient", SV_BanNum_f);
    }
#endif
    Cmd_AddCommand("kickall", SV_KickAll_f);
    Cmd_AddCommand("kicknum", SV_KickNum_f);
    Cmd_AddCommand("clientkick", SV_KickNum_f); // Legacy command
    Cmd_AddCommand("status", SV_Status_f);
    Cmd_AddCommand("serverinfo", SV_Serverinfo_f);
    Cmd_AddCommand("systeminfo", SV_Systeminfo_f);
    Cmd_AddCommand("dumpuser", SV_DumpUser_f);
    Cmd_AddCommand("restart", SV_MapRestart_f);
    Cmd_AddCommand("sectorlist", SV_SectorList_f);
    Cmd_AddCommand("spmap", SV_Map_f);
    Cmd_AddCommand("spdevmap", SV_Map_f);
    Cmd_AddCommand("map", SV_Map_f);
    Cmd_AddCommand("devmap", SV_Map_f);
    Cmd_AddCommand("gamemap", SV_GameMap_f);

    Cmd_AddCommand("killserver", SV_KillServer_f);
#ifndef DEDICATED
    Cmd_AddCommand("savegame", SV_Savegame_f);
    Cmd_AddCommand("autosavegame", SV_Autosavegame_f);
    Cmd_AddCommand("loadgame", SV_Loadgame_f);
    Cmd_AddCommand("loadlastgame", SV_LoadLastGame_f);
#endif

    if (com_dedicated->integer) {
		Cmd_AddCommand("say", SV_ConSay_f);
		Cmd_AddCommand("tell", SV_ConTell_f);
		Cmd_AddCommand("sayto", SV_ConSayto_f);
		Cmd_SetCommandCompletionFunc("sayto", SV_CompletePlayerName);
    }

	Cmd_AddCommand("rehashbans", SV_RehashBans_f);
	Cmd_AddCommand("listbans", SV_ListBans_f);
	Cmd_AddCommand("banaddr", SV_BanAddr_f);
	Cmd_AddCommand("exceptaddr", SV_ExceptAddr_f);
	Cmd_AddCommand("bandel", SV_BanDel_f);
	Cmd_AddCommand("exceptdel", SV_ExceptDel_f);
	Cmd_AddCommand("flushbans", SV_FlushBans_f);
	
    Cmd_AddCommand("difficultyEasy", SV_EasyMode_f);
    Cmd_AddCommand("difficultyMedium", SV_MediumMode_f);
    Cmd_AddCommand("difficultyHard", SV_HardMode_f);

    if (developer->integer) {
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
#ifndef DEDICATED
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
#endif
}

/*
==================
SV_HudDrawShader
==================
*/
void SV_HudDrawShader( int iInfo, char *name )
{
#ifndef DEDICATED
	strcpy( cls.HudDrawElements[ iInfo ].shaderName, name );
	cls.HudDrawElements[ iInfo ].string[ 0 ] = 0;
	cls.HudDrawElements[ iInfo ].pFont = NULL;
	cls.HudDrawElements[ iInfo ].fontName[ 0 ] = 0;

	if( cge ) {
		cge->CG_HudDrawShader( iInfo );
	}
#endif
}

/*
==================
SV_HudDrawAlign
==================
*/
void SV_HudDrawAlign( int iInfo, int iHorizontalAlign, int iVerticalAlign )
{
#ifndef DEDICATED
	cls.HudDrawElements[ iInfo ].iHorizontalAlign = iHorizontalAlign;
	cls.HudDrawElements[ iInfo ].iVerticalAlign = iVerticalAlign;
#endif
}

/*
==================
SV_HudDrawRect
==================
*/
void SV_HudDrawRect( int iInfo, int iX, int iY, int iWidth, int iHeight )
{
#ifndef DEDICATED
	cls.HudDrawElements[ iInfo ].iX = iX;
	cls.HudDrawElements[ iInfo ].iY = iY;
	cls.HudDrawElements[ iInfo ].iWidth = iWidth;
	cls.HudDrawElements[ iInfo ].iHeight = iHeight;
#endif
}

/*
==================
SV_HudDrawVirtualSize
==================
*/
void SV_HudDrawVirtualSize( int iInfo, qboolean bVirtualScreen )
{
#ifndef DEDICATED
	cls.HudDrawElements[ iInfo ].bVirtualScreen = bVirtualScreen;
#endif
}

/*
==================
SV_HudDrawColor
==================
*/
void SV_HudDrawColor( int iInfo, vec3_t vColor )
{
#ifndef DEDICATED
	VectorCopy( vColor, cls.HudDrawElements[ iInfo ].vColor );
#endif
}

/*
==================
SV_HudDrawAlpha
==================
*/
void SV_HudDrawAlpha( int iInfo, float alpha )
{
#ifndef DEDICATED
	cls.HudDrawElements[ iInfo ].vColor[ 3 ] = alpha;
#endif
}

/*
==================
SV_HudDrawString
==================
*/
void SV_HudDrawString( int iInfo, const char *string )
{
#ifndef DEDICATED
	cls.HudDrawElements[ iInfo ].hShader = 0;
	strcpy( cls.HudDrawElements[ iInfo ].string, string );
#endif
}

/*
==================
SV_HudDrawFont
==================
*/
void SV_HudDrawFont( int iInfo, const char *name )
{
#ifndef DEDICATED
	strcpy( cls.HudDrawElements[ iInfo ].fontName, name );
	cls.HudDrawElements[ iInfo ].hShader = 0;
	cls.HudDrawElements[ iInfo ].shaderName[ 0 ] = 0;

	if( cge ) {
		cge->CG_HudDrawFont( iInfo );
	}
#endif
}

/*
==================
SV_ArchiveViewModelAnimation
==================
*/
void SV_ArchiveViewModelAnimation( qboolean loading )
{
#ifndef DEDICATED
    int i;

    for (i = 0; i < MAX_FRAMEINFOS; i++)
    {
        ge->ArchiveInteger(&cls.anim.g_VMFrameInfo[i].index);
        ge->ArchiveFloat(&cls.anim.g_VMFrameInfo[i].time);
        ge->ArchiveFloat(&cls.anim.g_VMFrameInfo[i].weight);
    }

    ge->ArchiveInteger(&cls.anim.g_iLastVMAnim);
    ge->ArchiveInteger(&cls.anim.g_iLastVMAnimChanged);
    ge->ArchiveInteger(&cls.anim.g_iCurrentVMAnimSlot);
    ge->ArchiveInteger(&cls.anim.g_iCurrentVMDuration);
    ge->ArchiveInteger(&cls.anim.g_bCrossblending);
    ge->ArchiveInteger(&cls.anim.g_iLastEquippedWeaponStat);
    ge->ArchiveString(cls.anim.g_szLastActiveItem);
    ge->ArchiveInteger(&cls.anim.g_iLastAnimPrefixIndex);
    ge->ArchiveFloat(&cls.anim.g_vCurrentVMPosOffset[0]);
    ge->ArchiveFloat(&cls.anim.g_vCurrentVMPosOffset[1]);
    ge->ArchiveFloat(&cls.anim.g_vCurrentVMPosOffset[2]);
#endif
}

/*
==================
SV_ArchiveStopwatch
==================
*/
void SV_ArchiveStopwatch( qboolean loading )
{
#ifndef DEDICATED
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
#ifndef DEDICATED
	const char *name;

	Com_DPrintf( "SV_ArchivePersistantFile()\n" );
	name = Com_GetArchiveFileName( svs.gameName, "spv" );
	ge->ArchivePersistant( name, loading );
#endif
}

/*
==================
SV_ArchiveLevel
==================
*/
void SV_ArchiveLevel( qboolean loading )
{
#ifndef DEDICATED
	SV_ArchiveHudDrawElements( loading );
	SV_ArchiveViewModelAnimation( loading );
	SV_ArchiveStopwatch( loading );
#endif
}

/*
==================
SV_ArchiveLevelFile
==================
*/
qboolean SV_ArchiveLevelFile(qboolean loading, qboolean autosave)
{
#ifndef DEDICATED
    const char* name;
    fileHandle_t f;
    savegamestruct_t save;
    soundsystemsavegame_t SSsave;

    Com_DPrintf("SV_ArchiveLevelFile()\n");
    name = Com_GetArchiveFileName(svs.gameName, "sav");
    if (loading)
    {
		if (!ge->ReadLevel(name)) {
			return qfalse;
        }

        name = Com_GetArchiveFileName(svs.gameName, "ssv");
        FS_FOpenFileRead(name, &f, qfalse, qtrue);
        if (f)
        {
            FS_Read(&save, sizeof(savegamestruct_t), f);
            if (save.version != 4)
            {
                FS_FCloseFile(f);
                return qfalse;
            }

            FS_Read(&SSsave, sizeof(soundsystemsavegame_t), f);
            CM_ReadPortalState(f);
            FS_FCloseFile(f);
        }
    }
    else
    {
		cls.savedCgameStateSize = cge->CG_SaveStateToBuffer(&cls.savedCgameState, svs.time);
        ge->WriteLevel(name, autosave);
		Z_Free(cls.savedCgameState);
		cls.savedCgameState = NULL;
    }

    return qtrue;
#else
	return qfalse;
#endif
}

/*
==================
S_Save
==================
*/
void S_Save( fileHandle_t f )
{
#ifndef DEDICATED
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
#ifndef DEDICATED
	FS_Read( &svs.soundSystem, sizeof( svs.soundSystem ), f );
	S_LoadData(&svs.soundSystem);
#endif
}

/*
==================
SV_ArchiveServerFile
==================
*/
qboolean SV_ArchiveServerFile( qboolean loading, qboolean autosave )
{
#ifndef DEDICATED
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
			Com_Printf("Invalid or Old Server SaveGame Version\n");
			return qfalse;
		}

		S_StopAllSounds2( qtrue );
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
#else
	return qfalse;
#endif
}

/*
==================
SV_Loadgame_f
==================
*/
void SV_Loadgame_f( void )
{
#ifndef DEDICATED
	int length;
	const char *name;
	const char *archive_name;
	qboolean bStartedGame;

	if( com_cl_running && com_cl_running->integer &&
		clc.state != CA_DISCONNECTED && cg_gametype->integer ||
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
#endif
}

/*
==================
SV_SavegameFilename
==================
*/
void SV_SavegameFilename( const char *name, char *fileName, int length )
{
#ifndef DEDICATED
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
#endif
}

/*
==================
SV_AllowSaveGame
==================
*/
qboolean SV_AllowSaveGame( void )
{
#ifndef DEDICATED
	if( !com_sv_running || !com_sv_running->integer )
	{
		Com_DPrintf( "You must be in a game with a server to save.\n" );
		return qfalse;
	}
	else if( !com_cl_running || !com_cl_running->integer )
	{
		Com_DPrintf( "You must be in a game with a client to save.\n" );
		return qfalse;
	}
	else if( sv.state != SS_GAME )
	{
		Com_DPrintf( "You must be in game to save.\n" );
		return qfalse;
	}
	else if( clc.state != CA_DISCONNECTED && cg_gametype->integer )
	{
		Com_DPrintf( "Can't savegame in a multiplayer game\n" );
		return qfalse;
	}
	else if( g_gametype->integer )
	{
		Com_DPrintf( "Can't savegame in a multiplayer game\n" );
		return qfalse;
	}
	else if( !svs.clients || svs.clients->gentity == NULL || svs.clients->gentity->client == NULL || !svs.clients->gentity->client->ps.stats[ 0 ] )
	{
		Com_DPrintf( "Can't savegame when dead\n" );
		return qfalse;
	}
	else if( sv.state == SS_LOADING || sv.state == SS_LOADING2 )
	{
		Com_DPrintf( "Can't save game when loading\n" );
		return qfalse;
	}

	return qtrue;
#else
	return qfalse;
#endif
}

/*
==================
SV_DoSaveGame
==================
*/
static qboolean bSavegame;

qboolean SV_DoSaveGame()
{
#ifndef DEDICATED
	if( bSavegame )
	{
		if( SV_AllowSaveGame() ) {
			return qtrue;
		}
		bSavegame = qfalse;
	}
	return qfalse;
#else
	return qfalse;
#endif
}

/*
==================
SV_SaveGame
==================
*/
void SV_SaveGame( const char *gamename, qboolean autosave )
{
#ifndef DEDICATED
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
#ifndef DEDICATED
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
#endif
}

/*
==================
SV_CheckSaveGame
==================
*/
void SV_CheckSaveGame( void )
{
#ifndef DEDICATED
	if( !SV_DoSaveGame() ) {
		return;
	}

	if( cl.serverTime >= svs.time )
	{
		bSavegame = qfalse;
		SV_SaveGame( savegame_name[ 0 ] ? savegame_name : NULL, qfalse );
		UI_SetupFiles();
	}
#endif
}

void SV_Autosavegame_f( void )
{
#ifndef DEDICATED
	SV_SaveGame( NULL, qtrue );
#endif
}
