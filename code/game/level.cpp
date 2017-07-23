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

// level.cpp : Global Level Info.

#include "glb_local.h"
#include "level.h"
#include "parm.h"
#include "../game/navigate.h"
#include "../game/gravpath.h"

#if defined( GAME_DLL )
#include "../game/entity.h"
#include "../game/dm_manager.h"
#include "../game/player.h"
#endif

#include "scriptmaster.h"
#include "scriptvariable.h"

Level level;

gclient_t *spawn_client = NULL;

Event EV_Level_GetTime
	(
	"time",
	EV_DEFAULT,
	NULL,
	NULL,
	"current level time",
	EV_GETTER
	);

Event EV_Level_GetTotalSecrets
	(
	"total_secrets",
	EV_DEFAULT,
	NULL,
	NULL,
	"count of total secrets",
	EV_GETTER
	);
	
Event EV_Level_GetFoundSecrets
	(
	"found_secrets",
	EV_DEFAULT,
	NULL,
	NULL,
	"count of found secrets",
	EV_GETTER
	);
	
Event EV_Level_PreSpawnSentient
	(
	"prespawnsentient",
	EV_CODEONLY,
	NULL,
	NULL,
	"internal usage."
	);
	
Event EV_Level_GetAlarm
	(
	"alarm",
	EV_DEFAULT,
	NULL,
	NULL,
	"zero = global level alarm off, non-zero = alarm on",
	EV_GETTER
	);
	
Event EV_Level_SetAlarm
	(
	"alarm",
	EV_DEFAULT,
	"i",
	"alarm_status",
	"zero = global level alarm off, non-zero = alarm on",
	EV_SETTER
	);
	
Event EV_Level_SetNoDropHealth
	(
	"nodrophealth",
	EV_DEFAULT,
	"i",
	"alarm_status",
	"zero = automatically drop health according to cvars, non-zero = don't autodrop health (like hard mode)",
	EV_SETTER
	);
	
Event EV_Level_GetLoopProtection
	(
	"loop_protection",
	EV_DEFAULT,
	NULL,
	NULL,
	"says if infinite loop protection is enabled",
	EV_GETTER
	);
	
Event EV_Level_SetLoopProtection
	(
	"loop_protection",
	EV_DEFAULT,
	"i",
	"loop_protection",
	"specify if infinite loop protection is enabled",
	EV_SETTER
	);
	
Event EV_Level_GetPapersLevel
	(
	"papers",
	EV_DEFAULT,
	NULL,
	NULL,
	"the level of papers the player currently has",
	EV_GETTER
	);
	
Event EV_Level_SetPapersLevel
	(
	"papers",
	EV_DEFAULT,
	NULL,
	NULL,
	"the level of papers the player currently has",
	EV_SETTER
	);
	
Event EV_Level_GetDMRespawning
	(
	"dmrespawning",
	EV_DEFAULT,
	NULL,
	NULL,
	"returns 1 if wave-based DM, 0 if respawns are disabled within a round",
	EV_GETTER
	);
	
Event EV_Level_SetDMRespawning
	(
	"dmrespawning",
	EV_DEFAULT,
	"i",
	"allow_respawn",
	"set to 1 to turn on wave-based DM, to 0 to disable respawns within a round",
	EV_SETTER
	);
	
Event EV_Level_SetDMRespawning2
	(
	"dmrespawning",
	EV_DEFAULT,
	"i",
	"allow_respawn",
	"set to 1 to turn on wave-based DM, to 0 to disable respawns within a round"
	);
	
Event EV_Level_GetDMRoundLimit
	(
	"dmroundlimit",
	EV_DEFAULT,
	NULL,
	NULL,
	"gets the actual roundlimit, in minutes; may be 'roundlimit' cvar or the default round limit",
	EV_GETTER
	);
	
Event EV_Level_SetDMRoundLimit
	(
	"dmroundlimit",
	EV_DEFAULT,
	"i",
	"roundlimit",
	"sets the default roundlimit, in minutes; can be overridden by 'roundlimit' cvar",
	EV_SETTER
	);
	
Event EV_Level_SetDMRoundLimit2
	(
	"dmroundlimit",
	EV_DEFAULT,
	"i",
	"roundlimit",
	"sets the default roundlimit, in minutes; can be overridden by 'roundlimit' cvar"
	);
	
Event EV_Level_GetClockSide
	(
	"clockside",
	EV_DEFAULT,
	NULL,
	NULL,
	"Gets which side the clock is on... 'axis' or 'allies' win when time is up",
	EV_GETTER
	);
	
Event EV_Level_SetClockSide
	(
	"clockside",
	EV_DEFAULT,
	"s",
	"axis_or_allies",
	"Sets which side the clock is on... 'axis' or 'allies' win when time is up",
	EV_SETTER
	);
	
Event EV_Level_SetClockSide2
	(
	"clockside",
	EV_DEFAULT,
	"s",
	"axis_allies_draw_kills",
	"Sets which side the clock is on... 'axis' or 'allies' win when time is up, 'kills' gives the win to the team with more live members, 'draw' no one wins"
	);
	
Event EV_Level_GetBombPlantTeam
	(
	"planting_team",
	EV_DEFAULT,
	NULL,
	NULL,
	"Gets which is planting the bomb, 'axis' or 'allies'",
	EV_GETTER
	);
	
Event EV_Level_SetBombPlantTeam
	(
	"planting_team",
	EV_DEFAULT,
	"s",
	"axis_or_allies",
	"Sets which is planting the bomb, 'axis' or 'allies'",
	EV_SETTER
	);
	
Event EV_Level_SetBombPlantTeam2
	(
	"planting_team",
	EV_DEFAULT,
	"s",
	"axis_allies_draw_kills",
	"which is planting the bomb, 'axis' or 'allies'"
	);
	
Event EV_Level_GetTargetsToDestroy
	(
	"targets_to_destroy",
	EV_DEFAULT,
	NULL,
	NULL,
	"Gets the number of bomb targets that must be destroyed",
	EV_GETTER
	);
	
Event EV_Level_SetTargetsToDestroy
	(
	"targets_to_destroy",
	EV_DEFAULT,
	"i",
	"num",
	"Sets the number of bomb targets that must be destroyed",
	EV_SETTER
	);
	
Event EV_Level_SetTargetsToDestroy2
	(
	"targets_to_destroy",
	EV_DEFAULT,
	"i",
	"num",
	"the number of bomb targets that must be destroyed"
	);
	
Event EV_Level_GetTargetsDestroyed
	(
	"targets_destroyed",
	EV_DEFAULT,
	NULL,
	NULL,
	"Gets the number of bomb targets that have been destroyed",
	EV_GETTER
	);
	
Event EV_Level_SetTargetsDestroyed
	(
	"targets_destroyed",
	EV_DEFAULT,
	"i",
	"num",
	"Sets the number of bomb targets that have been destroyed",
	EV_SETTER
	);
	
Event EV_Level_SetTargetsDestroyed2
	(
	"targets_destroyed",
	EV_DEFAULT,
	"i",
	"num",
	"the number of bomb targets that have been destroyed"
	);
	
Event EV_Level_GetBombsPlanted
	(
	"bombs_planted",
	EV_DEFAULT,
	NULL,
	NULL,
	"Gets the number of bombs that are set",
	EV_GETTER
	);
	
Event EV_Level_SetBombsPlanted
	(
	"bombs_planted",
	EV_DEFAULT,
	"i",
	"num",
	"Sets the number of bombs that are set",
	EV_SETTER
	);
	
Event EV_Level_SetBombsPlanted2
	(
	"bombs_planted",
	EV_DEFAULT,
	"i",
	"num",
	"the number of bombs that are set"
	);
	
Event EV_Level_GetRoundBased
	(
	"roundbased",
	EV_DEFAULT,
	NULL,
	NULL,
	"Gets wether or not the game is currently round based or not",
	EV_GETTER
	);
	
Event EV_Level_Rain_Density_Set
	(
	"rain_density",
	EV_DEFAULT,
	NULL,
	NULL,
	"Sets the rain density",
	EV_SETTER
	);
	
Event EV_Level_Rain_Density_Get
	(
	"rain_density",
	EV_DEFAULT,
	NULL,
	NULL,
	"Sets the rain density",
	EV_GETTER
	);
	
Event EV_Level_Rain_Speed_Set
	(
	"rain_speed",
	EV_DEFAULT,
	NULL,
	NULL,
	"Sets the rain speed",
	3
	);
	
Event EV_Level_Rain_Speed_Get
	(
	"rain_speed",
	EV_DEFAULT,
	NULL,
	NULL,
	"Sets the rain speed",
	2
	);
	
Event EV_Level_Rain_Speed_Vary_Set
	(
	"rain_speed_vary",
	EV_DEFAULT,
	NULL,
	NULL,
	"Sets the rain speed variance",
	EV_SETTER
	);
	
Event EV_Level_Rain_Speed_Vary_Get
	(
	"rain_speed_vary",
	EV_DEFAULT,
	NULL,
	NULL,
	"Sets the rain speed variance",
	EV_GETTER
	);
	
Event EV_Level_Rain_Slant_Set
	(
	"rain_slant",
	EV_DEFAULT,
	NULL,
	NULL,
	"Sets the rain slant",
	EV_SETTER
	);
	
Event EV_Level_Rain_Slant_Get
	(
	"rain_slant",
	EV_DEFAULT,
	NULL,
	NULL,
	"Sets the rain slant",
	EV_GETTER
	);
	
Event EV_Level_Rain_Length_Set
	(
	"rain_length",
	EV_DEFAULT,
	NULL,
	NULL,
	"Sets the rain length",
	EV_SETTER
	);
	
Event EV_Level_Rain_Length_Get
	(
	"rain_length",
	EV_DEFAULT,
	NULL,
	NULL,
	"Sets the rain length",
	EV_GETTER
	);
	
Event EV_Level_Rain_Min_Dist_Set
	(
	"rain_min_dist",
	EV_DEFAULT,
	NULL,
	NULL,
	"Sets the rain min_dist",
	EV_SETTER
	);
	
Event EV_Level_Rain_Min_Dist_Get
	(
	"rain_min_dist",
	EV_DEFAULT,
	NULL,
	NULL,
	"Sets the rain min_dist",
	EV_GETTER
	);
	
Event EV_Level_Rain_Width_Set
	(
	"rain_width",
	EV_DEFAULT,
	NULL,
	NULL,
	"Sets the rain width",
	EV_SETTER
	);
	
Event EV_Level_Rain_Width_Get
	(
	"rain_width",
	EV_DEFAULT,
	NULL,
	NULL,
	"Sets the rain width",
	EV_GETTER
	);
	
Event EV_Level_Rain_Shader_Set
	(
	"rain_shader",
	EV_DEFAULT,
	NULL,
	NULL,
	"Sets the rain ",
	EV_SETTER
	);
	
Event EV_Level_Rain_Shader_Get
	(
	"rain_shader",
	EV_DEFAULT,
	NULL,
	NULL,
	"Sets the rain ",
	EV_GETTER
	);
	
Event EV_Level_Rain_NumShaders_Set
	(
	"rain_numshaders",
	EV_DEFAULT,
	NULL,
	NULL,
	"Sets the rain numshaders",
	EV_SETTER
	);
	
Event EV_Level_Rain_NumShaders_Get
	(
	"rain_numshaders",
	EV_DEFAULT,
	NULL,
	NULL,
	"Sets the rain numshaders",
	EV_GETTER
	);

Event EV_Level_GetFlags
	(
	"flag",
	EV_DEFAULT,
	NULL,
	NULL,
	"Initialized flag list",
	EV_GETTER
	);

Level::Level()
{
	Init();
}

Level::~Level()
{

}

void Level::Archive( Archiver& arc )
{
	bool prespawn;
	bool spawn;

	Listener::Archive( arc );

	if( arc.Saving() )
	{
		prespawn = classinfo()->WaitTillDefined( "prespawn" );
		spawn = classinfo()->WaitTillDefined( "spawn" );
	}

	arc.ArchiveBool( &prespawn );
	arc.ArchiveBool( &spawn );

	if( arc.Loading() )
	{
		if( prespawn ) {
			AddWaitTill( STRING_PRESPAWN );
		}

		if( spawn ) {
			AddWaitTill( STRING_SPAWN );
		}
	}

	arc.ArchiveInteger( &framenum );

	arc.ArchiveString( &level_name );
	arc.ArchiveString( &mapname );
	arc.ArchiveString( &spawnpoint );
	arc.ArchiveString( &nextmap );

	arc.ArchiveBoolean( &playerfrozen );

	arc.ArchiveFloat( &intermissiontime );
	ArchiveEnum( intermissiontype, INTTYPE_e );
	arc.ArchiveInteger( &exitintermission );

	arc.ArchiveInteger( &total_secrets );
	arc.ArchiveInteger( &found_secrets );

	arc.ArchiveFloat( &earthquake_magnitude );
	arc.ArchiveInteger( &num_earthquakes );

	for( int i = 0; i < num_earthquakes; i++ )
	{
		arc.ArchiveInteger( &earthquakes[ i ].duration );
		arc.ArchiveFloat( &earthquakes[ i ].magnitude );
		arc.ArchiveBool( &earthquakes[ i ].no_rampup );
		arc.ArchiveBool( &earthquakes[ i ].no_rampdown );

		arc.ArchiveInteger( &earthquakes[ i ].starttime );
		arc.ArchiveInteger( &earthquakes[ i ].endtime );

		arc.ArchiveSafePointer( &earthquakes[ i ].m_Thread );
	}

	arc.ArchiveBoolean( &cinematic );
	arc.ArchiveBoolean( &ai_on );
	arc.ArchiveBoolean( &mission_failed );
	arc.ArchiveBoolean( &died_already );

	arc.ArchiveVector( &water_color );
	arc.ArchiveVector( &lava_color );

	arc.ArchiveFloat( &water_alpha );
	arc.ArchiveFloat( &lava_alpha );

	arc.ArchiveString( &current_soundtrack );
	arc.ArchiveString( &saved_soundtrack );

	arc.ArchiveVector( &m_fade_color );
	arc.ArchiveFloat( &m_fade_alpha );
	arc.ArchiveFloat( &m_fade_time );
	arc.ArchiveFloat( &m_fade_time_start );
	ArchiveEnum( m_fade_style, fadestyle_t );
	ArchiveEnum( m_fade_type, fadetype_t );

	arc.ArchiveFloat( &m_letterbox_fraction );
	arc.ArchiveFloat( &m_letterbox_time );
	arc.ArchiveFloat( &m_letterbox_time_start );

	ArchiveEnum( m_letterbox_dir, letterboxdir_t );
	arc.ArchiveInteger( &m_iCuriousVoiceTime );
	arc.ArchiveInteger( &m_iAttackEntryAnimTime );
	arc.ArchiveInteger( &mHealthPopCount );

	arc.ArchiveBoolean( &m_bAlarm );
	arc.ArchiveBoolean( &mbNoDropHealth );
	arc.ArchiveInteger( &m_iPapersLevel );
	arc.ArchiveInteger( &m_LoopProtection );

	memset( skel_index, 255, sizeof( skel_index ) );

	if( arc.Loading() )
	{
		str saved = saved_soundtrack;
		ChangeSoundtrack( current_soundtrack );
		saved_soundtrack = saved;

		memset( &impact_trace, 0, sizeof( trace_t ) );
	}

	for( int i = 0; i < MAX_HEAD_SENTIENTS; i++ )
	{
		arc.ArchiveObjectPointer( ( Class ** )&m_HeadSentient[ i ] );
	}

	// FIXME: Archive Actor::mBodyQueue
	// FIXME: Archive Actor::mCurBody

	arc.ArchiveConfigString( CS_CURRENT_OBJECTIVE );

	for( int i = CS_OBJECTIVES; i < CS_OBJECTIVES + MAX_OBJECTIVES; i++ )
	{
		arc.ArchiveConfigString( i );
	}

	arc.ArchiveConfigString( CS_RAIN_DENSITY );
	arc.ArchiveConfigString( CS_RAIN_SPEED );
	arc.ArchiveConfigString( CS_RAIN_SPEEDVARY );
	arc.ArchiveConfigString( CS_RAIN_SLANT );
	arc.ArchiveConfigString( CS_RAIN_LENGTH );
	arc.ArchiveConfigString( CS_RAIN_MINDIST );
	arc.ArchiveConfigString( CS_RAIN_WIDTH );
	arc.ArchiveConfigString( CS_RAIN_SHADER );
	arc.ArchiveConfigString( CS_RAIN_NUMSHADERS );
}

void Level::Init( void )
{
	m_HeadSentient[ 1 ] = NULL;
	m_HeadSentient[ 0 ] = NULL;

	spawn_entnum = -1;

	current_map = NULL;

	framenum	= 0;
	time		= 0;
	frametime	= 0;

	level_name	= "";
	mapname		= "";
	spawnpoint	= "";
	nextmap		= "";

	total_secrets = 0;
	found_secrets = 0;

	m_iCuriousVoiceTime		= 0;
	m_iAttackEntryAnimTime	= 0;

	playerfrozen	= false;

	intermissiontime = 0.0f;
	exitintermission = 0;

	memset( &impact_trace, 0, sizeof( trace_t ) );

	cinematic		= false;
	ai_on			= true;

	near_exit		= false;
	mission_failed	= false;

	m_bAlarm		= false;
	m_iPapersLevel	= 0;

	died_already	= false;

	water_color	= vec_zero;
	lava_color	= vec_zero;

	lava_alpha	= 0.0f;
	water_alpha	= 0.0f;

	saved_soundtrack	= "";
	current_soundtrack	= "";

	automatic_cameras.ClearObjectList();

	m_fade_time_start	= 0.0f;
	m_fade_time			= -1.0f;
	m_fade_color		= vec_zero;
	m_fade_style		= additive;

	m_letterbox_dir		= letterbox_out;
	m_numArenas			= 1;

	m_fade_alpha		= 0;

	m_letterbox_fraction	= 0;
	m_letterbox_time_start	= 0;

	m_voteTime	= 0;
	m_numVoters = 0;

	m_LoopProtection	= true;
	m_LoopDrop			= true;

	m_letterbox_time	= -1.0f;

	m_voteYes	= 0;
	m_voteNo	= 0;

	m_vObjectiveLocation = vec_zero;

	svsEndTime = 0;

	earthquake_magnitude = 0;

	mHealthPopCount	= 0;
	mbNoDropHealth	= false;

	spawning		= false;

	m_bSpawnBot = false;
	m_bScriptSpawn = false;
	m_bRejectSpawn = false;
}

gentity_t *Level::AllocEdict( Entity *entity )
{
	int		   i;
	gentity_t   *edict;

	if( spawn_entnum >= 0 )
	{
		edict = &g_entities[ spawn_entnum ];
		spawn_entnum = -1;

		assert( !edict->inuse && !edict->entity );

		// free up the entity pointer in case we took one that still exists
		if( edict->inuse && edict->entity )
		{
			delete edict->entity;
		}
	}
	else
	{
		edict = &g_entities[ game.maxclients ];

		for( i = game.maxclients; i < globals.num_entities; i++, edict++ )
		{
			// the first couple seconds of server time can involve a lot of
			// freeing and allocating, so relax the replacement policy
			if(
				!edict->inuse &&
				(
				( edict->freetime < 2.0f ) ||
				( time - edict->freetime > 0.5f )
				)
				)
			{
				break;
			}
		}

		// allow two spots for none and world
		if( i == globals.max_entities - 2.0f )
		{
			// Try one more time before failing, relax timing completely

			edict = &g_entities[ game.maxclients ];

			for( i = game.maxclients; i < globals.num_entities; i++, edict++ )
			{
				if( !edict->inuse )
				{
					break;
				}
			}

			if( i == globals.max_entities - 2.0f )
			{
				gi.Error( ERR_DROP, "Level::AllocEdict: no free edicts" );
			}
		}
	}

	LL_Remove( edict, next, prev );

	InitEdict( edict );

	LL_Add( &active_edicts, edict, next, prev );

	// Tell the server about our data since we just spawned something
	if( ( edict->s.number < ENTITYNUM_WORLD ) && ( globals.num_entities <= edict->s.number ) )
	{
		globals.num_entities = edict->s.number + 1;
		gi.LocateGameData( g_entities, globals.num_entities, sizeof( gentity_t ), &game.clients[ 0 ].ps, sizeof( game.clients[ 0 ] ) );
	}

	edict->entity = entity;

	return edict;
}

void Level::FreeEdict( gentity_t *ed )
{
	gclient_t *client;

	// unlink from world
	gi.UnlinkEntity( ed );

	LL_Remove( ed, next, prev );

	client = ed->client;

	memset( ed, 0, sizeof( *ed ) );

	ed->client = client;
	ed->freetime = time;
	ed->inuse = false;
	ed->s.number = ed - g_entities;

	LL_Add( &free_edicts, ed, next, prev );
}

void Level::InitEdict( gentity_t *e )
{
	int i;

	e->inuse = true;

	e->s.renderfx |= RF_FRAMELERP;
	e->s.number = e - g_entities;

	// make sure a default scale gets set
	e->s.scale = 1.0f;
	// make sure the default constantlight gets set, initalize to r 1.0, g 1.0, b 1.0, r 0
	e->s.constantLight = 0xffffff;

	e->s.wasframe = 0;
	e->spawntime = level.time;

	for( i = 0; i < NUM_BONE_CONTROLLERS; i++ )
	{
		e->s.bone_tag[ i ] = -1;
		VectorClear( e->s.bone_angles[ i ] );
		EulerToQuat( e->s.bone_angles[ i ], e->s.bone_quat[ i ] );
	}
}

void Level::AddAutomaticCamera( Camera *cam )
{
	automatic_cameras.AddUniqueObject( cam );
}

bool Level::PreSpawned( void )
{
	return WaitTillDefined( "prespawn" );
}

bool Level::Spawned( void )
{
	return WaitTillDefined( "spawn" );
}

void Level::AddEarthquake( earthquake_t *e )
{
	if( num_earthquakes == MAX_EARTHQUAKES )
	{
		if( earthquakes[ 0 ].m_Thread ) {
			earthquakes[ 0 ].m_Thread->Wait( ( float )( e->endtime - inttime ) / 1000.0f - 0.5f );
		}

		num_earthquakes--;

		for( int i = 0; i < num_earthquakes; i++ )
		{
			earthquakes[ i ] = earthquakes[ i + 1 ];
		}
	}

	earthquakes[ num_earthquakes ] = *e;
	num_earthquakes++;

	e->m_Thread->Pause();
}

void Level::DoEarthquakes( void )
{
	int i, j;
	earthquake_t *e;
	int timedelta;
	int rampuptime;
	int rampdowntime;
	float test_magnitude;

	if( num_earthquakes <= 0 ) {
		return;
	}

	earthquake_magnitude = 0.0f;

	for( i = num_earthquakes; i > 0; i-- )
	{
		e = &earthquakes[ i - 1 ];

		if( inttime >= e->endtime || !e->m_Thread )
		{
			if( e->m_Thread ) {
				e->m_Thread->Wait( 0 );
			}

			num_earthquakes--;

			for( j = 0; j < num_earthquakes; j++ )
			{
				earthquakes[ j ] = earthquakes[ j + 1 ];
			}
		}
		else
		{
			test_magnitude = e->magnitude;

			timedelta = inttime - e->starttime;

			if( timedelta >= e->duration / 2 )
			{
				rampdowntime = 2 * e->duration / 3 + e->starttime;

				if( !e->no_rampdown && inttime > rampdowntime )
				{
					test_magnitude *= 1.0f - ( inttime - rampdowntime ) * 3.0f / e->duration;
				}
			}
			else
			{
				rampuptime = e->duration / 3 + e->starttime;

				if( !e->no_rampup && inttime < rampuptime )
				{
					test_magnitude *= ( inttime - e->starttime ) * 3.0f / e->duration;
				}
			}

			if( test_magnitude > earthquake_magnitude ) {
				earthquake_magnitude = test_magnitude;
			}
		}
	}
}

void Level::CleanUp( qboolean samemap, qboolean resetConfigStrings )
{
	DisableListenerNotify++;

	if( g_gametype->integer ) {
		dmManager.Reset();
	}

	Director.Reset( samemap );

	ClearCachedStatemaps();

	assert( active_edicts.next );
	assert( active_edicts.next->prev == &active_edicts );
	assert( active_edicts.prev );
	assert( active_edicts.prev->next == &active_edicts );
	assert( free_edicts.next );
	assert( free_edicts.next->prev == &free_edicts );
	assert( free_edicts.prev );
	assert( free_edicts.prev->next == &free_edicts );

	while( active_edicts.next != &active_edicts )
	{
		assert( active_edicts.next != &free_edicts );
		assert( active_edicts.prev != &free_edicts );

		assert( active_edicts.next );
		assert( active_edicts.next->prev == &active_edicts );
		assert( active_edicts.prev );
		assert( active_edicts.prev->next == &active_edicts );
		assert( free_edicts.next );
		assert( free_edicts.next->prev == &free_edicts );
		assert( free_edicts.prev );
		assert( free_edicts.prev->next == &free_edicts );

		if( active_edicts.next->entity )
		{
			delete active_edicts.next->entity;
		}
		else
		{
			FreeEdict( active_edicts.next );
		}
	}

	cinematic = false;
	ai_on = true;
	near_exit = false;

	mission_failed = false;
	died_already = false;

	globals.num_entities = game.maxclients + 1;
	gi.LocateGameData( g_entities, game.maxclients + 1, sizeof( gentity_t ), &game.clients[ 0 ].ps, sizeof( gclient_t ) );

	// clear up all AI node information
	PathManager.ResetNodes();

	// clear out automatic cameras
	automatic_cameras.ClearObjectList();

	// clear out level script variables
	level.Vars()->ClearList();

	// Clear out parm vars
	parm.Vars()->ClearList();

	// initialize the game variables
	// these get restored by the persistant data, so we can safely clear them here
	game.Vars()->ClearList();

	// clearout any waiting events
	L_ClearEventList();

	// reset all edicts
	ResetEdicts();

	// reset all grenade hints
	GrenadeHint::ResetHints();

	// Reset the boss health cvar
	gi.Cvar_Set( "bosshealth", "0" );

	// FIXME: Actor::ResetBodyQueue();

	if( world ) {
		world->FreeTargetList();
	}

	num_earthquakes = 0;

	classinfo()->RemoveWaitTill( "prespawn" );
	classinfo()->RemoveWaitTill( "spawn" );
	classinfo()->RemoveWaitTill( "skip" );
	classinfo()->RemoveWaitTill( "postthink" );

	if( g_gametype->integer >= GT_TEAM_ROUNDS )
	{
		classinfo()->RemoveWaitTill( "roundstart" );
		classinfo()->RemoveWaitTill( "allieswin" );
		classinfo()->RemoveWaitTill( "axiswin" );
		classinfo()->RemoveWaitTill( "draw" );
	}

	if( resetConfigStrings )
	{
		gi.SetConfigstring( CS_RAIN_DENSITY, "0" );
		gi.SetConfigstring( CS_RAIN_SPEED, "2048" );
		gi.SetConfigstring( CS_RAIN_SPEEDVARY, "512" );
		gi.SetConfigstring( CS_RAIN_SLANT, "50" );
		gi.SetConfigstring( CS_RAIN_LENGTH, "90" );
		gi.SetConfigstring( CS_RAIN_MINDIST, "512" );
		gi.SetConfigstring( CS_RAIN_WIDTH, "1" );
		gi.SetConfigstring( CS_RAIN_SHADER, "textures/rain" );
		gi.SetConfigstring( CS_RAIN_NUMSHADERS, "0" );
		gi.SetConfigstring( CS_CURRENT_OBJECTIVE, "" );

		for( int i = CS_OBJECTIVES; i < CS_OBJECTIVES + MAX_OBJECTIVES; i++ )
		{
			gi.SetConfigstring( i, "" );
		}
	}

	DisableListenerNotify--;
}

/*
==============
ResetEdicts
==============
*/
void Level::ResetEdicts( void )
{
	int i;

	memset( g_entities, 0, game.maxentities * sizeof( g_entities[ 0 ] ) );

	// Add all the edicts to the free list
	LL_Reset( &free_edicts, next, prev );
	LL_Reset( &active_edicts, next, prev );
	for( i = 0; i < game.maxentities; i++ )
	{
		LL_Add( &free_edicts, &g_entities[ i ], next, prev );
	}

	for( i = 0; i < game.maxclients; i++ )
	{
		// set client fields on player ents
		g_entities[ i ].client = game.clients + i;

		G_InitClientPersistant( &game.clients[ i ] );
	}

	globals.num_entities = game.maxclients;
}

qboolean Level::inhibitEntity( int spawnflags )

{
	if( !developer->integer && ( spawnflags & SPAWNFLAG_DEVELOPMENT ) )
	{
		return qtrue;
	}

	if( !detail->integer && ( spawnflags & SPAWNFLAG_DETAIL ) )
	{
		return qtrue;
	}

#ifdef _CONSOLE
	if( spawnflags & SPAWNFLAG_NOCONSOLE )
#else
	if( spawnflags & SPAWNFLAG_NOPC )
#endif
	{
		return qtrue;
	}

	if( g_gametype->integer )
	{
		if( spawnflags & SPAWNFLAG_NOT_DEATHMATCH )
		{
			return qtrue;
		}

		return qfalse;
	}

	switch( skill->integer )
	{
	case 0:
		return ( spawnflags & SPAWNFLAG_NOT_EASY ) != 0;
		break;

	case 1:
		return ( spawnflags & SPAWNFLAG_NOT_MEDIUM ) != 0;
		break;

	case 2:
	case 3:
		return ( spawnflags & SPAWNFLAG_NOT_HARD );
		break;
	}

	return qfalse;
}

void Level::setSkill( int value )
{
	int skill_level;

	skill_level = floor( ( float )value );
	skill_level = bound( skill_level, 0, 3 );

	gi.Cvar_Set( "skill", va( "%d", skill_level ) );
}

void Level::setTime( int levelTime )
{
	svsTime = levelTime;
	inttime = levelTime - svsStartTime;
	svsFloatTime = levelTime / 1000.0f;
	time = inttime / 1000.0f;
}

void Level::setFrametime( int frametime )
{
	intframetime = frametime;
	this->frametime = frametime / 1000.0f;
}

void Level::SetMap( const char *themapname )
{
	char *spawnpos;
	int i;
	str text;

	Init();

	spawning = true;

	// set a specific spawnpoint if the map was started with a $
	spawnpos = strchr( ( char * )themapname, '$' );
	if( spawnpos )
	{
		mapname = ( const char * )( spawnpos - themapname );
		spawnpoint = mapname;
	}
	else
	{
		mapname = themapname;
		spawnpoint = "";
	}

	current_map = ( char * )themapname;

	level_name = mapname;
	for( i = 0; i < level_name.length(); i++ )
	{
		if( level_name[ i ] == '.' )
		{
			level_name[ i ] = 0;
			break;
		}
	}

	m_mapscript = "maps/" + level_name + ".scr";
	m_precachescript = "maps/" + level_name + "_precache.scr";
	m_pathfile = "maps/" + level_name + ".pth";
	m_mapfile = "maps/" + level_name + ".bsp";
}

void Level::LoadAllScripts( const char *name, const char *extension )
{
	char **scriptFiles;
	char filename[ MAX_QPATH ];
	int numScripts;

	scriptFiles = gi.FS_ListFiles( name, extension, qfalse, &numScripts );

	if( !scriptFiles || !numScripts ) {
		return;
	}

	for( int i = 0; i < numScripts; i++ )
	{
		Com_sprintf( filename, sizeof( filename ), "%s/%s", name, scriptFiles[ i ] );

		// Compile the script
		Director.GetScript( filename );
	}

	gi.FS_FreeFileList( scriptFiles );
}

void Level::Precache( void )
{
	setTime( svsStartTime );
	setFrametime( 50 );

	if( gi.FS_ReadFile( m_precachescript, NULL, qtrue ) != -1 )
	{
		gi.DPrintf( "Adding script: '%s'\n", m_precachescript.c_str() );

		// temporarily disable the loop protection
		// because caching models require time
		m_LoopProtection = false;
		Director.ExecuteThread( m_precachescript );
		m_LoopProtection = true;
	}

	if( !g_gametype->integer ) {
		LoadAllScripts( "anim", ".scr" );
	}

	LoadAllScripts( "global", ".scr" );
}

void Level::SpawnEntities( char *entities, int svsTime )
{
	int				inhibit, simple = 0, count = 0;
	int				enttime;
	const char		*value;
	SpawnArgs		args;
	Listener		*listener;
	Entity			*ent;

	// Create the game variable list
	game.CreateVars();

	// Create the parm variable list
	parm.CreateVars();

	// Create the level variable list
	CreateVars();

	// set up time so functions still have valid times
	setTime( svsTime );
	setFrametime( 50 );

	setSkill( skill->integer );

	// reset out count of the number of game traces
	sv_numtraces = 0;

	// parse world
	entities = args.Parse( entities );
	spawn_entnum = ENTITYNUM_WORLD;
	args.SpawnInternal();

	// parse ents
	inhibit = 0;
	enttime = gi.Milliseconds();

	// Set up for a new map
	PathManager.LoadNodes();

	Com_Printf( "-------------------- Actual Spawning Entities -----------------------\n" );

	for( entities = args.Parse( entities ); entities != NULL; entities = args.Parse( entities ) )
	{
		// remove things (except the world) from different skill levels or deathmatch
		spawnflags = 0;
		value = args.getArg( "spawnflags" );

		if( value )
		{
			spawnflags = atoi( value );

			if( inhibitEntity( spawnflags ) )
			{
				inhibit++;
				continue;
			}
		}

		listener = args.SpawnInternal();

		if( listener )
		{
			simple++;

			if( listener->isSubclassOf( Entity ) )
			{
				count++;

				ent = ( Entity * )listener;
				ent->radnum = simple;

				Q_strncpyz( ent->edict->entname, ent->getClassID(), sizeof( ent->edict->entname ) );
			}
		}
	}

	Com_Printf( "-------------------- Actual Spawning Entities Done ------------------ %i ms\n", gi.Milliseconds() - enttime );

	world->UpdateConfigStrings();

	Event *ev = new Event( EV_Level_PreSpawnSentient );
	PostEvent( ev, EV_SPAWNENTITIES );

	L_ProcessPendingEvents();

	if( g_gametype->integer ) {
		dmManager.InitGame();
	}

	if( game.maxclients == 1 )
	{
		spawn_entnum = 0;
		new Player;
	}

	m_LoopProtection = false;
	AddWaitTill( STRING_PRESPAWN );
	Unregister( STRING_PRESPAWN );
	m_LoopProtection = true;

	Com_Printf( "%i entities spawned\n", count );
	Com_Printf( "%i simple entities spawned\n", simple );
	Com_Printf( "%i entities inhibited\n", inhibit );

	Com_Printf( "-------------------- Spawning Entities Done ------------------ %i ms\n", gi.Milliseconds() - enttime );
}

void Level::PreSpawnSentient( Event *ev )
{
	GameScript *script;

	// general initialization
	FindTeams();

	script = Director.GetScript( m_mapscript );
	if( script )
	{
		gi.DPrintf( "Adding script: '%s'\n", m_mapscript.c_str() );

		m_LoopProtection = false;
		Director.ExecuteThread( m_mapscript );
		m_LoopProtection = true;
	}

	PathManager.CreatePaths();
}

/*
================
FindTeams

Chain together all entities with a matching team field.
Entity teams are used for item groups and multi-entity mover groups.

All but the first will have the FL_TEAMSLAVE flag set and teammaster field set
All but the last will have the teamchain field set to the next one
================
*/
void Level::FindTeams()
{
	gentity_t	*ent, *ent2;
	Entity		*e, *e2;
	int		i, j;
	int		c, c2;

	c = 0;
	c2 = 0;
	for( i = 1, ent = g_entities + i; i < globals.num_entities; i++, ent++ )
	{
		if( !ent->inuse )
			continue;

		e = ent->entity;

		if( !e->moveteam.length() )
			continue;

		if( e->flags & FL_TEAMSLAVE )
			continue;

		e->teammaster = e;
		c++;
		c2++;

		for( j = i + 1, ent2 = ent + 1; j < globals.num_entities; j++, ent2++ )
		{
			if( !ent2->inuse )
				continue;

			e2 = ent->entity;

			if( !e2->moveteam.length() )
				continue;

			if( e2->flags & FL_TEAMSLAVE )
				continue;

			if( !strcmp( e->moveteam, e2->moveteam ) )
			{
				c2++;
				e2->teamchain = e->teamchain;
				e->teamchain = e2;
				e2->teammaster = e;
				e2->flags |= FL_TEAMSLAVE;

				// make sure that targets only point at the master
				if( e2->targetname ) {
					e->targetname = e2->targetname;
					e2->targetname = NULL;
				}
			}
		}
	}

	G_Printf( "%i teams with %i entities\n", c, c2 );
}

void Level::ServerSpawned( void )
{
	int i;
	gclient_t *client;
	gentity_t *ent;

	for( i = 0, client = game.clients; i < game.maxclients; i++, client++ )
	{
		client->ps.commandTime = svsTime;
	}

	if( !WaitTillDisabled( STRING_SPAWN ) )
	{
		AddWaitTill( STRING_SPAWN );

		Director.iPaused++;

		for( ent = active_edicts.next; ent != &active_edicts; ent = ent->next )
		{
			ent->entity->Unregister( STRING_SPAWN );
		}

		if( Director.iPaused-- == 1 ) {
			Director.ExecuteRunning();
		}

		Unregister( STRING_SPAWN );
	}
	else
	{
		Director.LoadMenus();
	}

	spawning = false;
}

void Level::CheckVote
	(
	void
	)

{
	// FIXME: show the vote HUD like in SH and BT

	if( !m_voteTime )
		return;

	if( time - m_voteTime >= 30000.0f )
	{
		dmManager.PrintAllClients( "Vote failed.\n" );
		m_voteTime = 0;
		return;
	}

	if( m_voteYes > m_numVoters / 2 )
	{
		dmManager.PrintAllClients( "Vote passed.\n" );

		// Pass arguments to console
		gi.SendConsoleCommand( va( "%s", level.m_voteString.c_str() ) );
		m_voteTime = 0;
		return;
	}

	if( m_voteNo >= m_numVoters / 2 )
	{
		dmManager.PrintAllClients( "Vote failed.\n" );
		m_voteTime = 0;
		return;
	}
}

void Level::GetTime
	(
	Event *ev
	)

{
	ev->AddFloat( level.time );
}

void Level::GetTotalSecrets
	(
	Event *ev
	)

{
	ev->AddInteger( total_secrets );
}

void Level::GetFoundSecrets
	(
	Event *ev
	)

{
	ev->AddInteger( found_secrets );
}

void Level::GetAlarm
	(
	Event *ev
	)

{
	ev->AddInteger( m_bAlarm );
}

void Level::SetAlarm
	(
	Event *ev
	)

{
	m_bAlarm = ev->GetInteger( 1 );
}

void Level::SetNoDropHealth
	(
	Event *ev
	)

{
	mbNoDropHealth = ev->GetInteger( 1 );
}

void Level::GetLoopProtection
	(
	Event *ev
	)

{
	ev->AddInteger( m_LoopProtection );
}

void Level::SetLoopProtection
	(
	Event *ev
	)

{
	m_LoopProtection = ev->GetInteger( 1 );
}

void Level::GetPapersLevel
	(
	Event *ev
	)

{
	ev->AddInteger( m_iPapersLevel );
}

void Level::SetPapersLevel
	(
	Event *ev
	)

{
	m_iPapersLevel = ev->GetInteger( 1 );
}

void Level::EventGetDMRespawning
	(
	Event *ev
	)

{
	ev->AddInteger( dmManager.GameAllowsRespawns() );
}

void Level::EventSetDMRespawning
	(
	Event *ev
	)

{
	dmManager.SetGameAllowsRespawns( ev->GetBoolean( 1 ) );
}

void Level::EventGetDMRoundLimit
	(
	Event *ev
	)

{
	ev->AddInteger( dmManager.GetRoundLimit() );
}

void Level::EventSetDMRoundLimit
	(
	Event *ev
	)

{
	int round_limit = ev->GetInteger( 1 );

	if( round_limit < 0 )
	{
		ScriptError( "round limit must be greater than 0" );
	}

	dmManager.SetDefaultRoundLimit( round_limit );
}

void Level::EventGetClockSide
	(
	Event *ev
	)

{
	ev->AddConstString( dmManager.GetClockSide() );
}

void Level::EventSetClockSide
	(
	Event *ev
	)

{
	const_str clockside = ev->GetConstString( 1 );

	if( clockside < STRING_ALLIES || clockside > STRING_KILLS)
	{
		ScriptError( "clockside must be 'axis', 'allies', 'kills', or 'draw'" );
	}

	dmManager.SetClockSide( clockside );
}

void Level::EventGetBombPlantTeam
	(
	Event *ev
	)

{
	ev->AddConstString( dmManager.GetBombPlantTeam() );
}

void Level::EventSetBombPlantTeam
	(
	Event *ev
	)

{
	const_str plant_team = ev->GetConstString( 1 );

	if( plant_team < STRING_ALLIES || plant_team > STRING_AXIS )
	{
		ScriptError( "bombplantteam must be 'axis' or 'allies'" );
	}

	dmManager.SetBombPlantTeam( plant_team );
}

void Level::EventGetTargetsToDestroy
	(
	Event *ev
	)

{
	ev->AddInteger( dmManager.GetTargetsToDestroy() );
}

void Level::EventSetTargetsToDestroy
	(
	Event *ev
	)

{
	dmManager.SetTargetsToDestroy( ev->GetInteger( 1 ) );
}

void Level::EventGetTargetsDestroyed
	(
	Event *ev
	)

{
	ev->AddInteger( dmManager.GetTargetsDestroyed() );
}

void Level::EventSetTargetsDestroyed
	(
	Event *ev
	)

{
	dmManager.SetTargetsDestroyed( ev->GetInteger( 1 ) );
}

void Level::EventGetBombsPlanted
	(
	Event *ev
	)

{
	ev->AddInteger( dmManager.GetBombsPlanted() );
}

void Level::EventSetBombsPlanted
	(
	Event *ev
	)

{
	dmManager.SetBombsPlanted( ev->GetInteger( 1 ) );
}

void Level::EventGetRoundBased
	(
	Event *ev
	)

{
	ev->AddInteger( g_gametype->integer >= GT_TEAM_ROUNDS );
}

void Level::EventRainDensitySet
	(
	Event *ev
	)

{
	gi.SetConfigstring( CS_RAIN_DENSITY, ev->GetString( 1 ) );
}

void Level::EventRainDensityGet
	(
	Event *ev
	)

{
	ev->AddString( gi.GetConfigstring( CS_RAIN_DENSITY ) );
}

void Level::EventRainSpeedSet
	(
	Event *ev
	)

{
	gi.SetConfigstring( CS_RAIN_SPEED, ev->GetString( 1 ) );
}

void Level::EventRainSpeedGet
	(
	Event *ev
	)

{
	ev->AddString( gi.GetConfigstring( CS_RAIN_SPEED ) );
}

void Level::EventRainSpeedVarySet
	(
	Event *ev
	)

{
	gi.SetConfigstring( CS_RAIN_SPEEDVARY, ev->GetString( 1 ) );
}

void Level::EventRainSpeedVaryGet
	(
	Event *ev
	)

{
	ev->AddString( gi.GetConfigstring( CS_RAIN_SPEEDVARY ) );
}

void Level::EventRainSlantSet
	(
	Event *ev
	)

{
	gi.SetConfigstring( CS_RAIN_SLANT, ev->GetString( 1 ) );
}

void Level::EventRainSlantGet
	(
	Event *ev
	)

{
	ev->AddString( gi.GetConfigstring( CS_RAIN_SLANT ) );
}

void Level::EventRainLengthSet
	(
	Event *ev
	)

{
	gi.SetConfigstring( CS_RAIN_LENGTH, ev->GetString( 1 ) );
}

void Level::EventRainLengthGet
	(
	Event *ev
	)

{
	ev->AddString( gi.GetConfigstring( CS_RAIN_LENGTH ) );
}

void Level::EventRainMin_DistSet
	(
	Event *ev
	)

{
	gi.SetConfigstring( CS_RAIN_MINDIST, ev->GetString( 1 ) );
}

void Level::EventRainMin_DistGet
	(
	Event *ev
	)

{
	ev->AddString( gi.GetConfigstring( CS_RAIN_MINDIST ) );
}

void Level::EventRainWidthSet
	(
	Event *ev
	)

{
	gi.SetConfigstring( CS_RAIN_WIDTH, ev->GetString( 1 ) );
}

void Level::EventRainWidthGet
	(
	Event *ev
	)

{
	ev->AddString( gi.GetConfigstring( CS_RAIN_WIDTH ) );
}

void Level::EventRainShaderSet
	(
	Event *ev
	)

{
	gi.SetConfigstring( CS_RAIN_SHADER, ev->GetString( 1 ) );
}

void Level::EventRainShaderGet
	(
	Event *ev
	)

{
	ev->AddString( gi.GetConfigstring( CS_RAIN_SHADER ) );
}

void Level::EventRainNumShadersSet
	(
	Event *ev
	)

{
	gi.SetConfigstring( CS_RAIN_NUMSHADERS, ev->GetString( 1 ) );
}

void Level::EventRainNumShadersGet
	(
	Event *ev
	)

{
	ev->AddString( gi.GetConfigstring( CS_RAIN_NUMSHADERS ) );
}

void Level::EventGetFlags
	(
	Event *ev
	)

{
}

CLASS_DECLARATION( Listener, Level, NULL )
{
	{ &EV_Level_GetTime, 					&Level::GetTime },
	{ &EV_Level_GetTotalSecrets, 			&Level::GetTotalSecrets },
	{ &EV_Level_GetFoundSecrets, 			&Level::GetFoundSecrets },
	{ &EV_Level_PreSpawnSentient, 			&Level::PreSpawnSentient },
	{ &EV_Level_GetAlarm, 					&Level::GetAlarm },
	{ &EV_Level_SetAlarm, 					&Level::SetAlarm },
	{ &EV_Level_SetNoDropHealth, 			&Level::SetNoDropHealth },
	{ &EV_Level_GetLoopProtection, 			&Level::GetLoopProtection },
	{ &EV_Level_SetLoopProtection, 			&Level::SetLoopProtection },
	{ &EV_Level_GetPapersLevel, 			&Level::GetPapersLevel },
	{ &EV_Level_SetPapersLevel, 			&Level::SetPapersLevel },
	{ &EV_Level_GetDMRespawning, 			&Level::EventGetDMRespawning },
	{ &EV_Level_SetDMRespawning, 			&Level::EventSetDMRespawning },
	{ &EV_Level_SetDMRespawning2, 			&Level::EventSetDMRespawning },
	{ &EV_Level_GetDMRoundLimit, 			&Level::EventGetDMRoundLimit },
	{ &EV_Level_SetDMRoundLimit, 			&Level::EventSetDMRoundLimit },
	{ &EV_Level_SetDMRoundLimit2, 			&Level::EventSetDMRoundLimit },
	{ &EV_Level_GetClockSide, 				&Level::EventGetClockSide },
	{ &EV_Level_SetClockSide, 				&Level::EventSetClockSide },
	{ &EV_Level_SetClockSide2, 				&Level::EventSetClockSide },
	{ &EV_Level_GetBombPlantTeam, 			&Level::EventGetBombPlantTeam },
	{ &EV_Level_SetBombPlantTeam, 			&Level::EventSetBombPlantTeam },
	{ &EV_Level_SetBombPlantTeam2, 			&Level::EventSetBombPlantTeam },
	{ &EV_Level_GetTargetsToDestroy, 		&Level::EventGetTargetsToDestroy },
	{ &EV_Level_SetTargetsToDestroy, 		&Level::EventSetTargetsToDestroy },
	{ &EV_Level_SetTargetsToDestroy2, 		&Level::EventSetTargetsToDestroy },
	{ &EV_Level_GetTargetsDestroyed, 		&Level::EventGetTargetsDestroyed },
	{ &EV_Level_SetTargetsDestroyed, 		&Level::EventSetTargetsDestroyed },
	{ &EV_Level_SetTargetsDestroyed2, 		&Level::EventSetTargetsDestroyed },
	{ &EV_Level_GetBombsPlanted, 			&Level::EventGetBombsPlanted },
	{ &EV_Level_SetBombsPlanted, 			&Level::EventSetBombsPlanted },
	{ &EV_Level_SetBombsPlanted2, 			&Level::EventSetBombsPlanted },
	{ &EV_Level_GetRoundBased, 				&Level::EventGetRoundBased },
	{ &EV_Level_Rain_Density_Set, 			&Level::EventRainDensitySet },
	{ &EV_Level_Rain_Density_Get, 			&Level::EventRainDensityGet },
	{ &EV_Level_Rain_Speed_Set, 			&Level::EventRainSpeedSet },
	{ &EV_Level_Rain_Speed_Get, 			&Level::EventRainSpeedGet },
	{ &EV_Level_Rain_Speed_Vary_Set, 		&Level::EventRainSpeedVarySet },
	{ &EV_Level_Rain_Speed_Vary_Get, 		&Level::EventRainSpeedVaryGet },
	{ &EV_Level_Rain_Slant_Set, 			&Level::EventRainSlantSet },
	{ &EV_Level_Rain_Slant_Get, 			&Level::EventRainSlantGet },
	{ &EV_Level_Rain_Length_Set, 			&Level::EventRainLengthSet },
	{ &EV_Level_Rain_Length_Get, 			&Level::EventRainLengthGet },
	{ &EV_Level_Rain_Min_Dist_Set, 			&Level::EventRainMin_DistSet },
	{ &EV_Level_Rain_Min_Dist_Get, 			&Level::EventRainMin_DistGet },
	{ &EV_Level_Rain_Width_Set, 			&Level::EventRainWidthSet },
	{ &EV_Level_Rain_Width_Get, 			&Level::EventRainWidthGet },
	{ &EV_Level_Rain_Shader_Set, 			&Level::EventRainShaderSet },
	{ &EV_Level_Rain_Shader_Get, 			&Level::EventRainShaderGet },
	{ &EV_Level_Rain_NumShaders_Set, 		&Level::EventRainShaderSet },
	{ &EV_Level_Rain_NumShaders_Get, 		&Level::EventRainShaderGet },
	{ &EV_Level_GetFlags,					&Level::EventGetFlags },
	{ NULL, NULL }
};
