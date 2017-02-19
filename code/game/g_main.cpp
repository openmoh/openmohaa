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

#include "g_local.h"
#include "debuglines.h"
#include "entity.h"
#include "gamecmds.h"
#include "dm_manager.h"
#include "player.h"
#include <scriptmaster.h>
#include "lodthing.h"
#include "viewthing.h"
#include "playerbot.h"

#ifdef WIN32
#include <intrin.h>
#endif

#define SAVEGAME_VERSION 80
#define PERSISTANT_VERSION 2

static char		G_ErrorMessage[ 4096 ];
profGame_t		G_profStruct;

qboolean		LoadingSavegame = false;
qboolean		LoadingServer = false;
Archiver		*currentArc = NULL;

gameExport_t	globals;
gameImport_t	gi;

gentity_t active_edicts;
gentity_t free_edicts;

int					sv_numtraces = 0;
int					sv_numpmtraces = 0;

gentity_t			*g_entities;
qboolean			g_iInThinks = 0;
qboolean			g_bBeforeThinks = qfalse;
static				float g_fMsecPerClock = 0;

usercmd_t			*current_ucmd;
usereyes_t			*current_eyeinfo;
Player				*g_pPlayer;

gclient_t			g_clients[ MAX_CLIENTS ];

void ( *SV_Error )( int type, const char *fmt, ... );
void *( *SV_Malloc )( int size );
void ( *SV_Free )( void *ptr );

void QDECL G_Printf( const char *fmt, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, fmt);
	vsprintf (text, fmt, argptr);
	va_end (argptr);

	gi.Printf( text );
}

void QDECL G_Error( const char *fmt, ... )
{
	va_list		argptr;
	char		text[ 1024 ];

	va_start( argptr, fmt );
	vsprintf( text, fmt, argptr );
	va_end( argptr );

	gi.Error( ERR_DROP, text );
}

void QDECL G_Error( int type, const char *fmt, ... )
{
	va_list		argptr;
	char		text[ 1024 ];

	va_start( argptr, fmt );
	vsprintf( text, fmt, argptr );
	va_end( argptr );

	// need to manually crash otherwise visual studio fuck up with the stack pointer...
	*( int * )0 = 0;

	assert( 0 );
	SV_Error( type, text );
}

void* G_Malloc( int size )
{
	return SV_Malloc( size );
}

void G_Free( void *ptr )
{
	int zoneId = *( int * )( ( byte * )ptr - 4 );
	int size = ( int )( ( byte * )ptr - 16 );

	assert( ptr );

	if( zoneId == 0xC057 )
		return;

	assert( zoneId == 0x2015 );
	if( zoneId != 0x2015 )
		return;

	assert( *( unsigned int * )( *( unsigned int * )size + size - 4 ) == 0x2015 );
	if( *( unsigned int * )( *( unsigned int * )size + size - 4 ) != 0x2015 )
		return;

	SV_Free( ptr );
}

/*
===============
G_ExitWithError

Calls the server's error function with the last error that occurred.
Should only be called after an exception.
===============
*/
void G_ExitWithError( const char *error )
{
	//ServerError( ERR_DROP, error );

	Q_strncpyz( G_ErrorMessage, error, sizeof( G_ErrorMessage ) );

	globals.errorMessage = G_ErrorMessage;
}

void G_RemapTeamShaders( void ) {
#ifdef MISSIONPACK
	char string[1024];
	float f = level.time * 0.001;
	Com_sprintf( string, sizeof(string), "team_icon/%s_red", g_redteam.string );
	AddRemap("textures/ctf2/redteam01", string, f);
	AddRemap("textures/ctf2/redteam02", string, f);
	Com_sprintf( string, sizeof(string), "team_icon/%s_blue", g_blueteam.string );
	AddRemap("textures/ctf2/blueteam01", string, f);
	AddRemap("textures/ctf2/blueteam02", string, f);
	gi.SetConfigstring(CS_SHADERSTATE, BuildShaderStateConfig());
#endif
}

void G_SetFogInfo(int cull, float distance, vec3_t farplane_color) {
	// cg.farplane_cull cg.farplane_distance cg.farplane_color[3]
//	gi.SetConfigstring(CS_FOGINFO,va("%i %f %f %f %f",0,4096.f,1.f,1.f,1.f));

	
}

void G_AllocGameData( void )
{
	int i;

	// de-allocate from previous level
	G_DeAllocGameData();

	// Initialize debug lines
	G_AllocDebugLines();

	// Initialize debug strings
	G_AllocDebugStrings();

	// initialize all entities for this game
	game.maxentities = maxentities->integer;

	g_entities = ( gentity_t * )gi.Malloc( game.maxentities * sizeof( g_entities[ 0 ] ) );

	// clear out the entities
	memset( g_entities, 0, sizeof( g_entities ) );
	globals.gentities = g_entities;
	globals.max_entities = game.maxentities;

	// Add all the edicts to the free list
	LL_Reset( &free_edicts, next, prev );
	LL_Reset( &active_edicts, next, prev );

	for( i = 0; i < game.maxentities; i++ )
	{
		LL_Add( &free_edicts, &g_entities[ i ], next, prev );
	}

	// initialize all clients for this game
	game.clients = ( gclient_t * )gi.Malloc( game.maxclients * sizeof( game.clients[ 0 ] ) );
	memset( game.clients, 0, game.maxclients * sizeof( game.clients[ 0 ] ) );

	for( i = 0; i < game.maxclients; i++ )
	{
		// set client fields on player ents
		g_entities[ i ].client = game.clients + i;

		G_InitClientPersistant( &game.clients[ i ] );
	}

	globals.num_entities = game.maxclients;

	// Tell the server about our data
	gi.LocateGameData( g_entities, globals.num_entities, sizeof( gentity_t ), &game.clients[ 0 ].ps, sizeof( game.clients[ 0 ] ) );
}

void G_DeAllocGameData( void )
{
	// Initialize debug lines
	G_DeAllocDebugLines();

	// free up the entities
	if( g_entities )
	{
		gi.Free( g_entities );
		g_entities = NULL;
	}

	// free up the clients
	if( game.clients )
	{
		gi.Free( game.clients );
		game.clients = NULL;
	}
}

/*
============
G_InitGame

============
*/
void G_InitGame( int levelTime, int randomSeed )
{
	G_Printf( "------- Game Initialization -------\n" );
	G_Printf( "gamename: %s\n", GAMEVERSION );
	G_Printf( "gamedate: %s\n", __DATE__ );

	srand( randomSeed );

	CVAR_Init();

	game.Vars()->ClearList();

	// set some level globals
	level.svsStartTime = levelTime;
	level.reborn = sv_reborn->integer ? true : false;
	if( level.reborn )
	{
		gi.Cvar_Set( "protocol", "9" );
	}

	G_InitConsoleCommands();

	Director.Reset();
	Actor::Init();
	PlayerBot::Init();

	sv_numtraces = 0;

	if( developer->integer && !g_gametype->integer )
	{
		Viewmodel.Init();
		LODModel.Init();
	}

	game.maxentities = maxentities->integer;
	if( game.maxclients * 8 > maxentities->integer )
	{
		game.maxentities = game.maxclients * 8;
	}

	game.maxclients = maxclients->integer + maxbots->integer;

	L_InitEvents();

	G_AllocGameData();
}

/*
============
G_SpawnEntities

============
*/
void G_SpawnEntities( char *entities, int svsTime )
{
	level.SpawnEntities( entities, svsTime );
}

/*
=================
G_ShutdownGame
=================
*/
void G_ShutdownGame() {
	gi.Printf( "==== ShutdownGame ====\n" );

	// write all the client session data so we can get it back
	G_WriteSessionData();

	level.CleanUp();

	G_DeAllocDebugLines();

/*
	if( g_entities )
	{
		gi.Free( g_entities );
		g_entities = NULL;
	}
*/

	if( game.clients )
	{
		gi.Free( game.clients );
		game.clients = NULL;
	}
}



//===================================================================

void QDECL Com_Error ( int level, const char *error, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	vsprintf (text, error, argptr);
	va_end (argptr);

	G_Error( "%s", text);
}

void QDECL Com_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	G_Printf ("%s", text);
}

/*
================
G_Precache

Calls precache scripts
================
*/
void G_Precache( void )
{
	level.Precache();
}

/*
================
G_Precache

Called when server finished initializating
================
*/
void G_ServerSpawned( void )
{
	level.ServerSpawned();
}

void G_CheckExitRules( void )
{
	if( g_gametype->integer )
	{
		if( level.intermissiontime == 0.0f )
		{
			dmManager.CheckEndMatch();
		}
		else
		{
			G_CheckIntermissionExit();
		}
	}
}

void G_CheckStartRules( void )
{
	if( ( !dmManager.IsGameActive() ) && ( !dmManager.WaitingForPlayers() ) )
	{
		dmManager.StartRound();
	}
}

/*
================
G_AddGEntity


================
*/
void G_AddGEntity( gentity_t *edict, qboolean showentnums )
{
	unsigned long long start, end;
	Entity *ent = edict->entity;

	if( g_timeents->integer )
	{
		start = rdtsc();
		G_RunEntity( ent );
		end = rdtsc();

		gi.DebugPrintf( "%d: <%s> '%s'(%d) : %d clocks, %.1f msec\n",
			level.framenum, ent->getClassname(), ent->targetname.c_str(), end - start, g_fMsecPerClock );
	}
	else
	{
		G_RunEntity( ent );
	}

	// remove the entity in case of invalid server flags
	if( ( edict->r.svFlags & SVF_NOTSINGLECLIENT ) && ( edict->r.svFlags & SVF_CAPSULE ) )
	{
		ent->PostEvent( EV_Remove, 0 );
	}

	if( showentnums )
	{
		G_DrawDebugNumber( ent->origin, ent->entnum, 2.0f, 1.0f, 1.0f, 0.0f );
	}

	if( g_entinfo->integer &&
		( g_pPlayer &&
		( edict->r.lastNetTime >= level.inttime - 200 || ent->IsSubclassOfPlayer() ) ) )
	{
		float fDist = ( g_pPlayer->centroid - g_pPlayer->EyePosition() ).length();

		if( fDist != 0.0f )
		{
			float fDot = _DotProduct( g_vEyeDir, ( g_pPlayer->centroid - g_pPlayer->EyePosition() ) );
			ent->ShowInfo( 0, fDist );
		}
	}
}

/*
================
G_RunFrame

Advances the non-player objects in the world
================
*/
void G_RunFrame( int levelTime, int frameTime )
{
	gentity_t	*edict;
	int			num;
	qboolean	showentnums;
	unsigned long long	start;
	unsigned long long	end;
	int			i;
	static		int processed[ MAX_GENTITIES ] = { 0 };
	static		int processedFrameID = 0;

	try
	{
		// exit intermissions
		if( level.exitintermission )
		{
			if( level.nextmap != level.current_map )
			{
				G_ExitLevel();
			}
			else
			{
				G_RestartLevelWithDelay( 0.1f );
				level.nextmap = "";
				level.intermissiontime = 0;
				level.exitintermission = qfalse;
			}

			return;
		}

		level.setFrametime( frameTime );
		level.setTime( levelTime );

		if( level.intermissiontime || level.died_already )
		{
			L_ProcessPendingEvents();

			for( i = 0, edict = g_entities; i < game.maxclients; i++, edict++ )
			{
				if( !edict->inuse || !edict->client || !edict->entity ) {
					continue;
				}

				edict->entity->CalcBlend();
			}

			if( g_gametype->integer && g_maxintermission->value != 0.0f )
			{
				if( level.time - level.intermissiontime > g_maxintermission->value ) {
					level.exitintermission = true;
				}
			}

			return;
		}

		if( g_scripttrace->integer ) {
			gi.DPrintf2( "====SERVER FRAME==========================================================================\n" );
		}

		g_bBeforeThinks = true;
		Director.iPaused = -1;

		// Process most of the events before the physics are run
		// so that we can affect the physics immediately
		L_ProcessPendingEvents();

		Director.iPaused = 1;
		Director.SetTime( level.inttime );

		//
		// treat each object in turn
		//
		for( edict = active_edicts.next, num = 0; edict != &active_edicts; edict = edict->next, num++ )
		{
			assert( edict );
			assert( edict->inuse );
			assert( edict->entity );

			Actor *actor = ( Actor * )edict->entity;
			if( actor->IsSubclassOfActor() )
			{
				actor->m_bUpdateAnimDoneFlags = 0;
				if( actor->m_bAnimating != 0 )
					actor->PreAnimate();
			}
		}

		Director.iPaused--;
		g_iInThinks++;

		if( !Director.iPaused )
		{
			Director.ExecuteRunning();
		}

		g_iInThinks--;

		// Process any pending events that got posted during the script code
		L_ProcessPendingEvents();

		path_checksthisframe = 0;

		// Reset debug lines
		G_InitDebugLines();
		G_InitDebugStrings();

		PathManager.ShowNodes();

		showentnums = ( sv_showentnums->integer && ( !g_gametype->integer || sv_cheats->integer ) );

		g_iInThinks++;
		processedFrameID++;

		if( g_entinfo->integer )
		{
			g_pPlayer = ( Player * )G_GetEntity( 0 );

			if( !g_pPlayer->IsSubclassOfPlayer() )
			{
				g_pPlayer = NULL;
			}
			else
			{
				Vector vAngles = g_pPlayer->GetViewAngles();
				vAngles.AngleVectorsLeft( &g_vEyeDir );
			}
		}

		if( g_timeents->integer )
		{
			g_fMsecPerClock = 1.0f / gi.Cvar_Get( "CPS", "1", 0 )->value;
			start = rdtsc();
		}

		for( edict = active_edicts.next; edict != &active_edicts; edict = edict->next )
		{
			if( edict->entity->IsSubclassOfBot() )
				G_BotThink( edict, frameTime );
		}

		for( edict = active_edicts.next; edict != &active_edicts; edict = edict->next )
		{
			num = edict->s.parent;

			if( num != ENTITYNUM_NONE )
			{
				while( 1 )
				{
					if( processed[ num ] == processedFrameID )
						break;

					processed[ num ] = processedFrameID;
					G_AddGEntity( edict, showentnums );

					if( edict->s.parent == ENTITYNUM_NONE )
						break;
				}
			}

			if( processed[ edict - g_entities ] != processedFrameID )
			{
				processed[ edict - g_entities ] = processedFrameID;
				G_AddGEntity( edict, showentnums );
			}
		}

		if( g_timeents->integer )
		{
			gi.Cvar_Set( "g_timeents", va( "%d", g_timeents->integer - 1 ) );
			end = rdtsc();

			gi.DebugPrintf( "\n%i total: %d (%.1f)\n-----------------------\n",
				level.framenum, end - start, ( float )( end - start ) * g_fMsecPerClock );
		}

		g_iInThinks--;
		g_bBeforeThinks = qfalse;

		// Process any pending events that got posted during the physics code.
		L_ProcessPendingEvents();
		level.DoEarthquakes();

		// build the playerstate_t structures for all players
		G_ClientEndServerFrames();

		level.Unregister( STRING_POSTTHINK );

		// Process any pending events that got posted during the script code
		L_ProcessPendingEvents();

		// show how many traces the game code is doing
		if( sv_traceinfo->integer )
		{
			if( sv_traceinfo->integer == 3 )
			{
				if( sv_drawtrace->integer <= 1 )
				{
					gi.DebugPrintf( "%0.2f : Total traces %3d\n", level.time, sv_numtraces );
				}
				else
				{
					gi.DebugPrintf( "%0.2f : Total traces %3d    pmove traces %3d\n", level.time, sv_numtraces, sv_numpmtraces );
				}
			}
			else
			{
				if( sv_drawtrace->integer <= 1 )
				{
					gi.DebugPrintf( "%0.2f : Total traces %3d\n", level.time, sv_numtraces );
				}
				else
				{
					gi.DebugPrintf( "%0.2f : Total traces %3d    pmove traces %3d\n", level.time, sv_numtraces, sv_numpmtraces );
				}
			}
		}

		level.framenum++;

		// reset out count of the number of game traces
		sv_numtraces = 0;
		sv_numpmtraces = 0;

		G_ClientDrawBoundingBoxes();

		G_UpdateMatchEndTime();
		G_CheckExitRules();
		G_CheckStartRules();

		gi.SetConfigstring( CS_WARMUP, va( "%.0f", dmManager.GetMatchStartTime() ) );

		if( g_gametype->integer ) {
			level.CheckVote();
		}

		if( g_animdump->integer )
		{
			for( edict = active_edicts.next; edict != &active_edicts; edict = edict->next )
			{
				Animate *anim = ( Animate * )edict->entity;

				if( anim->IsSubclassOfAnimate() )
				{
					anim->DumpAnimInfo();
				}
			}
		}
	}

	catch( const char *error )
	{
		G_ExitWithError( error );
	}
}

/*
=================
G_ClientDrawBoundingBoxes
=================
*/
void G_ClientDrawBoundingBoxes
	(
	void
	)

{
	gentity_t  *edict;
	Entity	*ent;
	Vector	eye;

	// don't show bboxes during deathmatch
	if( ( !sv_showbboxes->integer ) || ( g_gametype->integer && !sv_cheats->integer ) )
	{
		return;
	}

	if( sv_showbboxes->integer )
	{
		edict = g_entities;
		ent = edict->entity;
		if( ent )
		{
			eye = ent->origin;
			ent = findradius( NULL, eye, 1000 );
			while( ent )
			{
				ent->DrawBoundingBox( sv_showbboxes->integer );
				ent = findradius( ent, eye, 1000 );
			}
		}
	}
}

void G_PrepFrame( void )
{

}

void G_RegisterSounds( void )
{
	int startTime;
	int endTime;

	Com_Printf( "\n\n-----------PARSING UBERSOUND (SERVER)------------\n" );
	Com_Printf( "Any SetCurrentTiki errors means that tiki wasn't prefetched and tiki-specific sounds for it won't work. To fix prefetch the tiki. Ignore if you don't use that tiki on this level.\n" );

	startTime = gi.Milliseconds();
	G_Command_ProcessFile( "ubersound/ubersound.scr", qfalse );
	endTime = gi.Milliseconds();

	Com_Printf( "Parse/Load time: %f seconds.\n", ( float )( endTime - startTime ) / 1000.0 );
	Com_Printf( "-------------UBERSOUND DONE (SERVER)---------------\n\n" );
	Com_Printf( "\n\n-----------PARSING UBERDIALOG (SERVER)------------\n" );
	Com_Printf( "Any SetCurrentTiki errors means that tiki wasn't prefetched and tiki-specific sounds for it won't work. To fix prefetch the tiki. Ignore if you don't use that tiki on this level.\n" );

	startTime = gi.Milliseconds();
	G_Command_ProcessFile( "ubersound/uberdialog.scr", qfalse );
	endTime = gi.Milliseconds();

	Com_Printf( "Parse/Load time: %f seconds.\n", ( float )( endTime - startTime ) / 1000.0 );
	Com_Printf( "-------------UBERDIALOG DONE (SERVER)---------------\n\n" );
}

void G_Restart( void )
{
	G_InitWorldSession();
}

void G_SetFrameNumber( int framenum )
{
	level.frame_skel_index = framenum;
}

void G_SetMap( const char *mapname )
{
	level.SetMap( mapname );
}

void G_SetTime( int svsStartTime, int svsTime )
{
	if( level.svsStartTime != svsTime )
	{
		gi.SetConfigstring( CS_LEVEL_START_TIME, va( "%i", svsTime ) );
	}

	level.svsStartTime = svsStartTime;
	level.setTime( svsTime );
}

void G_SoundCallback( int entNum, soundChannel_t channelNumber, const char *name )
{
	gentity_t *ent = &g_entities[ entNum ];
	Entity *entity = ent->entity;

	if( !entity )
	{
		ScriptError( "ERROR:  wait on playsound only works on entities that still exist when the sound is done playing." );
	}

	entity->CancelEventsOfType( EV_SoundDone );

	Event *ev = new Event( EV_SoundDone );
	ev->AddInteger( channelNumber );
	ev->AddString( name );
	entity->PostEvent( ev, level.frametime );
}

qboolean G_AllowPaused( void )
{
	return ( !level.exitintermission ) && ( level.intermissiontime == 0.0f ) && ( !level.died_already );
}

void G_ArchiveFloat( float *fl )
{
	currentArc->ArchiveFloat( fl );
}

void G_ArchiveInteger( int *i )
{
	currentArc->ArchiveInteger( i );
}

void G_ArchiveString( char *s )
{
	if( currentArc->Loading() )
	{
		str string;
		currentArc->ArchiveString( &string );
		strcpy( s, string.c_str() );
	}
	else
	{
		str string = s;
		currentArc->ArchiveString( &string );
	}
}

void G_ArchiveSvsTime( int *pi )
{
	currentArc->ArchiveSvsTime( pi );
}

void G_ArchivePersistantData
	(
	Archiver &arc
	)

{
	gentity_t   *ed;
	int         i;

	for( i = 0; i < game.maxclients; i++ )
	{
		Entity   *ent;

		ed = &g_entities[ i ];
		if( !ed->inuse || !ed->entity )
			continue;

		ent = ed->entity;
		if( !ent->IsSubclassOfPlayer() )
			continue;
		( ( Player * )ent )->ArchivePersistantData( arc );
	}
}

void G_ArchivePersistant( const char *name, qboolean loading )
{
	int version;
	Archiver arc;

	if( loading )
	{
		if( !arc.Read( name, qfalse ) )
		{
			return;
		}

		arc.ArchiveInteger( &version );
		if( version < PERSISTANT_VERSION )
		{
			gi.Printf( "Persistant data from an older version (%d) of MOHAA.\n", version );
			arc.Close();
			return;
		}
		else if( version > PERSISTANT_VERSION )
		{
			gi.DPrintf( "Persistant data from newer version %d of MOHAA.\n", version );
			arc.Close();
			return;
		}
	}
	else
	{
		arc.Create( name );

		version = PERSISTANT_VERSION;
		arc.ArchiveInteger( &version );
	}


	arc.ArchiveObject( game.Vars() );
	G_ArchivePersistantData( arc );

	arc.Close();
	return;
}

qboolean G_ReadPersistant
	(
	const char *name
	)

{
	try
	{
		G_ArchivePersistant( name, qtrue );
	}

	catch( const char *error )
	{
		G_ExitWithError( error );
	}
	return qfalse;
}

/*
============
G_WritePersistant

This will be called whenever the game goes to a new level,

A single player death will automatically restore from the
last save position.
============
*/

void G_WritePersistant
	(
	const char *name
	)

{
	try
	{
		G_ArchivePersistant( name, qfalse );
	}

	catch( const char *error )
	{
		G_ExitWithError( error );
	}
}


/*
=================
LevelArchiveValid
=================
*/
qboolean LevelArchiveValid
	(
	Archiver &arc
	)

{
	int      version;
	int      savegame_version;

	// read the version number
	arc.ArchiveInteger( &version );
	arc.ArchiveInteger( &savegame_version );

	if( version < GAME_API_VERSION )
	{
		gi.Printf( "Savegame from an older version (%d) of MOHAA.\n", version );
		return qfalse;
	}
	else if( version > GAME_API_VERSION )
	{
		gi.Printf( "Savegame from version %d of MOHAA.\n", version );
		return qfalse;
	}

	if( savegame_version < SAVEGAME_VERSION )
	{
		gi.Printf( "Savegame from an older version (%d) of MoHAA.\n", version );
		return qfalse;
	}
	else if( savegame_version > SAVEGAME_VERSION )
	{
		gi.Printf( "Savegame from version %d of MoHAA.\n", version );
		return qfalse;
	}
	return qtrue;
}

void G_Cleanup( qboolean samemap )
{
	gi.Printf( "==== CleanupGame ====\n" );

	G_WriteSessionData();

	level.CleanUp( samemap, qtrue );
}

void ArchiveAliases
	(
	Archiver &arc
	)

{
	int i;
	byte another;
	AliasList_t *alias_list;
	AliasListNode_t *alias_node;
	str alias_name;
	str model_name;
	const char *name;
	dtikianim_t *modelanim;
	Container< dtikianim_t * > animlist;

	if( arc.Saving() )
	{
		for( i = 0; i < MAX_MODELS; i++ )
		{
			name = gi.GetConfigstring( CS_MODELS + i );
			if( name && *name && *name != '*' )
			{
				const char *p = name;

				while( p )
				{
					name = p;
					p = strchr( name, '|' );
				}

				modelanim = gi.modeltikianim( name );
				if( modelanim && !animlist.IndexOfObject( modelanim ) )
				{
					animlist.AddObject( modelanim );

					alias_list = ( AliasList_t * )modelanim->alias_list;
					if( alias_list )
					{
						alias_node = alias_list->data_list;

						if( alias_node )
						{
							another = true;
							arc.ArchiveByte( &another );

							alias_name = name;
							arc.ArchiveString( &alias_name );

							for( ; alias_node != NULL; alias_node = alias_node->next )
							{
								another = true;
								arc.ArchiveByte( &another );

								alias_name = alias_node->alias_name;
								arc.ArchiveString( &alias_name );
							}

							another = false;
							arc.ArchiveByte( &another );
						}
					}
				}
			}
		}

		another = false;
		arc.ArchiveByte( &another );
	}
	else
	{
		arc.ArchiveByte( &another );

		while( another )
		{
			arc.ArchiveString( &model_name );

			modelanim = gi.modeltikianim( model_name.c_str() );

			arc.ArchiveByte( &another );

			while( another )
			{
				// Read in aliases

				arc.ArchiveString( &alias_name );

				gi.Alias_UpdateDialog( modelanim, alias_name.c_str() );

				arc.ArchiveByte( &another );
			}

			arc.ArchiveByte( &another );
		}
	}
}


/*
=================
G_ArchiveLevel

=================
*/
qboolean G_ArchiveLevel
	(
	const char *filename,
	qboolean autosave,
	qboolean loading
	)

{
	try
	{
		int		i;
		int      num;
		Archiver arc;
		gentity_t  *edict;
		char szSaveName[ MAX_STRING_TOKENS ];
		const char *pszSaveName;
		cvar_t *cvar;

		COM_StripExtension( filename, szSaveName, sizeof( szSaveName ) );
		pszSaveName = COM_SkipPath( szSaveName );

		gi.Cvar_Set( "g_lastsave", pszSaveName );

		if( loading )
		{
			LoadingSavegame = true;

			arc.Read( filename );
			if( !LevelArchiveValid( arc ) )
			{
				arc.Close();
				return qfalse;
			}

			// Read in the pending events.  These are read in first in case
			// later objects need to post events.
			L_UnarchiveEvents( arc );
		}
		else
		{
			int temp;

			arc.Create( filename );

			// write out the version number
			temp = GAME_API_VERSION;
			arc.ArchiveInteger( &temp );
			temp = SAVEGAME_VERSION;
			arc.ArchiveInteger( &temp );

			// Write out the pending events.  These are written first in case
			// later objects need to post events when reading the archive.
			L_ArchiveEvents( arc );
		}

		if( arc.Saving() )
		{
			str s;

			num = 0;
			for( cvar = gi.NextCvar( NULL ); cvar != NULL; cvar = gi.NextCvar( cvar ) )
			{
				if( cvar->flags & CVAR_ROM ) {
					num++;
				}
			}

			arc.ArchiveInteger( &num );
			for( cvar = gi.NextCvar( NULL ); cvar != NULL; cvar = gi.NextCvar( cvar ) )
			{
				if( cvar->flags & CVAR_ROM )
				{
					s = cvar->name;
					arc.ArchiveString( &s );

					s = cvar->string;
					arc.ArchiveString( &s );

					arc.ArchiveBoolean( &cvar->modified );
					arc.ArchiveInteger( &cvar->modificationCount );
					arc.ArchiveFloat( &cvar->value );
					arc.ArchiveInteger( &cvar->integer );
				}
			}
		}
		else
		{
			str sName, sValue;

			arc.ArchiveInteger( &num );
			for( int i = 0; i < num; i++ )
			{
				arc.ArchiveString( &sName );
				arc.ArchiveString( &sValue );

				cvar = gi.cvar_set2( sName, sValue, qfalse );

				arc.ArchiveBoolean( &cvar->modified );
				arc.ArchiveInteger( &cvar->modificationCount );
				arc.ArchiveFloat( &cvar->value );
				arc.ArchiveInteger( &cvar->integer );
			}
		}

		// archive the game object
		arc.ArchiveObject( &game );

		// archive Level
		arc.ArchiveObject( &level );

		// archive camera paths
		arc.ArchiveObject( &CameraMan );

		// archive paths
		arc.ArchiveObject( &PathManager );

		// archive script controller
		arc.ArchiveObject( &Director );

		// archive lightstyles
		arc.ArchiveObject( &lightStyles );

		if( arc.Saving() )
		{
			// count the entities
			num = 0;
			for( i = 0; i < globals.num_entities; i++ )
			{
				edict = &g_entities[ i ];
				if( edict->inuse && edict->entity && !( edict->entity->flags & FL_DONTSAVE ) )
				{
					num++;
				}
			}
		}

		// archive all the entities
		arc.ArchiveInteger( &globals.num_entities );
		arc.ArchiveInteger( &num );

		if( arc.Saving() )
		{
			// write out the world
			arc.ArchiveObject( world );

			for( i = 0; i < globals.num_entities; i++ )
			{
				edict = &g_entities[ i ];
				if( !edict->inuse || !edict->entity || ( edict->entity->flags & FL_DONTSAVE ) )
				{
					continue;
				}

				arc.ArchiveObject( edict->entity );
			}
		}
		else
		{
			// Tell the server about our data
			gi.LocateGameData( g_entities, globals.num_entities, sizeof( gentity_t ), &game.clients[ 0 ].ps, sizeof( game.clients[ 0 ] ) );

			// read in the world
			arc.ReadObject();

			// FIXME: PathSearch::LoadNodes();
			//PathSearch::LoadNodes();

			for( i = 0; i < num; i++ )
			{
				arc.ReadObject();
			}
		}

		ArchiveAliases( arc );

		currentArc = &arc;
		gi.ArchiveLevel( arc.Loading() );
		currentArc = NULL;

		// FIXME: PathSearch::ArchiveDynamic();
		//PathSearch::ArchiveDynamic();

		arc.Close();

		if( arc.Loading() )
		{
			LoadingSavegame = false;
			gi.Printf( HUD_MESSAGE_YELLOW "%s\n", gi.LV_ConvertString( "Game Loaded" ) );
		}
		else
		{
			gi.Printf( HUD_MESSAGE_YELLOW "%s\n", gi.LV_ConvertString( "Game Saved" ) );
		}

		if( arc.Loading() )
		{
			// Make sure all code that needs to setup the player after they have been loaded is run

			for( i = 0; i < game.maxclients; i++ )
			{
				edict = &g_entities[ i ];

				if( edict->inuse && edict->entity )
				{
					Player *player = ( Player * )edict->entity;
					player->Loaded();
				}
			}
		}

		return qtrue;
	}

	catch( const char *error )
	{
		G_ExitWithError( error );
	}
	return qfalse;
}

/*
=================
G_WriteLevel

=================
*/
void G_WriteLevel
	(
	const char *filename,
	qboolean autosave
	)

{
	game.autosaved = autosave;
	G_ArchiveLevel( filename, autosave, qfalse );
	game.autosaved = false;
}

/*
=================
G_ReadLevel

SpawnEntities will already have been called on the
level the same way it was when the level was saved.

That is necessary to get the baselines set up identically.

The server will have cleared all of the world links before
calling ReadLevel.

No clients are connected yet.
=================
*/
qboolean G_ReadLevel
	(
	const char *filename
	)

{
	qboolean status;

	status = G_ArchiveLevel( filename, qfalse, qtrue );
	// if the level load failed make sure that these variables are not set
	if( !status )
	{
		LoadingSavegame = false;
		LoadingServer = false;
	}
	return status;
}

/*
=================
G_LevelArchiveValid
=================
*/
qboolean G_LevelArchiveValid
	(
	const char *filename
	)

{
	try
	{
		qboolean ret;

		Archiver arc;

		if( !arc.Read( filename ) )
		{
			return qfalse;
		}

		ret = LevelArchiveValid( arc );

		arc.Close();

		return ret;
	}

	catch( const char *error )
	{
		G_ExitWithError( error );
		return qfalse;
	}
}

/*
================
GetGameAPI

Gets game imports and returns game exports
================
*/
#ifndef WIN32
extern "C"
__attribute__((visibility("default")))
#endif
gameExport_t* GetGameAPI( gameImport_t *import )
{
	gi = *import;

#ifdef _DEBUG
	SV_Error						= gi.Error;
	gi.Error						= G_Error;

	//SV_Malloc						= gi.Malloc;
	///gi.Malloc						= G_Malloc;

	//SV_Free = gi.Free;
	//gi.Free							= G_Free;
#endif

	globals.apiversion				= GAME_API_VERSION;

	globals.AllowPaused				= G_AllowPaused;	
	globals.ArchiveFloat			= G_ArchiveFloat;
	globals.ArchiveInteger			= G_ArchiveInteger;
	globals.ArchivePersistant		= G_ArchivePersistant;
	globals.ArchiveString			= G_ArchiveString;
	globals.ArchiveSvsTime			= G_ArchiveSvsTime;

	globals.BotBegin				= G_BotBegin;
	globals.BotThink				= G_BotThink;

	globals.Cleanup					= G_Cleanup;

	globals.ClientCommand			= G_ClientCommand;
	globals.ClientConnect			= G_ClientConnect;

	globals.ClientBegin				= G_ClientBegin;
	globals.ClientThink				= G_ClientThink;

	globals.ClientDisconnect		= G_ClientDisconnect;
	globals.ClientThink				= G_ClientThink;

	globals.ClientUserinfoChanged	= G_ClientUserinfoChanged;

	globals.ConsoleCommand			= G_ConsoleCommand;

	globals.DebugCircle				= G_DebugCircle;
	globals.errorMessage			= NULL;


	globals.gentities				= g_entities;
	globals.gentitySize				= sizeof( g_entities );

	globals.Init					= G_InitGame;

	globals.LevelArchiveValid		= G_LevelArchiveValid;
	globals.max_entities			= sizeof( g_entities )/sizeof( g_entities[ 0 ] );
	globals.num_entities			= 0;

	globals.Precache				= G_Precache;
	globals.SpawnEntities			= G_SpawnEntities;


	globals.PrepFrame				= G_PrepFrame;

	globals.profStruct				= &G_profStruct;
	globals.ReadLevel				= G_ReadLevel;
	globals.WriteLevel				= G_WriteLevel;
	globals.RegisterSounds			= G_RegisterSounds;
	globals.Restart					= G_Restart;


	globals.RunFrame				= G_RunFrame;


	globals.ServerSpawned			= G_ServerSpawned;


	globals.SetFrameNumber			= G_SetFrameNumber;
	globals.SetMap					= G_SetMap;
	globals.SetTime					= G_SetTime;

	globals.Shutdown				= G_ShutdownGame;


	globals.SoundCallback			= G_SoundCallback;
	globals.SpawnEntities			= G_SpawnEntities;
	globals.TIKI_Orientation		= G_TIKI_Orientation;


	return &globals;
}

#ifndef WIN32

#include <signal.h>
#include <sys/mman.h>

struct sigaction origSignalActions[NSIG];

int backtrace(void **buffer, int size) {}
char **backtrace_symbols(void *const *buffer, int size) {}
void backtrace_symbols_fd(void *const *buffer, int size, int fd) {}

void resetsighandlers( void ){
	sigaction( SIGSEGV, &origSignalActions[ SIGSEGV ], NULL );
	sigaction( SIGFPE, &origSignalActions[ SIGFPE ], NULL );
	sigaction( SIGILL, &origSignalActions[ SIGILL ], NULL );
	sigaction( SIGBUS, &origSignalActions[ SIGBUS ], NULL );
	sigaction( SIGABRT, &origSignalActions[ SIGABRT ], NULL );
	sigaction( SIGSYS, &origSignalActions[ SIGSYS ], NULL );
}

void sighandler( int sig, siginfo_t *info, void *secret ) {

	void *trace[ 100 ];
	char **messages = ( char ** )NULL;
	int i, trace_size = 0;
	//ucontext_t *uc = (ucontext_t *)secret;

	char * signame = strsignal( sig );

	/* Do something useful with siginfo_t */
	if( sig == SIGSEGV ){
		printf( "Got signal %d - %s, faulty address is %p\n", sig, signame, info->si_addr );

		if( gi.Printf != NULL ){
			gi.Printf( "Got signal %d - %s, faulty address is %p\n", sig, signame, info->si_addr );
		}
	}
	else {
		if( gi.Printf != NULL ){
			gi.Printf( "Got signal %d - %s\n", sig, signame );
		}
		else {
			printf( "Got signal %d - %s\n", sig, signame );
		}
	}

	trace_size = backtrace( trace, 100 );

	/* overwrite sigaction with caller's address */
	//trace[1] = (void *) uc->uc_mcontext.gregs[REG_EIP];

	messages = ( char** )( long )backtrace_symbols( trace, trace_size );

	printf( "Execution path:\n" );

	if( gi.Printf != NULL ){
		gi.Printf( "Execution path:\n" );
	}

	for( i = 1; i<trace_size; ++i ){ /* skip first stack frame (points here) */
		printf( " --> %s\n", messages[ i ] );

		if( gi.Printf != NULL ){
			gi.Printf( " --> %s\n", messages[ i ] );
		}
	}

	resetsighandlers();  // reset original MOHAA or default signal handlers, so no more signals are handled by this handler (a signal is considered to be a fatal program error, the original implementations should exit)

	raise( sig );
}

void initsighandlers( void ){
	/* Install our signal handlers */
	struct sigaction sa;

	*( unsigned int* )&sa.sa_sigaction = ( unsigned int )sighandler;
	sigemptyset( &sa.sa_mask );
	sa.sa_flags = SA_RESTART | SA_SIGINFO;

	sigaction( SIGSEGV, &sa, &origSignalActions[ SIGSEGV ] );
	sigaction( SIGFPE, &sa, &origSignalActions[ SIGFPE ] );
	sigaction( SIGILL, &sa, &origSignalActions[ SIGILL ] );
	sigaction( SIGBUS, &sa, &origSignalActions[ SIGBUS ] );
	sigaction( SIGABRT, &sa, &origSignalActions[ SIGABRT ] );
	sigaction( SIGSYS, &sa, &origSignalActions[ SIGSYS ] );
}

void __attribute__((constructor)) load( void )
{
	initsighandlers();
}

#endif
