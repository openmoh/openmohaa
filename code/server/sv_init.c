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
#include "../client/client.h"
#include "localization.h"

static char last_mapname[ MAX_QPATH ];
static int g_iSvsTimeFixupCount;
static int *g_piSvsTimeFixups[ 2048 ];

/*
===============
SV_ClearSvsTimeFixups
===============
*/
void SV_ClearSvsTimeFixups( void )
{
	g_iSvsTimeFixupCount = 0;
}

/*
===============
SV_FinishSvsTimeFixups
===============
*/
void SV_FinishSvsTimeFixups( void )
{
	int i;
	for( i = 0; i < g_iSvsTimeFixupCount; i++ )
	{
		*g_piSvsTimeFixups[ i ] = svs.time;
	}
	SV_ClearSvsTimeFixups();
}

/*
===============
SV_AddSvsTimeFixup
===============
*/
void SV_AddSvsTimeFixup( int *piTime )
{
	g_piSvsTimeFixups[ g_iSvsTimeFixupCount++ ] = piTime;
}

void SV_SendConfigstring( client_t *client, int index );

/*
===============
SV_SetConfigstring

===============
*/
void SV_SetConfigstring (int index, const char *val) {
	int i;
	size_t len;
	client_t	*client;

	if ( index < 0 || index >= MAX_CONFIGSTRINGS ) {
		Com_Error( ERR_DROP, "SV_SetConfigstring: bad index %i\n", index );
	}

	if ( !val ) {
		val = "";
	}

	// don't bother broadcasting an update if no change
	if ( !strcmp( val, sv.configstrings[ index ] ) ) {
		return;
	}

	// change the string in sv
	Z_Free( sv.configstrings[index] );
	sv.configstrings[index] = CopyString( val );

	// send it to all the clients if we aren't
	// spawning a new server
	if ( sv.state == SS_GAME || sv.restarting ) {

		// send the data to all relevent clients
		for (i = 0, client = svs.clients; i < svs.iNumClients ; i++, client++) {
			if ( client->state < CS_ACTIVE ) {
				if ( client->state == CS_PRIMED )
					client->csUpdated[ index ] = qtrue;
				continue;
			}
			// do not always send server info to all clients
			if ( index == CS_SERVERINFO && client->gentity && (client->gentity->r.svFlags & SVF_NOSERVERINFO) ) {
				continue;
			}
		

			len = strlen( val );
			SV_SendConfigstring(client, index);
		}
	}
}

/*
===============
SV_GetConfigstring

===============
*/
char *SV_GetConfigstring( int index )
{
	char *buffer;

	if ( index < 0 || index >= MAX_CONFIGSTRINGS ) {
		Com_Error (ERR_DROP, "SV_GetConfigstring: bad index %i\n", index);
	}
	if ( !sv.configstrings[index] ) {
		return NULL;
	}

	buffer = Hunk_AllocateTempMemory( strlen( sv.configstrings[ index ] ) + 1 );

	strcpy( buffer, sv.configstrings[index] );

	return buffer;
}

/*
================
SV_FindIndex
================
*/
int SV_FindIndex( const char *name, int start, int max, qboolean create ) {
	int		i;
	char	*s;

	if( !name || !name[ 0 ] ) {
		return 0;
	}

	if( start >= MAX_CONFIGSTRINGS ) {
		Com_Error( 1, "SV_FindIndex: bad start index %i\n", start );
	}

	if( max < 0 || max + start >= MAX_CONFIGSTRINGS ) {
		Com_Error( 1, "SV_FindIndex: bad max index %i\n", max );
	}

	for( i = 1; i<max; i++ ) {
		s = sv.configstrings[ start + i ];

		if( !s || !s[ 0 ] ) {
			break;
		}
		if( !strcmp( s, name ) ) {
			return i;
		}
	}

	if( !create ) {
		return 0;
	}

	if( i == max ) {
		Com_Error( 1, "SV_FindIndex: overflow  max%d create%d  name %s", max, create, name );
	}

	SV_SetConfigstring( start + i, name );
	return i;
}

/*
===============
SV_ModelIndex
===============
*/
int SV_ModelIndex( const char *name )
{
	return SV_FindIndex( name, CS_MODELS, MAX_MODELS, qtrue );
}

/*
===============
SV_ClearModel
===============
*/
void SV_ClearModel( int index )
{
	SV_SetConfigstring( CS_MODELS + index, "" );
}

/*
===============
SV_SoundIndex
===============
*/
int SV_SoundIndex( const char *name, qboolean streamed )
{
	char buf[ 1024 ];

	strcpy( buf, name );
	strcat( buf, va( "%d", streamed ) );

	return SV_FindIndex( buf, CS_SOUNDS, MAX_SOUNDS, qtrue );
}

/*
===============
SV_ImageIndex
===============
*/
int SV_ImageIndex( const char *name )
{
	return SV_FindIndex( name, CS_IMAGES, MAX_IMAGES, qtrue );
}

/*
===============
SV_ItemIndex
===============
*/
int SV_ItemIndex( const char *name )
{
	return SV_FindIndex( name, CS_WEAPONS, MAX_WEAPONS, qtrue );
}

/*
===============
SV_SetLightStyle
===============
*/
void SV_SetLightStyle( int index, const char *data )
{
	SV_SetConfigstring( CS_LIGHTSTYLES + index, data );
}

/*
===============
SV_SendConfigstring

Creates and sends the server command necessary to update the CS index for the
given client
===============
*/
static void SV_SendConfigstring(client_t *client, int index)
{
	int maxChunkSize = MAX_STRING_CHARS - 24;
	size_t len;

	len = strlen(sv.configstrings[index]);

	if( len >= maxChunkSize ) {
		int		sent = 0;
		size_t	remaining = len;
		char	*cmd;
		char	buf[MAX_STRING_CHARS];

		while (remaining > 0 ) {
			if ( sent == 0 ) {
				cmd = "bcs0";
			}
			else if( remaining < maxChunkSize ) {
				cmd = "bcs2";
			}
			else {
				cmd = "bcs1";
			}
			Q_strncpyz( buf, &sv.configstrings[index][sent],
				maxChunkSize );

			SV_SendServerCommand( client, "%s %i \"%s\"\n", cmd,
				index, buf );

			sent += (maxChunkSize - 1);
			remaining -= (maxChunkSize - 1);
		}
	} else {
		// standard cs, just send it
		SV_SendServerCommand( client, "cs %i \"%s\"\n", index,
			sv.configstrings[index] );
	}
}

/*
===============
SV_UpdateConfigstrings

Called when a client goes from CS_PRIMED to CS_ACTIVE.  Updates all
Configstring indexes that have changed while the client was in CS_PRIMED
===============
*/
void SV_UpdateConfigstrings(client_t *client)
{
	int index;

	for( index = 0; index <= MAX_CONFIGSTRINGS; index++ ) {
		// if the CS hasn't changed since we went to CS_PRIMED, ignore
		if(!client->csUpdated[index])
			continue;

		// do not always send server info to all clients
		if ( index == CS_SERVERINFO && client->gentity &&
			(client->gentity->r.svFlags & SVF_NOSERVERINFO) ) {
			continue;
		}
		SV_SendConfigstring(client, index);
		client->csUpdated[index] = qfalse;
	}
}

/*
===============
SV_SetUserinfo

===============
*/
void SV_SetUserinfo( int index, const char *val ) {
	if ( index < 0 || index >= sv_maxclients->integer ) {
		Com_Error (ERR_DROP, "SV_SetUserinfo: bad index %i\n", index);
	}

	if ( !val ) {
		val = "";
	}

	Q_strncpyz( svs.clients[index].userinfo, val, sizeof( svs.clients[ index ].userinfo ) );
	Q_strncpyz( svs.clients[index].name, Info_ValueForKey( val, "name" ), sizeof(svs.clients[index].name) );
}



/*
===============
SV_GetUserinfo

===============
*/
void SV_GetUserinfo( int index, char *buffer, int bufferSize ) {
	if ( bufferSize < 1 ) {
		Com_Error( ERR_DROP, "SV_GetUserinfo: bufferSize == %i", bufferSize );
	}
	if ( index < 0 || index >= sv_maxclients->integer ) {
		Com_Error (ERR_DROP, "SV_GetUserinfo: bad index %i\n", index);
	}
	Q_strncpyz( buffer, svs.clients[ index ].userinfo, bufferSize );
}


/*
================
SV_CreateBaseline

Entity baselines are used to compress non-delta messages
to the clients -- only the fields that differ from the
baseline will be transmitted
================
*/
void SV_CreateBaseline( void ) {
	gentity_t		*ent;
	int				entnum;	

	for ( entnum = 1; entnum < sv.num_entities ; entnum++ ) {
		ent = SV_GentityNum(entnum);
		if (!ent->r.linked) {
			continue;
		}
		ent->s.number = entnum;

		//
		// take current state as baseline
		//
		sv.svEntities[entnum].baseline = ent->s;
	}
}

/*
===============
SV_BoundMaxClients

===============
*/
void SV_BoundMaxClients( int minimum ) {
	// get the current maxclients value
	Cvar_Get( "sv_maxclients", "8", 0 );

	sv_maxclients->modified = qfalse;

	if ( sv_maxclients->integer < minimum ) {
		Cvar_Set( "sv_maxclients", va("%i", minimum) );
	} else if ( sv_maxclients->integer > MAX_CLIENTS ) {
		Cvar_Set( "sv_maxclients", va("%i", MAX_CLIENTS) );
	}
}

void SV_ClientsAlloc( void )
{
	svs.iNumClients = sv_maxclients->integer;
	svs.clients = Z_Malloc( svs.iNumClients * sizeof( client_t ) );
	Com_Memset( svs.clients, 0, svs.iNumClients * sizeof( client_t ) );
	if( g_gametype->integer ) {
		svs.numSnapshotEntities = svs.iNumClients * PACKET_BACKUP * 64;
	} else {
		svs.numSnapshotEntities = 1 * 32 * 64;
	}

	SV_InitAllCGMessages();
}


/*
===============
SV_Startup

Called when a host starts a map when it wasn't running
one before.  Successive map or map_restart commands will
NOT cause this to be called, unless the game is exited to
the menu system first.
===============
*/
void SV_Startup( void ) {
	if ( svs.initialized ) {
		Com_Error( ERR_FATAL, "SV_Startup: svs.initialized already set" );
	}
	SV_BoundMaxClients( 1 );
	SV_ClientsAlloc();

	svs.initialized = qtrue;
	memset( last_mapname, 0, sizeof( last_mapname ) );

	// FIXME: use another master network ?
	//SV_InitGamespy();
	Cvar_Set( "sv_running", "1" );
}

/*
==================
SV_ChangeMaxClients
==================
*/
void SV_ChangeMaxClients( void ) {
	int			oldMaxClients;
	int			i;
	client_t	*oldClients;
	int			count;

	// get the highest client number in use
	count = 0;
	for ( i = 0 ; i < svs.iNumClients ; i++ ) {
		if ( svs.clients[i].state >= CS_CONNECTED ) {
			if (i > count)
				count = i;
		}
	}
	count++;

	oldMaxClients = sv_maxclients->integer;
	// never go below the highest client number in use
	SV_BoundMaxClients( count );
	// if still the same
	if ( sv_maxclients->integer == oldMaxClients ) {
		return;
	}

	oldClients = svs.clients;
	SV_ClientsAlloc();

	// copy the clients to hunk memory
	for ( i = 0 ; i < count ; i++ ) {
		if( oldClients[ i ].state >= CS_CONNECTED ) {
			svs.clients[ i ] = oldClients[ i ];
		}
	}

	// free old clients arrays
	Z_Free( oldClients );
}

/*
================
SV_ClearServer
================
*/
void SV_ClearServer(void) {
	int i;

	for ( i = 0 ; i < MAX_CONFIGSTRINGS ; i++ ) {
		if ( sv.configstrings[i] ) {
			Z_Free( sv.configstrings[i] );
		}
	}
	Com_Memset (&sv, 0, sizeof(sv));
}

/*
================
SV_TouchCGame

  touch the cgame.vm so that a pure client can load it if it's in a seperate pk3
================
*/
void SV_TouchCGame(void) {
	fileHandle_t	f;
	char filename[MAX_QPATH];

	Com_sprintf( filename, sizeof(filename), "vm/%s.qvm", "cgame" );
	FS_FOpenFileRead( filename, &f, qfalse, qtrue );
	if ( f ) {
		FS_FCloseFile( f );
	}
}

/*
================
SV_SpawnServer

Change the server to a new map, taking all connected
clients along with it.
This is NOT called for map_restart
================
*/
void SV_SpawnServer( const char *server, qboolean loadgame, qboolean restart, qboolean bTransition ) {
	int			i;
	int			iStart;
	int			iEnd;
	int			checksum;
	char		systemInfo[ MAX_INFO_STRING ];
	char		mapname[ MAX_QPATH ];
	char		*spawnpos;
	qboolean	differentmap;
	qboolean	keep_scripts;
	const char	*p;

	keep_scripts = restart;

	Com_Printf ("------ Server Initialization ------\n");
	iStart = Sys_Milliseconds();
	Com_Printf ("Server: %s\n",server);

	sv.state = SS_LOADING;
	svs.autosave = qfalse;
	svs.soundsNeedLoad = qfalse;

	Com_Unpause();
	
	differentmap = qtrue;

	strncpy( svs.rawServerName, server, sizeof( svs.rawServerName ) );

	// check for a spawn position
	spawnpos = strchr( server, '$' );
	if( spawnpos )
	{
		Q_strncpyz( mapname, server, spawnpos - server + 1 );
	}
	else
	{
		strcpy( mapname, server );
	}

	strncpy( svs.mapName, mapname, sizeof( svs.mapName ) );

	if( svs.initialized )
	{
		if( ge && !stricmp( last_mapname, mapname ) && ( restart || loadgame ) ) {
			differentmap = qfalse;
		}

		if( sv_maxclients->modified ) {
			SV_ChangeMaxClients();
		}
	}
	else
	{
		keep_scripts = qfalse;
		SV_Startup();
	}

	sv.restarting = ( keep_scripts || !differentmap );
	strncpy( last_mapname, mapname, sizeof( last_mapname ) );

	// if not running a dedicated server CL_MapLoading will connect the client to the server
	// also print some status stuff
	CL_MapLoading( differentmap, mapname );

	UI_LoadResource( "*132" );

	if( !loadgame )
	{
		svs.startTime = svs.time;
		svs.serverLagTime = svs.time;
	}

	if( differentmap )
	{
		// shut down the existing game if it is running
		SV_ShutdownGameProgs();

		// clear the whole hunk because we're (re)loading the server
		Hunk_Clear();

		// init game imports
		SV_InitGameProgs();

		// allocate the snapshot entities on the hunk
		svs.snapshotEntities = Hunk_Alloc( sizeof( entityState_t )*svs.numSnapshotEntities );
		svs.nextSnapshotEntities = 0;

		// wipe the entire per-level structure
		SV_ClearServer();
		for( i = 0; i < MAX_CONFIGSTRINGS; i++ ) {
			sv.configstrings[ i ] = CopyString( "" );
		}
	}
	else
	{
		ge->Restart();
	}

	keep_scripts = qfalse;

	if( !differentmap && ( !developer->integer || loadgame ) ) {
		keep_scripts = qtrue;
	}

	ge->Cleanup( keep_scripts );
	ge->SetTime( svs.startTime, svs.time );

	UI_LoadResource( "*133" );

	SV_ClearModelUserCounts();

	TIKI_End();
	TIKI_Begin();

	UI_LoadResource( "*134" );

	if( differentmap )
	{
		char filename[ MAX_QPATH ];

		TIKI_FreeAll();
		Com_sprintf( filename, sizeof( filename ), "maps/%s.bsp", mapname );
		CM_LoadMap( filename, qfalse, &checksum );

		// set checksum
		Cvar_Set( "sv_mapChecksum", va( "%i", checksum ) );

		// set serverinfo visible name
		Cvar_Set( "mapname", server );
	}

	CL_InitClientSavedData();

	UI_LoadResource( "*135" );

	if( differentmap )
	{
		sv.serverId = com_frameTime;
		sv.restartedServerId = com_frameTime;
	}
	else
	{
		sv.restartedServerId = sv.serverId;
		sv.serverId = com_frameTime;
	}

	Cvar_Set( "sv_serverid", va( "%i", sv.serverId ) );

	// toggle the server bit so clients can detect that a
	// server has changed
	svs.snapFlagServerBit ^= SNAPFLAG_SERVERCOUNT;

	CM_ResetAreaPortals();

	UI_LoadResource( "*136" );

	// clear soundtrack
	if( !loadgame ) {
		SV_SetConfigstring( CS_MUSIC, "none" );
	}

	// clear physics interaction links
	SV_ClearWorld();

	UI_LoadResource( "*137" );

	// set game dll map
	ge->SetMap( sv_mapname->string );

	if( !keep_scripts ) {
		ge->Precache();
	}

	UI_LoadResource( "*138" );

	if( loadgame )
	{
		if( SV_ArchiveLevelFile( qtrue, qfalse ) )
		{
			if( differentmap && com_dedicated->integer )
			{
				TIKI_FinishLoad();
			}
		}
		else
		{
			ge->Cleanup( keep_scripts );
			loadgame = qfalse;
		}
	}

	UI_LoadResource( "*139" );

	if( !loadgame )
	{
		// load and spawn all other entities
		svs.areabits_warning_time = 0;
		UI_LoadResource( "*139a" );

		// tell the game dll to spawn entities
		ge->SpawnEntities( CM_EntityString(), svs.time );

		UI_LoadResource( "*140" );

		p = ge->errorMessage;
		if( p )
		{
			ge->errorMessage = NULL;
			Com_Error( ERR_DROP, p );
		}

		// save persistant structures
		if( bTransition ) {
			SV_ArchivePersistantFile( qtrue );
		}

		UI_LoadResource( "*141" );

		if( differentmap && com_dedicated->integer ) {
			TIKI_FinishLoad();
		}

		// don't allow a map_restart if game is modified
		g_gametype->modified = qfalse;

		// run a few frames to allow everything to settle
		for( i = 0; i < 3; i++ )
		{
			ge->RunFrame( svs.time, 100 );
			svs.time += 100;

			p = ge->errorMessage;
			if( p )
			{
				ge->errorMessage = NULL;
				Com_Error( ERR_DROP, p );
			}
		}

		svs.mapTime = svs.time - svs.startTime;
	}

	UI_LoadResource( "*142" );

	if( differentmap )
	{
		// create a baseline for more efficient communications
		SV_CreateBaseline();

		for( i = 0; i < svs.iNumClients; i++ ) {
			// send the new gamestate to all connected clients
			if( svs.clients[ i ].state >= CS_CONNECTED ) {
				char	*denied;

				if( svs.clients[ i ].netchan.remoteAddress.type != NA_BOT
					|| Cvar_VariableIntegerValue( "g_gametype" ) == 0 )
				{
					// connect the client again
					denied = ge->ClientConnect( i, qfalse );

					p = ge->errorMessage;
					if( p )
					{
						ge->errorMessage = NULL;
						Com_Error( ERR_DROP, p );
					}

					if( denied ) {
						// this generally shouldn't happen, because the client
						// was connected before the level change
						SV_DropClient( &svs.clients[ i ], denied );
					} else {
						// when we get the next packet from a connected client,
						// the new gamestate will be sent
						svs.clients[ i ].state = CS_CONNECTED;
					}
				}
				else
				{
					SV_DropClient( &svs.clients[ i ], denied );
				}
			}
		}

		// the server sends these to the clients so they can figure
		// out which pk3s should be auto-downloaded
		Cvar_Set( "sv_paks", "" );
		p = FS_ReferencedPakChecksums();
		Cvar_Set( "sv_referencedPaks", p );
		p = FS_ReferencedPakNames();
		Cvar_Set( "sv_referencedPakNames", p );

		// save systeminfo and serverinfo strings
		Q_strncpyz( systemInfo, Cvar_InfoString( CVAR_SYSTEMINFO ), sizeof( systemInfo ) );
		cvar_modifiedFlags &= ~CVAR_SYSTEMINFO;
		SV_SetConfigstring( CS_SYSTEMINFO, systemInfo );

		SV_SetConfigstring( CS_SERVERINFO, Cvar_InfoString( CVAR_SERVERINFO ) );
		cvar_modifiedFlags &= ~CVAR_SERVERINFO;

		// any media configstring setting now should issue a warning
		// and any configstring changes should be reliably transmitted
		// to all clients
		sv.state = SS_GAME;
		sv.restarting = qfalse;

		// send a heartbeat now so the master will get up to date info
		SV_Heartbeat_f();

		Hunk_SetMark();

		if( com_dedicated->integer ) {
			ge->RegisterSounds();
		}
	}
	else
	{
		sv.state = SS_GAME;
		sv.restarting = qfalse;

		for( i = 0; i < svs.iNumClients; i++ )
		{
			if( svs.clients[ i ].state >= CS_CONNECTED ) {
				char	*denied;

				// connect the client again
				denied = ge->ClientConnect( i, qfalse );

				p = ge->errorMessage;
				if( p )
				{
					ge->errorMessage = NULL;
					Com_Error( ERR_DROP, p );
				}

				if( denied ) {
					// this generally shouldn't happen, because the client
					// was connected before the level change
					SV_DropClient( &svs.clients[ i ], denied );
				} else {
					// send the client's last usercmd
					SV_ClientEnterWorld( &svs.clients[ i ], &svs.clients[ i ].lastUsercmd );
				}
			}
		}
	}

	strcpy( svs.gameName, "current" );

	iEnd = Sys_Milliseconds();
	Com_Printf( "------ Server Initialization Complete ------ %5.2f seconds\n", ( float )iEnd / 1000.0f );

	UI_LoadResource( "*143" );

	if( g_gametype->integer ) {
		SV_ServerLoaded();
	}
}

/*
===============
SV_ServerLoaded
===============
*/
void SV_ServerLoaded( void ) {
	const char	*p;

	if( !com_sv_running->integer ) {
		return;
	}

	if( sv.state == SS_GAME ) {
		return;
	}

	sv.state = SS_GAME;
	sv.timeResidual = 0;
	svs.startTime = svs.time - svs.mapTime;
	if( svs.time - svs.mapTime < 0 )
	{
		svs.time -= svs.time - svs.mapTime;
		svs.startTime = 0;
	}

	// notify the new time
	svs.serverLagTime = svs.time;
	ge->SetTime( svs.startTime, svs.time );
	SV_FinishSvsTimeFixups();

	// notify the game dll
	ge->ServerSpawned();

	p = ge->errorMessage;
	if( p )
	{
		ge->errorMessage = NULL;
		Com_Error( ERR_DROP, p );
	}

	if( svs.autosave )
	{
		if( stricmp( svs.rawServerName, "credits" ) ) {
			SV_Autosavegame_f();
		}
		svs.autosave = qfalse;
	}
}

/*
===============
SV_Init

Only called at main exe startup, not for each game
===============
*/
void SV_Init( void ) {
	SV_AddOperatorCommands();

	// serverinfo vars
	Cvar_Get( "dmflags", "0", CVAR_SERVERINFO );
	Cvar_Get( "fraglimit", "20", CVAR_SERVERINFO );
	Cvar_Get( "timelimit", "0", CVAR_SERVERINFO );
	Cvar_Get( "sv_keywords", "", CVAR_SERVERINFO );
	Cvar_Get( "protocol", va( "%i", PROTOCOL_VERSION ), CVAR_SERVERINFO | CVAR_ROM );
	sv_mapname = Cvar_Get( "mapname", "nomap", CVAR_SERVERINFO | CVAR_ROM );
	sv_privateClients = Cvar_Get( "sv_privateClients", "0", CVAR_SERVERINFO );
	sv_hostname = Cvar_Get( "sv_hostname", "Nameless Battle", CVAR_SERVERINFO | CVAR_ARCHIVE );
	sv_maxclients = Cvar_Get( "sv_maxclients", "8", CVAR_SERVERINFO | CVAR_LATCH );
	sv_maxRate = Cvar_Get( "sv_maxRate", "0", CVAR_ARCHIVE | CVAR_SERVERINFO );
	sv_minPing = Cvar_Get( "sv_minPing", "0", CVAR_ARCHIVE | CVAR_SERVERINFO );
	sv_maxPing = Cvar_Get( "sv_maxPing", "0", CVAR_ARCHIVE | CVAR_SERVERINFO );
	sv_floodProtect = Cvar_Get( "sv_floodProtect", "1", CVAR_ARCHIVE | CVAR_SERVERINFO );
	sv_maplist = Cvar_Get( "sv_maplist", "", CVAR_ARCHIVE | CVAR_SERVERINFO );
	g_gametype = Cvar_Get ("g_gametype", "0", CVAR_SERVERINFO | CVAR_LATCH );
	g_gametypestring = Cvar_Get( "g_gametypestring", "0", CVAR_SERVERINFO | CVAR_LATCH );

	// systeminfo
	Cvar_Get( "cheats", "1", CVAR_INIT | CVAR_LATCH );
	sv_serverid = Cvar_Get ("sv_serverid", "0", CVAR_SYSTEMINFO | CVAR_ROM );
	// wombat: pure only makes problems at current stage
	sv_pure = Cvar_Get ("sv_pure", "0", CVAR_SYSTEMINFO );
	Cvar_Get ("sv_paks", "", CVAR_SYSTEMINFO | CVAR_ROM );

	// server vars
	sv_rconPassword = Cvar_Get ("rconPassword", "", CVAR_TEMP );
	sv_privatePassword = Cvar_Get ("sv_privatePassword", "", CVAR_TEMP );
	sv_fps = Cvar_Get ("sv_fps", "20", CVAR_TEMP );
	sv_timeout = Cvar_Get ("sv_timeout", "120", CVAR_TEMP );
	sv_zombietime = Cvar_Get ("sv_zombietime", "2", CVAR_TEMP );
	Cvar_Get ("nextmap", "", CVAR_TEMP );

	sv_allowDownload = Cvar_Get ("sv_allowDownload", "1", CVAR_SERVERINFO);
	Cvar_Get ("sv_dlURL", "", CVAR_SERVERINFO | CVAR_ARCHIVE);
	sv_master[ 0 ] = Cvar_Get( "sv_master1", MASTER_SERVER_NAME, 0 );
	sv_master[ 1 ] = Cvar_Get( "sv_master2", "", CVAR_ARCHIVE );
	sv_master[ 2 ] = Cvar_Get( "sv_master3", "", CVAR_ARCHIVE );
	sv_master[ 3 ] = Cvar_Get( "sv_master4", "", CVAR_ARCHIVE );
	sv_master[ 4 ] = Cvar_Get( "sv_master5", "", CVAR_ARCHIVE );
	sv_reconnectlimit = Cvar_Get ("sv_reconnectlimit", "3", 0);
	sv_showloss = Cvar_Get ("sv_showloss", "0", 0);
	sv_padPackets = Cvar_Get ("sv_padPackets", "0", 0);
	sv_killserver = Cvar_Get ("sv_killserver", "0", 0);
	sv_mapChecksum = Cvar_Get ("sv_mapChecksum", "", CVAR_ROM);
	sv_drawentities = Cvar_Get( "sv_drawentities", "1", 0 );
	sv_deeptracedebug = Cvar_Get( "sv_deeptracedebug", "0", 0 );
	sv_chatter = Cvar_Get( "sv_chatter", "0", 0 );
	sv_lanForceRate = Cvar_Get ("sv_lanForceRate", "1", CVAR_ARCHIVE );
	sv_strictAuth = Cvar_Get ("sv_strictAuth", "1", CVAR_ARCHIVE );

	strcpy( svs.gameName, "current" );

	// dday vars
	Cvar_Get( "g_ddayfodderguys", "0", CVAR_ARCHIVE );
	Cvar_Get( "g_ddayfog", "2", CVAR_ARCHIVE );
	Cvar_Get( "g_ddayshingleguys", "0", CVAR_ARCHIVE );
}


/*
==================
SV_FinalMessage

Used by SV_Shutdown to send a final message to all
connected clients before the server goes down.  The messages are sent immediately,
not just stuck on the outgoing message list, because the server is going
to totally exit after returning from this function.
==================
*/
void SV_FinalMessage( const char *message ) {
	int			i, j;
	client_t	*cl;
	
	// send it twice, ignoring rate
	for ( j = 0 ; j < 2 ; j++ ) {
		for (i=0, cl = svs.clients ; i < sv_maxclients->integer ; i++, cl++) {
			if (cl->state >= CS_CONNECTED) {
				// don't send a disconnect to a local client
				if ( cl->netchan.remoteAddress.type != NA_LOOPBACK ) {
					SV_SendServerCommand( cl, "print \"%s\n\"\n", message );
					SV_SendServerCommand( cl, "disconnect \"%s\"", message );
				}
				// force a snapshot to be sent
				cl->nextSnapshotTime = -1;
				SV_SendClientSnapshot( cl );
			}
		}
	}
}


/*
================
SV_Shutdown

Called when each game quits,
before Sys_Quit or SyScriptError
================
*/
void SV_Shutdown( const char *finalmsg ) {
	if ( !com_sv_running || !com_sv_running->integer ) {
		return;
	}

	Com_Printf( "----- Server Shutdown (%s) -----\n", finalmsg );

	if ( svs.clients && !com_errorEntered ) {
		SV_FinalMessage( finalmsg );
	}

	SV_RemoveOperatorCommands();
	SV_MasterShutdown();
	SV_ShutdownGameProgs();

	// free current level
	SV_ClearServer();

	// free server static data
	if ( svs.clients ) {
		Z_Free( svs.clients );
	}
	Com_Memset( &svs, 0, sizeof( svs ) );

	Cvar_Set( "sv_running", "0" );
	Cvar_Set("ui_singlePlayerActive", "0");

	Com_Printf( "---------------------------\n" );

	// disconnect any local clients
	if( sv_killserver->integer != 2 )
		CL_Disconnect( qfalse );
}

