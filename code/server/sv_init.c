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
#include "../qcommon/tiki.h"
#include "../qcommon/bg_compat.h"
#include "../gamespy/sv_gamespy.h"

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
	int		i;
	client_t	*client;

	if ( index < 0 || index >= MAX_CONFIGSTRINGS ) {
		Com_Error (ERR_DROP, "SV_SetConfigstring: bad index %i\n", index);
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
	if (sv.state == SS_LOADING2 || sv.state == SS_GAME || sv.restarting ) {

		// send the data to all relevant clients
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
		if( !Q_stricmp( s, name ) ) {
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

	Q_strncpyz( buf, name, sizeof(buf) );
	Q_strcat( buf, sizeof(buf), va( "%d", streamed ) );

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
void SV_SendConfigstring(client_t *client, int index)
{
	int maxChunkSize = MAX_STRING_CHARS - 24;
	int denormalized;
	size_t len;

	denormalized = CPT_DenormalizeConfigstring(index);
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
				denormalized, buf );

			sent += (maxChunkSize - 1);
			if (remaining > maxChunkSize - 1) {
				remaining -= (maxChunkSize - 1);
			} else {
				// Make sure to not go negative
				remaining = 0;
			}
		}
	} else {
		// standard cs, just send it
		SV_SendServerCommand( client, "cs %i \"%s\"\n", denormalized,
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

	for( index = 0; index < MAX_CONFIGSTRINGS; index++ ) {
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
	if( g_gametype->integer != GT_SINGLE_PLAYER ) {
		svs.numSnapshotEntities = svs.iNumClients * PACKET_BACKUP * MAX_CLIENTS;
	} else {
		svs.numSnapshotEntities = 1 * PACKET_BACKUP * MAX_CLIENTS;
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

	SV_InitGamespy();
	Cvar_Set( "sv_running", "1" );
	
	// Join the ipv6 multicast group now that a map is running so clients can scan for us on the local network.
	NET_JoinMulticast6();
}

/*
==================
SV_NumClients
==================
*/
int SV_NumClients(void) {
	int i;
	int numClients = 0;

	for (i = 0; i < svs.iNumClients; i++) {
		if (svs.clients[i].state > CS_FREE) {
			++numClients;
		}
	}

	return numClients;
}

/*
==================
SV_ChangeMaxClients
==================
*/
void SV_ChangeMaxClients( void ) {
	int		oldMaxClients;
	int		i;
	client_t	*oldClients;
	int		count;

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

	if( g_gametype->integer != GT_SINGLE_PLAYER ) {
		svs.numSnapshotEntities = svs.iNumClients * PACKET_BACKUP * MAX_CLIENTS;
	} else {
		svs.numSnapshotEntities = 1 * PACKET_BACKUP * MAX_CLIENTS;
	}
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
	sv.frameTime = 1.0 / sv_fps->value;
}

/*
================
SV_TouchFile
================
*/
static void SV_TouchFile( const char *filename ) {
	fileHandle_t	f;

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

	if (svs.rawServerName != server) {
		// Fixed in OPM
		//  Make sure to not copy from the same string...
		strncpy( svs.rawServerName, server, sizeof( svs.rawServerName ) );
	}

	// Added in 2.30
	Cvar_Set("g_netoptimize", "1");

	// check for a spawn position
	spawnpos = strchr( server, '$' );
	if( spawnpos )
	{
		Q_strncpyz( mapname, server, spawnpos - server + 1 );
	}
	else
	{
		Q_strncpyz( mapname, server, sizeof(mapname) );
	}

	strncpy( svs.mapName, mapname, sizeof( svs.mapName ) );

	if( svs.initialized )
	{
		if( ge && !Q_stricmp( last_mapname, mapname ) && ( restart || loadgame ) ) {
			differentmap = qfalse;
		}

		if( sv_maxclients->modified
			&& differentmap // Fixed in OPM
			//  OG  doesn't check for different map when changing the max clients
			//  which means the snapshot entities memory isn't reallocated
			) {
			SV_ChangeMaxClients();
		}
	}
	else
	{
		keep_scripts = qfalse;
		SV_Startup();
	}

	// Added in 2.0
	SV_InitRadar();

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
		svs.snapshotEntities = Hunk_Alloc( sizeof( entityState_t )*svs.numSnapshotEntities, h_dontcare);
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
		if ( CL_UseLargeLightmap( mapname ) ) {
			Com_sprintf( filename, sizeof( filename ), "maps/%s.bsp", mapname );
		} else {
			// Added in 2.0
			Com_sprintf( filename, sizeof( filename ), "maps/%s_sml.bsp", mapname );
		}
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
			Com_Error(ERR_DROP, "%s", p);
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
			svs.time += 100;
			ge->RunFrame( svs.time, 100 );

			p = ge->errorMessage;
			if( p )
			{
				ge->errorMessage = NULL;
				Com_Error(ERR_DROP, "%s", p);
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
				const char	*denied;

				if( svs.clients[ i ].netchan.remoteAddress.type != NA_BOT
					|| Cvar_VariableIntegerValue( "g_gametype" ) == 0 )
				{
					// connect the client again
					denied = ge->ClientConnect( i, qfalse, qtrue );

					p = ge->errorMessage;
					if( p )
					{
						ge->errorMessage = NULL;
						Com_Error(ERR_DROP, "%s", p);
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

		if ( sv_pure->integer ) {
			// the server sends these to the clients so they will only
			// load pk3s also loaded at the server
			p = FS_LoadedPakChecksums();
			Cvar_Set( "sv_paks", p );
			if (strlen(p) == 0) {
				Com_Printf( "WARNING: sv_pure set but no PK3 files loaded\n" );
			}
			p = FS_LoadedPakNames();
			Cvar_Set( "sv_pakNames", p );
		}
		else {
			Cvar_Set( "sv_paks", "" );
			Cvar_Set( "sv_pakNames", "" );
		}

		if (sv_allowDownload->integer) {
			// the server sends these to the clients so they can figure
			// out which pk3s should be auto-downloaded
			p = FS_ReferencedPakChecksums();
			Cvar_Set("sv_referencedPaks", p);
			p = FS_ReferencedPakNames();
			Cvar_Set("sv_referencedPakNames", p);
        }
        else {
            // Changed in OPM
            //  Don't export the pak list if download is disabled

			Cvar_Set("sv_referencedPaks", "");
			Cvar_Set("sv_referencedPakNames", "");
		}

		// save systeminfo and serverinfo strings
		Q_strncpyz( systemInfo, Cvar_InfoString_Big( CVAR_SYSTEMINFO ), sizeof( systemInfo ) );
		cvar_modifiedFlags &= ~CVAR_SYSTEMINFO;
		SV_SetConfigstring( CS_SYSTEMINFO, systemInfo );

		SV_SetConfigstring( CS_SERVERINFO, Cvar_InfoString( CVAR_SERVERINFO ) );
		cvar_modifiedFlags &= ~CVAR_SERVERINFO;

		// any media configstring setting now should issue a warning
		// and any configstring changes should be reliably transmitted
		// to all clients
		sv.state = SS_LOADING2;
		sv.restarting = qfalse;

		// send a heartbeat now so the master will get up to date info
		SV_Heartbeat_f();

		Hunk_SetMark();

		if( g_gametype->integer != GT_SINGLE_PLAYER ) {
			ge->RegisterSounds();
		}
	}
	else
	{
		sv.state = SS_LOADING2;
		sv.restarting = qfalse;

		for( i = 0; i < svs.iNumClients; i++ )
		{
			if( svs.clients[ i ].state >= CS_CONNECTED ) {
				const char	*denied;

				// connect the client again
				denied = ge->ClientConnect( i, qfalse, qfalse );

				p = ge->errorMessage;
				if( p )
				{
					ge->errorMessage = NULL;
					Com_Error(ERR_DROP, "%s", p);
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

	Q_strncpyz(svs.gameName, "current", sizeof(svs.gameName) );

	iEnd = Sys_Milliseconds();
	Com_Printf( "------ Server Initialization Complete ------ %5.2f seconds\n", ( float )iEnd / 1000.0f );

	UI_LoadResource( "*143" );

	if( g_gametype->integer != GT_SINGLE_PLAYER ) {
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
	if( svs.startTime < 0 )
	{
		svs.time -= svs.startTime;
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
		Com_Error(ERR_DROP, "%s", p);
	}

	if( svs.autosave )
	{
		if(Q_stricmp( svs.rawServerName, "credits" ) ) {
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
void SV_Init (void)
{
	int index;

	SV_AddOperatorCommands();

	// serverinfo vars
	Cvar_Get ("dmflags", "0", CVAR_SERVERINFO);
	Cvar_Get ("fraglimit", "20", CVAR_SERVERINFO);
	Cvar_Get ("timelimit", "0", CVAR_SERVERINFO);
	g_gametype = Cvar_Get ("g_gametype", "0", CVAR_SERVERINFO | CVAR_LATCH );
	g_gametypestring = Cvar_Get( "g_gametypestring", "0", CVAR_SERVERINFO | CVAR_LATCH );
	Cvar_Get( "sv_keywords", "", CVAR_SERVERINFO );
	Cvar_Get( "protocol", va( "%i", PROTOCOL_VERSION ), CVAR_SERVERINFO | CVAR_ROM );
	sv_mapname = Cvar_Get( "mapname", "nomap", CVAR_SERVERINFO | CVAR_ROM );
	sv_privateClients = Cvar_Get( "sv_privateClients", "0", CVAR_SERVERINFO );
	sv_hostname = Cvar_Get( "sv_hostname", "Nameless OpenMoHAA Battle", CVAR_SERVERINFO | CVAR_ARCHIVE );
	sv_maxclients = Cvar_Get( "sv_maxclients", "1", CVAR_SERVERINFO | CVAR_LATCH );

	sv_minRate = Cvar_Get ("sv_minRate", "0", CVAR_ARCHIVE | CVAR_SERVERINFO );
	sv_maxRate = Cvar_Get( "sv_maxRate", "0", CVAR_ARCHIVE | CVAR_SERVERINFO );
	sv_dlRate = Cvar_Get("sv_dlRate", "100", CVAR_ARCHIVE | CVAR_SERVERINFO);
	sv_minPing = Cvar_Get( "sv_minPing", "0", CVAR_ARCHIVE | CVAR_SERVERINFO );
	sv_maxPing = Cvar_Get( "sv_maxPing", "0", CVAR_ARCHIVE | CVAR_SERVERINFO );
	// Flood protect causes some annoyance like when trying to reload the weapon while checking scores
	// so it's disabled by default.
	// Since 2.0, flood protect only protect against "dmmessage" flood.
	sv_floodProtect = Cvar_Get( "sv_floodProtect", "0", CVAR_ARCHIVE | CVAR_SERVERINFO );
	sv_maplist = Cvar_Get( "sv_maplist", "", CVAR_ARCHIVE | CVAR_SERVERINFO );

	// systeminfo
	Cvar_Get( "cheats", "1", CVAR_LATCH | CVAR_SYSTEMINFO );
	sv_serverid = Cvar_Get ("sv_serverid", "0", CVAR_SYSTEMINFO | CVAR_ROM );
    // wombat: pure only makes problems at current stage
    // sv_pure is disabled by default in mohaa
	// the problem is because of difference in pak files between languages
	sv_pure = Cvar_Get ("sv_pure", "0", CVAR_SYSTEMINFO );
#ifdef USE_VOIP
	sv_voip = Cvar_Get("sv_voip", "1", CVAR_LATCH);
	Cvar_CheckRange(sv_voip, 0, 1, qtrue);
	sv_voipProtocol = Cvar_Get("sv_voipProtocol", sv_voip->integer ? "opus" : "", CVAR_SYSTEMINFO | CVAR_ROM );
#endif
	Cvar_Get ("sv_paks", "", CVAR_SYSTEMINFO | CVAR_ROM );
	Cvar_Get ("sv_pakNames", "", CVAR_SYSTEMINFO | CVAR_ROM );
	Cvar_Get ("sv_referencedPaks", "", CVAR_SYSTEMINFO | CVAR_ROM );
	Cvar_Get ("sv_referencedPakNames", "", CVAR_SYSTEMINFO | CVAR_ROM );

	// server vars
	sv_rconPassword = Cvar_Get ("rconPassword", "", CVAR_TEMP );
	sv_privatePassword = Cvar_Get ("sv_privatePassword", "", CVAR_TEMP );
	sv_fps = Cvar_Get ("sv_fps", "20", CVAR_SAVEGAME | CVAR_SERVERINFO | CVAR_NORESTART );
	sv_timeout = Cvar_Get ("sv_timeout", "120", CVAR_TEMP );
	sv_zombietime = Cvar_Get ("sv_zombietime", "2", CVAR_TEMP );
	Cvar_Get ("nextmap", "", CVAR_TEMP );

	sv_allowDownload = Cvar_Get ("sv_allowDownload", "0", CVAR_SERVERINFO);
	Cvar_Get ("sv_dlURL", "", CVAR_SERVERINFO | CVAR_ARCHIVE);
	sv_master[ 0 ] = Cvar_Get( "sv_master1", MASTER_SERVER_NAME, 0 );
	for(index = 1; index < MAX_MASTER_SERVERS; index++)
		sv_master[index] = Cvar_Get(va("sv_master%d", index + 1), "", CVAR_ARCHIVE);

	sv_reconnectlimit = Cvar_Get ("sv_reconnectlimit", "3", 0);
	sv_showloss = Cvar_Get ("sv_showloss", "0", 0);
	sv_padPackets = Cvar_Get ("sv_padPackets", "0", 0);
	sv_killserver = Cvar_Get ("sv_killserver", "0", 0);
	sv_mapChecksum = Cvar_Get ("sv_mapChecksum", "", CVAR_ROM | CVAR_SERVERINFO);
	sv_drawentities = Cvar_Get( "sv_drawentities", "1", 0 );
	sv_deeptracedebug = Cvar_Get( "sv_deeptracedebug", "0", 0 );
    sv_netprofile = Cvar_Get("sv_netprofile", "0", 0);
    sv_netprofileoverlay = Cvar_Get("sv_netprofileoverlay", "0", 0);
    sv_netoptimize = Cvar_Get("sv_netoptimize", "0", 0);
	sv_netoptimize_vistime = Cvar_Get("sv_netoptimize_vistime", "200", 0);
    g_netoptimize = Cvar_Get("g_netoptimize", "1", 0);
	sv_chatter = Cvar_Get( "sv_chatter", "0", 0 );
	sv_lanForceRate = Cvar_Get ("sv_lanForceRate", "1", CVAR_ARCHIVE );
#ifndef STANDALONE
	sv_strictAuth = Cvar_Get ("sv_strictAuth", "1", CVAR_ARCHIVE );
#endif
	sv_banFile = Cvar_Get("sv_banFile", "serverbans.dat", CVAR_ARCHIVE);

	Q_strncpyz( svs.gameName, "current", sizeof(svs.gameName) );

	// dday vars
	Cvar_Get( "g_ddayfodderguys", "0", CVAR_ARCHIVE );
	Cvar_Get( "g_ddayfog", "2", CVAR_ARCHIVE );
	Cvar_Get( "g_ddayshingleguys", "0", CVAR_ARCHIVE );
	
	// Load saved bans
	Cbuf_AddText("rehashbans\n");

    if (com_gotOriginalConfig) {
        // Added in OPM
        //  Apply config tweaks after loading the original config
		SV_ApplyOriginalConfigTweaks();
	}
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
				cl->lastSnapshotTime = 0;
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
	SV_ShutdownGamespy();
	SV_MasterShutdown();
	SV_ShutdownGameProgs();

	// free current level
	SV_ClearServer();

	// free server static data
	if(svs.clients)
	{
		int index;
		
		for(index = 0; index < sv_maxclients->integer; index++)
			SV_FreeClient(&svs.clients[index]);
		
		Z_Free(svs.clients);
	}
	Com_Memset( &svs, 0, sizeof( svs ) );

	Cvar_Set( "sv_running", "0" );
	Cvar_Set("ui_singlePlayerActive", "0");

	Com_Printf( "---------------------------\n" );

	// disconnect any local clients
	if( sv_killserver->integer != 2 )
		CL_Disconnect();
}

//
// Added in OPM
//

/*
===============
SV_PVSSoundIndex
===============
*/
int SV_PVSSoundIndex( const char *name, qboolean streamed )
{
	int		i;
	int		max = MAX_SOUNDS;
	char	*s;

	if( !name || !name[ 0 ] ) {
		return 0;
	}

	for( i = 1; i<max; i++ ) {
		s = sv.configstrings[CS_SOUNDS + i ];

		if( !s || !s[ 0 ] ) {
			SV_SetConfigstring(CS_SOUNDS + i, name);
			break;
		}
		if( !strcmp( s, name ) ) {
			break;
		}
	}

	if( i == max ) {
		int found = 0;

		// Remove existing sound indexes
		for (i = 1; i < max; i++) {
			if (svs.nonpvs_sound_cache[i].inUse) {
				if (!found) {
					found = i;
				}

				svs.nonpvs_sound_cache[i].inUse = qfalse;
				SV_SetConfigstring(CS_SOUNDS + i, NULL);
			}
		}

		if (!found) {
			return 0;
		}

		i = found;
	}

	svs.nonpvs_sound_cache[i].inUse = qtrue;
	svs.nonpvs_sound_cache[i].deleteTime = svs.time + 200;

	return i;
}

/*
=======================
SV_CacheNonPVSSound
=======================
*/
void SV_CacheNonPVSSound(void)
{
	gentity_t* ent;
	nonpvs_sound_t* npvs;
	int i, j;

	for (i = 0; i < sv.num_entities; i++) {
		ent = SV_GentityNum(i);

		for (j = 0; j < ent->r.num_nonpvs_sounds; j++) {
			npvs = &ent->r.nonpvs_sounds[j];
			if (npvs->index) {
				svs.nonpvs_sound_cache[npvs->index].inUse = qtrue;
				svs.nonpvs_sound_cache[npvs->index].deleteTime = svs.time + 200;
			}
		}
	}
}

/*
=======================
SV_CleanupNonPVSSound
=======================
*/
void SV_CleanupNonPVSSound(void)
{
	nonpvs_sound_cache_t* cache;
	int i, j, k, l;

	for (i = 1; i < MAX_SOUNDS; i++) {
		cache = &svs.nonpvs_sound_cache[i];

		if (!sv.configstrings[CS_SOUNDS + i]) {
			continue;
		}

		if (!cache->inUse) {
			continue;
		}

		if (svs.time >= cache->deleteTime) {
			SV_SetConfigstring(CS_SOUNDS + i, NULL);
			cache->inUse = qfalse;
		}
	}
}

/*
===============
SV_HandleNonPVSSound
===============
*/
void SV_HandleNonPVSSound()
{
	gentity_t *ent;
	int i;

	SV_CacheNonPVSSound();

	for (i = 0; i < sv.num_entities; i++) {
		ent = SV_GentityNum(i);
		ent->r.num_nonpvs_sounds = 0;
	}

	// Free up sound indices
	SV_CleanupNonPVSSound();
}

/*
===============
SV_ApplyOriginalConfigTweaks
===============
*/
void SV_ApplyOriginalConfigTweaks()
{
}
