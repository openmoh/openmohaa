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
// dm_team.cpp: Deathmatch Team Manager.

#include "player.h"
#include "dm_team.h"
#include "dm_manager.h"
#include "PlayerStart.h"

typedef struct spawnsort_s {
	PlayerStart *spawnpoint;
	float fMetric;
} spawnsort_t;

static qboolean SpotWouldTelefrag( float *origin )
{
	static Vector mins = Vector( -16, -16, 1 );
	static Vector maxs = Vector( 16, 16, 97 );
	trace_t trace;

	trace = G_Trace( origin, mins, maxs, origin, NULL, MASK_PLAYERSOLID, qfalse, "SpotWouldTelefrag" );

	if( trace.startsolid || trace.allsolid )
		return qtrue;
	else
		return qfalse;
}

static int compare_spawnsort( const void *pe1, const void *pe2 )
{
	float fDelta = ( ( spawnsort_t * )pe1 )->fMetric - ( ( spawnsort_t * )pe2 )->fMetric;

	if( fDelta >= -0.001f )
	{
		if( fDelta <= 0.001f )
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return 1;
	}
}

static PlayerStart *GetRandomSpawnpointFromList( spawnsort_t *pSpots, int nSpots )
{
	int i = nSpots;
	float fTotalMetric;
	float fMinPosMetric;
	float fChosen;

	if( nSpots <= 0 ) {
		return NULL;
	}

	qsort( pSpots, nSpots, sizeof( spawnsort_t ), compare_spawnsort );

	if( pSpots[ 0 ].fMetric > 0.0f )
	{
		if( nSpots > 5 ) {
			nSpots = 5;
		}

		fMinPosMetric = pSpots[ 0 ].fMetric * nSpots;
		fTotalMetric = fMinPosMetric;

		if( nSpots <= 1 )
		{
			fChosen = fMinPosMetric;
		}
		else
		{
			i = 0;
			fTotalMetric = 0.0f;

			for( i = 0; i < nSpots; i++ )
			{
				if( pSpots[ i ].fMetric <= 0.0f ) {
					break;
				}

				fChosen = pSpots[ i ].fMetric * ( nSpots - i );
				fTotalMetric += fChosen;

				i++;
			}

			if( i < nSpots )
			{
				fChosen = fMinPosMetric;
			}

			fMinPosMetric = fTotalMetric;
		}

		fTotalMetric = ( fMinPosMetric - i * fChosen * 0.90f ) * G_Random();
		for( i = 0; i < nSpots - 1; i++ )
		{
			fTotalMetric -= ( nSpots - i ) * pSpots[ i ].fMetric - ( fChosen * 0.90f );
			if( fTotalMetric <= 0.0f ) {
				break;
			}
		}

		return pSpots[ i ].spawnpoint;
	}
	else
	{
		// return the spot anyway
		return pSpots[ 0 ].spawnpoint;
	}
}

void DM_Team::Reset( void )
{
	m_spawnpoints.ClearObjectList();
	m_players.ClearObjectList();
}

void DM_Team::AddDeaths( Player *player, int numDeaths )
{
	if( level.intermissiontime || dmManager.GetTeamWin() )
		return;

	if( g_gametype->integer == GT_TEAM_ROUNDS || g_gametype->integer == GT_OBJECTIVE )
		return;

	player->AddDeaths( numDeaths );

	if( m_teamnumber > TEAM_FREEFORALL )
		m_iDeaths += numDeaths;
}

void DM_Team::AddKills( Player *player, int numKills )
{
	if( level.intermissiontime || dmManager.GetTeamWin() )
		return;

	player->AddKills( numKills );

	if( m_teamnumber > TEAM_FREEFORALL )
	{
		m_iKills += numKills;

		if( g_gametype->integer == GT_TEAM_ROUNDS || g_gametype->integer == GT_OBJECTIVE )
			player->AddDeaths( numKills );
		else
			m_teamwins += numKills;
	}
}

void DM_Team::AddPlayer( Player *player )
{
	m_players.AddObject( player );
}

void DM_Team::RemovePlayer( Player *player )
{
	m_players.RemoveObject( player );
}

void DM_Team::TeamWin( void )
{
	m_teamwins++;
	m_wins_in_a_row++;

	for( int i = 1; i <= m_players.NumObjects(); i++ )
	{
		m_players.ObjectAt( i )->WonMatch();
	}

	UpdateTeamStatus();

	if( m_teamnumber == TEAM_ALLIES )
	{
		gi.Cvar_Set( "g_scoreboardpicover", "textures/hud/allieswin" );
	}
	else if( m_teamnumber == TEAM_AXIS )
	{
		gi.Cvar_Set( "g_scoreboardpicover", "textures/hud/axiswin" );
	}
}

void DM_Team::TeamLoss( void )
{
	m_wins_in_a_row = 0;

	for( int i = 1; i <= m_players.NumObjects(); i++ )
	{
		m_players.ObjectAt( i )->LostMatch();
	}

	UpdateTeamStatus();
}

bool DM_Team::IsDead( void ) const
{
	Player *player;

	if( IsEmpty() )
	{
		if( dmManager.IsGameActive() ) {
			return true;
		}

		return false;
	}

	if( !m_bHasSpawnedPlayers ) {
		return false;
	}

	if( dmManager.AllowRespawn() ) {
		return false;
	}

	for( int i = m_players.NumObjects(); i > 0; i-- )
	{
		player = m_players.ObjectAt( i );

		if( player->IsSpectator() ) {
			continue;
		}

		if( !player->IsDead() )
		{
			return false;
		}
	}

	return true;
}

bool DM_Team::IsEmpty( void ) const
{
	return m_players.NumObjects() <= 0;
}

bool DM_Team::IsReady( void ) const
{
	return !NumNotReady();
}

int DM_Team::NumNotReady( void ) const
{
	Player *player;
	int num = 0;

	for( int i = 1; i <= m_players.NumObjects(); i++ )
	{
		player = m_players.ObjectAt( i );

		if( player->IsReady() && !player->IsDead() )
			i++;
	}

	return num;
}

int DM_Team::NumLivePlayers( void ) const
{
	Player *player;
	int num = 0;

	for( int i = 1; i <= m_players.NumObjects(); i++ )
	{
		player = m_players.ObjectAt( i );

		if( !player->IsDead() && !player->IsSpectator() )
		{
			num++;
		}
	}

	return num;
}

int DM_Team::TotalPlayersKills( void ) const
{
	Player *player;
	int iKills = 0;

	for( int i = 1; i <= m_players.NumObjects(); i++ )
	{
		player = m_players.ObjectAt( i );

		if( !player->IsDead() && !player->IsSpectator() )
		{
			iKills += player->GetNumKills();
		}
	}

	return iKills;
}

void DM_Team::TeamInvulnerable( void )
{
	for( int i = 1; i <= m_players.NumObjects(); i++ )
	{
		m_players.ObjectAt( i )->takedamage = DAMAGE_NO;
	}
}

void DM_Team::BeginFight( void )
{
	for( int i = 1; i <= m_players.NumObjects(); i++ )
	{
		m_players.ObjectAt( i )->BeginFight();
	}
}

void DM_Team::UpdateTeamStatus( void )
{
	for( int i = 1; i <= m_players.NumObjects(); i++ )
	{
		m_players.ObjectAt( i )->UpdateStatus( va( "%d wins (%d in a row)", m_teamwins, m_wins_in_a_row ) );
	}
}

float SpawnpointMetric_Ffa( const float *origin, DM_Team *dmTeam, const Player *player )
{
	float fMinEnemyDistSquared = 23170.0f * 23170.0f;
	int i;
	int nPlayers = dmManager.PlayerCount();
	float fDist;

	for( i = 1; i < nPlayers; i++ )
	{
		Player *teammate = dmManager.GetPlayer( i );
		if( teammate == player || teammate->IsDead() || teammate->IsSpectator() ) {
			continue;
		}

		fDist = ( teammate->origin - Vector( origin ) ).lengthSquared();

		if( fMinEnemyDistSquared > fDist ) {
			fMinEnemyDistSquared = fDist;
		}
	}

	return fMinEnemyDistSquared - ( G_Random( 0.25f ) + 1.0f ) * ( 1024.0f * 1024.0f );
}

float SpawnpointMetric_Team( const float *origin, DM_Team *dmTeam, const Player *player )
{
	float fMinEnemyDistSquared = 23170.0f * 23170.0f;
	float fSumFriendDistSquared = 0.0f;
	float fDistSquared;
	float fMetric;
	int i;
	int nPlayers = dmManager.PlayerCount();
	int nFriends = 0;

	for( i = 1; i < nPlayers; i++ )
	{
		Player *teammate = dmManager.GetPlayer( i );
		if( teammate == player || teammate->IsDead() || teammate->IsSpectator() ) {
			continue;
		}

		fDistSquared = ( teammate->origin - Vector( origin ) ).lengthSquared();

		if( teammate->GetDM_Team() == dmTeam )
		{
			nFriends++;
			fSumFriendDistSquared += fDistSquared;
		}
		else
		{
			if( fMinEnemyDistSquared > fDistSquared ) {
				fMinEnemyDistSquared = fDistSquared;
			}
		}
	}

	fMetric = fMinEnemyDistSquared - ( G_Random( 0.25f ) + 1.0f ) * ( 1024.0f * 1024.0f );

	if( nFriends )
	{
		fMetric += 0.25f * ( ( 23170.0f * 23170.0f ) - fSumFriendDistSquared / nFriends );
	}

	return fMetric;
}

float SpawnpointMetric_Objective( const float *origin, DM_Team *dmTeam, const Player *player )
{
	return rand() * 0.0000000005f;
}

PlayerStart *DM_Team::GetRandomFfaSpawnpoint( Player *player )
{
	return GetRandomSpawnpointWithMetric( player, SpawnpointMetric_Ffa );
}

PlayerStart *DM_Team::GetRandomTeamSpawnpoint( Player *player )
{
	return GetRandomSpawnpointWithMetric( player, SpawnpointMetric_Team );
}

PlayerStart *DM_Team::GetRandomObjectiveSpawnpoint( Player *player )
{
	return GetRandomSpawnpointWithMetric( player, SpawnpointMetric_Objective );
}

float DM_Team::PlayersRangeFromSpot( PlayerStart *spot )
{
	float bestplayerdistance = 9999999.0f;
	Vector vDist;
	int i;
	gentity_t *ent;
	Player *player;

	if( g_gametype->integer > GT_FFA ) {
		return bestplayerdistance;
	}

	// find the nearest player from the post
	for( i = 0, ent = g_entities; i < game.maxclients; i++, ent++ )
	{
		if( !ent->client || !ent->entity ) {
			continue;
		}

		player = ( Player * )ent->entity;
		if( !player->IsDead() && !player->IsSpectator() && m_teamnumber != player->GetTeam() )
		{
			vDist = spot->origin - player->origin;
			if( bestplayerdistance > vDist.length() )
				bestplayerdistance = vDist.length();
		}
	}

	return bestplayerdistance;
}

PlayerStart *DM_Team::FarthestSpawnPoint( void )
{
	int i;
	int iNumPoints;
	float bestdistance;
	float bestplayerdistance;
	PlayerStart *pSpot;
	PlayerStart *pBestSpot;
	PlayerStart *pSpot2;
	PlayerStart *pSpot3;

	bestplayerdistance = 0.0f;
	pBestSpot = NULL;
	pSpot2 = NULL;
	pSpot3 = NULL;
	iNumPoints = m_spawnpoints.NumObjects();

	for( i = 1; i <= iNumPoints; i++ )
	{
		pSpot = m_spawnpoints.ObjectAt( i );

		bestdistance = PlayersRangeFromSpot( m_spawnpoints.ObjectAt( i ) );
		if( bestdistance > bestplayerdistance )
		{
			bestplayerdistance = bestdistance;
			pSpot3 = pSpot2;
			pSpot2 = pBestSpot;
			pBestSpot = pSpot;
		}
	}

	if( pSpot3 && G_Random() < 0.2f )
	{
		return pSpot3;
	}
	else if( pSpot2 && G_Random() < 0.3f )
	{
		return pSpot2;
	}
	else
	{
		return pBestSpot;
	}
}

PlayerStart *DM_Team::GetRandomSpawnpoint( void )
{
	PlayerStart *spot = NULL;
	int numPoints = m_spawnpoints.NumObjects();

	if( numPoints )
	{
		spot = FarthestSpawnPoint();
		if( !spot )
		{
			spot = m_spawnpoints.ObjectAt( ( int )( G_Random( numPoints ) + 1.0f ) );
		}
	}
	else
	{
		warning( "DM_Team::GetRandomSpawnpoint", "No spawnpoints found\n" );
		spot = NULL;
	}

	return spot;
}

PlayerStart *DM_Team::GetRandomSpawnpointWithMetric( Player *player, float( *MetricFunction ) ( const float *origin, DM_Team *dmTeam, const Player *player ) )
{
	static float offset[ 4 ][ 3 ];
	spawnsort_t points[ 1024 ];
	PlayerStart *spot = NULL;
	int numSpots = 0;
	int iPoint = 0;

	for( int i = 1; i <= m_spawnpoints.NumObjects(); i++ )
	{
		spot = m_spawnpoints.ObjectAt( i );
		if( spot->m_bForbidSpawns || player->GetLastSpawnpoint() == spot ) {
			continue;
		}

		if( !SpotWouldTelefrag( spot->origin ) )
		{
			points[ numSpots ].spawnpoint = spot;
			points[ numSpots ].fMetric = MetricFunction( spot->origin, this, player );
			numSpots++;

			if( numSpots >= ( sizeof( points ) / sizeof( points[ 0 ] ) ) ) {
				break;
			}
		}
	}

	spot = GetRandomSpawnpointFromList( points, numSpots );
	if( spot ) {
		return spot;
	}

	numSpots = 0;

	for( int i = 1; i <= m_spawnpoints.NumObjects(); i++ )
	{
		spot = m_spawnpoints.ObjectAt( i );
		if( spot->m_bForbidSpawns ) {
			continue;
		}

		for( int j = 0; j < sizeof( offset ) / sizeof( offset[ 0 ] ); j++ )
		{
			Vector vNewSpawn = spot->origin + offset[ j ];

			if( G_SightTrace(
				spot->origin,
				player->mins,
				player->maxs,
				vNewSpawn,
				( Entity * )NULL,
				( Entity * )NULL,
				MASK_PLAYERSOLID,
				qfalse,
				"DM_Team::GetRandomSpawnpointWithMetric" ) == 1 )
			{
				Vector vEnd = vNewSpawn - Vector( 0, 0, 64 );

				trace_t trace = G_Trace(
					vNewSpawn,
					player->mins,
					player->maxs,
					vEnd,
					player,
					MASK_PLAYERSOLID,
					qfalse,
					"TempSpawnPoint" );

				if( !trace.allsolid && !trace.startsolid &&
					trace.fraction != 1.0f && trace.fraction != 1.0f &&
					trace.plane.dist >= 0.8f )
				{
					points[ numSpots ].spawnpoint = new PlayerStart;
					points[ numSpots ].spawnpoint->setOrigin( trace.endpos );
					points[ numSpots ].spawnpoint->setAngles( spot->angles );
					points[ numSpots ].fMetric = MetricFunction( vNewSpawn, this, player );

					if( numSpots >= ( sizeof( points ) / sizeof( points[ 0 ] ) ) ) {
						break;
					}
				}
			}
		}
	}

	spot = GetRandomSpawnpointFromList( points, numSpots );
	for( int i = 0; i < numSpots; i++ )
	{
		// delete all created spawnpoint
		if( points[ numSpots ].spawnpoint != spot ) {
			delete points[ numSpots ].spawnpoint;
		}
	}

	if( spot ) {
		return spot;
	}

	numSpots = 0;

	for( int i = 1; i <= m_spawnpoints.NumObjects(); i++ )
	{
		spot = m_spawnpoints.ObjectAt( i );
		if( !spot->m_bForbidSpawns && player->GetLastSpawnpoint() != spot ) {
			continue;
		}

		if( !SpotWouldTelefrag( spot->origin ) )
		{
			points[ numSpots ].spawnpoint = spot;
			points[ numSpots ].fMetric = MetricFunction( spot->origin, this, player );

			if( numSpots >= ( sizeof( points ) / sizeof( points[ 0 ] ) ) ) {
				break;
			}
		}
	}

	return GetRandomSpawnpointFromList( points, numSpots );
}

void DM_Team::InitSpawnPoints
	(
	void
	)

{
	int i;

	if( teamType == TEAM_SPECTATOR )
	{
		for( i = level.m_SimpleArchivedEntities.NumObjects(); i > 0; i-- )
		{
			PlayerStart *spawnpoint = ( PlayerStart * )level.m_SimpleArchivedEntities.ObjectAt( i );
			const char *classname = spawnpoint->getClassID();

			if( !Q_stricmp( classname, "info_player_deathmatch" ) )
			{
				m_spawnpoints.AddObject( spawnpoint );
			}
			else if( !Q_stricmp( classname, "info_player_intermission" ) )
			{
				m_spawnpoints.AddObject( spawnpoint );
			}
		}

		return;
	}

	if( g_gametype->integer <= GT_FFA )
	{
		for( i = level.m_SimpleArchivedEntities.NumObjects(); i > 0; i-- )
		{
			PlayerStart *spawnpoint = ( PlayerStart * )level.m_SimpleArchivedEntities.ObjectAt( i );
			const char *classname = spawnpoint->getClassID();

			if( !Q_stricmp( classname, "info_player_deathmatch" ) )
			{
				m_spawnpoints.AddObject( spawnpoint );
			}
		}

		if( m_spawnpoints.NumObjects() <= 1 )
		{
			// Try with team spawnpoints
			for( i = level.m_SimpleArchivedEntities.NumObjects(); i > 0; i-- )
			{
				PlayerStart *spawnpoint = ( PlayerStart * )level.m_SimpleArchivedEntities.ObjectAt( i );
				const char *classname = spawnpoint->getClassID();

				if( !Q_stricmp( classname, "info_player_allied" ) )
				{
					m_spawnpoints.AddObject( spawnpoint );
				}
				else if( !Q_stricmp( classname, "info_player_axis" ) )
				{
					m_spawnpoints.AddObject( spawnpoint );
				}
			}
		}
	}
	else if( g_gametype->integer >= GT_TEAM )
	{
		if( teamType >= TEAM_ALLIES )
		{
			for( i = level.m_SimpleArchivedEntities.NumObjects(); i > 0; i-- )
			{
				PlayerStart *spawnpoint = ( PlayerStart * )level.m_SimpleArchivedEntities.ObjectAt( i );
				const char *classname = spawnpoint->getClassID();

				if( teamType == TEAM_ALLIES && !Q_stricmp( classname, "info_player_allied" ) )
				{
					m_spawnpoints.AddObject( spawnpoint );
				}
				else if( teamType == TEAM_AXIS && !Q_stricmp( classname, "info_player_axis" ) )
				{
					m_spawnpoints.AddObject( spawnpoint );
				}
			}
		}

		if( !m_spawnpoints.NumObjects() )
		{
			// Try with info_player_deathmatch
			for( int i = level.m_SimpleArchivedEntities.NumObjects(); i > 0; i-- )
			{
				PlayerStart *spawnpoint = ( PlayerStart * )level.m_SimpleArchivedEntities.ObjectAt( i );
				const char *classname = spawnpoint->getClassID();

				if( !Q_stricmp( classname, "info_player_deathmatch" ) )
				{
					m_spawnpoints.AddObject( spawnpoint );
				}
			}
		}
	}
}

DM_Team::DM_Team()
{
	m_countdown = 0;
	m_teamwins = 0;
	m_wins_in_a_row = 0;
	m_teamnumber = -1;
	m_iKills = 0;
	m_iDeaths = 0;
	m_bHasSpawnedPlayers = false;
}

DM_Team::~DM_Team()
{

}

CLASS_DECLARATION( Listener, DM_Team, NULL )
{
	{ NULL, NULL }
};
