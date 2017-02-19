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
// dm_manager.cpp: Deathmatch Manager.

#include "player.h"
#include "dm_manager.h"
#include "dm_team.h"
#include "PlayerStart.h"

cvar_t *g_tempaxisscore;
cvar_t *g_tempaxiswinsinrow;
cvar_t *g_tempalliesscore;
cvar_t *g_tempallieswinsinrow;

DM_Manager dmManager;

Event EV_DM_Manager_DoRoundTransition
(
	"doroundtransition",
	EV_DEFAULT,
	NULL,
	NULL,
	"delayed function call to (possibly) determine round winner and restart next round"
);
Event EV_DM_Manager_FinishRoundTransition
(
	"finishroundtransition",
	EV_DEFAULT,
	NULL,
	NULL,
	"delayed function call to do the actual restart for the next round"
);

CLASS_DECLARATION( Listener, DM_Manager, NULL )
{
	{ &EV_DM_Manager_DoRoundTransition,			&DM_Manager::EventDoRoundTransition },
	{ &EV_DM_Manager_FinishRoundTransition,		&DM_Manager::EventFinishRoundTransition },
	{ NULL, NULL }
};

DM_Manager::DM_Manager()
{
	m_team_spectator.m_maxplayers = MAX_CLIENTS;
	m_team_spectator.setName( "spectator" );
	m_team_spectator.setNumber( TEAM_SPECTATOR );
	m_team_spectator.teamType = TEAM_SPECTATOR;

	m_team_freeforall.m_maxplayers = MAX_CLIENTS;
	m_team_freeforall.setName( "free-for-all" );
	m_team_freeforall.setNumber( TEAM_FREEFORALL );
	m_team_freeforall.teamType = TEAM_FREEFORALL;

	m_team_allies.m_maxplayers = MAX_CLIENTS;
	m_team_allies.setName( "allies" );
	m_team_allies.setNumber( TEAM_ALLIES );
	m_team_allies.teamType = TEAM_ALLIES;

	m_team_axis.m_maxplayers = MAX_CLIENTS;
	m_team_axis.setName( "axis" );
	m_team_axis.setNumber( TEAM_AXIS );
	m_team_axis.teamType = TEAM_AXIS;

	m_iTeamWin = 0;
	m_csTeamClockSide = STRING_AXIS;
	m_csTeamBombPlantSide = STRING_DRAW;
	m_fRoundEndTime = 0.0f;
	m_bAllowRespawns = qtrue;
	m_bRoundBasedGame = qfalse;
	m_iDefaultRoundLimit = 0;
	m_iNumTargetsToDestroy = 0;
	m_iNumTargetsDestroyed = 0;
	m_iNumBombsPlanted = 0;
}

DM_Manager::~DM_Manager()
{

}

DM_Team *DM_Manager::GetTeam( str name )
{
	if ( name.icmp( "spectator" ) == 0 ) {
		return &m_team_spectator;
	} else if ( name.icmp( "freeforall" ) == 0 ) {
		return &m_team_freeforall;
	} else if ( name.icmp( "allies" ) == 0 ) {
		return &m_team_allies;
	} else if ( name.icmp( "axis" ) == 0 ) {
		return &m_team_axis;
	} else {
		ScriptError( "Invalid team %s !\n", name.c_str() );
	}

	return NULL;
}

DM_Team *DM_Manager::GetTeam( teamtype_t team )
{
	switch( team )
	{
	case TEAM_NONE:
	case TEAM_SPECTATOR:
		return &m_team_spectator;

	case TEAM_FREEFORALL:
		return &m_team_freeforall;

	case TEAM_ALLIES:
		return &m_team_allies;

	case TEAM_AXIS:
		return &m_team_axis;

	default:
		return NULL;
	}
}

DM_Team *DM_Manager::GetTeamAllies( void )
{
	return &m_team_allies;
}

DM_Team *DM_Manager::GetTeamAxis( void )
{
	return &m_team_axis;
}

bool DM_Manager::JoinTeam( Player *player, teamtype_t teamType )
{
	DM_Team *team = player->GetDM_Team();
	DM_Team *pDMTeam = GetTeam( teamType );

	if( !pDMTeam )
	{
		return false;
	}

	if( pDMTeam->m_players.NumObjects() >= pDMTeam->m_maxplayers )
	{
		gi.centerprintf( player->edict, gi.LV_ConvertString( "That team is full" ) );
		return false;
	}

	if( team )
	{
		LeaveTeam( player );
	}

	pDMTeam->AddPlayer( player );
	AddPlayer( player );
	player->SetDM_Team( pDMTeam );

	if( teamType == TEAM_SPECTATOR ) {
		player->EndFight();
	} else {
		player->BeginFight();
	}

	return true;
}

void DM_Manager::LeaveTeam( Player *player )
{
	DM_Team *team = player->GetDM_Team();

	if( team )
	{
		if( team->m_players.IndexOfObject( player ) )
		{
			team->RemovePlayer( player );
			RemovePlayer( player );
			player->SetDM_Team( NULL );
			RebuildTeamConfigstrings();
		}
		else
		{
			warning( "DM_Manager::LeaveTeam", "Could not find team in the arena\n");
		}
	}
	else
	{
		warning( "DM_Manager::LeaveTeam", "Could not find a team for this player\n" );
	}
}

Player *DM_Manager::GetPlayer( int num )
{
	return m_players.ObjectAt( num );
}

void DM_Manager::InitGame( void )
{
	m_teams.ClearObjectList();

	m_teams.AddObject( &m_team_spectator );

	if( g_gametype->integer >= GT_TEAM )
	{
		m_teams.AddObject( &m_team_allies );
		m_teams.AddObject( &m_team_axis );
	}
	else
	{
		m_teams.AddObject( &m_team_freeforall );
	}

	m_team_spectator.InitSpawnPoints();
	m_team_allies.InitSpawnPoints();
	m_team_axis.InitSpawnPoints();
	m_team_freeforall.InitSpawnPoints();

	if( g_gametype->integer < 0 || g_gametype->integer >= GT_MAX_GAME_TYPE )
	{
		Com_Printf( "Unknown game mode" );
	}

	m_bIgnoringClockForBomb = false;
	m_fRoundTime = 0;
	m_fRoundEndTime = 0;
	m_iTeamWin = 0;
	m_iNumBombsPlanted = 0;

	if( g_gametype->integer >= 0 && g_gametype->integer < GT_MAX_GAME_TYPE )
	{
		if( g_gametype->integer <= GT_TEAM )
		{
			m_bAllowRespawns = qtrue;
			m_bRoundBasedGame = qfalse;
		}
		else
		{
			m_bAllowRespawns = qfalse;
			m_bRoundBasedGame = qtrue;

			g_tempaxisscore = gi.Cvar_Get( "g_tempaxisscore", "0", 0 );
			g_tempaxiswinsinrow = gi.Cvar_Get( "g_tempaxiswinsinrow", "0", 0 );
			g_tempalliesscore = gi.Cvar_Get( "g_tempalliesscore", "0", 0 );
			g_tempallieswinsinrow = gi.Cvar_Get( "g_tempallieswinsinrow", "0", 0 );

			m_team_axis.m_teamwins = g_tempaxisscore->integer;
			m_team_axis.m_wins_in_a_row = g_tempaxiswinsinrow->integer;
			m_team_allies.m_teamwins = g_tempalliesscore->integer;
			m_team_allies.m_wins_in_a_row = g_tempallieswinsinrow->integer;

			gi.Cvar_Set( "g_tempaxisscore", "0" );
			gi.Cvar_Set( "g_tempaxiswinsinrow", "0" );
			gi.Cvar_Set( "g_tempalliesscore", "0" );
			gi.Cvar_Set( "g_tempaxiswinsinrow", "0" );

			m_iTotalMapTime = gi.Cvar_Get( "g_tempmaptime", "0", 0 )->integer;
			gi.Cvar_Set( "g_tempmaptime", "0" );
		}
	}
}

void DM_Manager::AddPlayer( Player *player )
{
	m_players.AddUniqueObject( player );
}

void DM_Manager::RemovePlayer( Player *player )
{
	DM_Team *pDMTeam;

	m_players.RemoveObject( player );

	for( int i = m_teams.NumObjects(); i > 0; i-- )
	{
		pDMTeam = m_teams.ObjectAt( i );

		if( pDMTeam->m_players.IndexOfObject( player ) )
		{
			pDMTeam->RemovePlayer( player );

			if( !pDMTeam->m_players.NumObjects() ) {
				pDMTeam->m_bHasSpawnedPlayers = qfalse;
			}
		}
	}

	player->SetDM_Team( NULL );

	RebuildTeamConfigstrings();
}

int	DM_Manager::compareScores( const void *elem1, const void *elem2 )
{
	if( *( int * )elem1 < -1 || *( int * )elem2 < 0 )
	{
		return 0;
	}

	Player *p1 = ( Player * )G_GetEntity( *( int * )elem1 );
	Player *p2 = ( Player * )G_GetEntity( *( int * )elem2 );

	if( p1->GetNumKills() < p2->GetNumKills() )
	{
		return 1;
	}
	else if( p1->GetNumKills() == p2->GetNumKills() )
	{
		// sort by death if they have the same number of kills
		if( p1->GetNumDeaths() < p2->GetNumDeaths() )
		{
			return 1;
		}
		else if( p1->GetNumDeaths() == p2->GetNumDeaths() )
		{
			// sort by netname if they have the same number of deaths
			return Q_stricmp( p1->client->pers.netname, p2->client->pers.netname );
		}
		else if( p1->GetNumDeaths() > p2->GetNumDeaths() )
		{
			return -1;
		}
	}
	else if( p1->GetNumKills() > p2->GetNumKills() )
	{
		return -1;
	}

	// just to avoid the compiler warning
	// shouldn't go there
	return 0;
}

void DM_Manager::InsertEntry( const char *entry )
{
	size_t len = strlen( entry );

	if( scoreLength + len < MAX_STRING_CHARS )
	{
		strcpy( scoreString + scoreLength, entry );

		scoreLength += len;
		scoreEntries++;
	}
}

void DM_Manager::InsertEmpty( void )
{
	if( g_gametype->integer > GT_FFA )
	{
		InsertEntry( "-2 \"\" \"\" \"\" \"\" \"\" " );
	}
	else
	{
		InsertEntry( "-1 \"\" \"\" \"\" \"\" " );
	}
}

void DM_Manager::BuildTeamInfo( DM_Team *dmTeam )
{
	int		iPing = 0;
	int		iKills;
	int		iDeaths;
	int		iNumPlayers = 0;
	Player	*pTeamPlayer;
	char	entry[ MAX_STRING_TOKENS ];

	for( int i = iNumPlayers; i > 0; i-- )
	{
		pTeamPlayer = dmTeam->m_players.ObjectAt( i );

		if( pTeamPlayer->IsSubclassOfBot() )
		{
			continue;
		}

		iNumPlayers++;
		iPing += pTeamPlayer->client->ps.ping;
	}

	if( iNumPlayers > 0 )
	{
		iPing /= iNumPlayers;
	}

	if( g_gametype->integer >= GT_TEAM_ROUNDS )
	{
		iKills = dmTeam->m_wins_in_a_row;
		iDeaths = dmTeam->m_teamwins;
	}
	else
	{
		iKills = dmTeam->m_iKills;
		iDeaths = dmTeam->m_iDeaths;
	}

	if( g_gametype->integer > GT_FFA )
	{
		if( dmTeam->teamType > TEAM_FREEFORALL )
		{
			Com_sprintf( entry, sizeof( entry ), "%i %i %i %i \"\" %i ",
				-1,
				dmTeam->m_teamnumber,
				iKills,
				iDeaths,
				iPing
				);
		}
		else
		{
			Com_sprintf( entry, sizeof( entry ), "%i %i \"\" \"\" \"\" \"\" ",
				-1,
				dmTeam->m_teamnumber
				);
		}
	}
	else
	{
		Com_sprintf( entry, sizeof( entry ), "%i \"\" \"\" \"\" \"\" \"\" ",
			-1 - dmTeam->m_teamnumber
			);
	}

	InsertEntry( entry );
}

void DM_Manager::BuildPlayerTeamInfo( DM_Team *dmTeam, int *iPlayerList, DM_Team *ignoreTeam )
{
	char		entry[ MAX_STRING_CHARS ];
	Player		*pTeamPlayer;

	for( int i = 0; i < game.maxclients; i++ )
	{
		if( iPlayerList[ i ] == -1 ) {
			break;
		}

		pTeamPlayer = ( Player * )G_GetEntity( iPlayerList[ i ] );

		if( dmTeam != NULL && pTeamPlayer->GetDM_Team() != dmTeam )
		{
			continue;
		}

		if( ignoreTeam != NULL && pTeamPlayer->GetDM_Team() == ignoreTeam )
		{
			continue;
		}

		if( g_gametype->integer >= GT_TEAM )
		{
			Com_sprintf( entry, sizeof( entry ), "%i %i %i %i %s %s ",
				pTeamPlayer->client->ps.clientNum,
				pTeamPlayer->IsDead() ? -pTeamPlayer->GetTeam() : pTeamPlayer->GetTeam(), // negative team means death
				pTeamPlayer->GetNumKills(),
				pTeamPlayer->GetNumDeaths(),
				G_TimeString( level.svsFloatTime - pTeamPlayer->edict->client->pers.enterTime ),
				pTeamPlayer->IsSubclassOfBot() ? "bot" : va( "%d", pTeamPlayer->client->ps.ping )
				);
		}
		else
		{
			Com_sprintf( entry, sizeof( entry ), "%i %i %i %s %s ",
				pTeamPlayer->client->ps.clientNum,
				pTeamPlayer->GetNumKills(),
				pTeamPlayer->GetNumDeaths(),
				G_TimeString( level.svsFloatTime - pTeamPlayer->edict->client->pers.enterTime ),
				pTeamPlayer->IsSubclassOfBot() ? "bot" : va( "%d", pTeamPlayer->client->ps.ping )
				);
		}

		InsertEntry( entry );
	}
}

void DM_Manager::Score( Player *player )
{
	int				i, j;
	int				count = 0;
	int				stringlength = 0;
	Player			*currentPlayer;
	int				iPlayerList[ MAX_CLIENTS ];
	DM_Team			*pDMTeam;

	assert( player );

	scoreString[ 0 ] = 0;
	scoreLength = 0;
	scoreEntries = 0;

	pDMTeam = NULL;

	// make the winning team at top
	if( g_gametype->integer >= GT_TEAM_ROUNDS )
	{
		if( m_team_allies.m_teamwins <= m_team_axis.m_teamwins )
		{
			if( m_team_axis.m_teamwins > m_team_allies.m_teamwins )
			{
				pDMTeam = &m_team_axis;
			}
			else if( m_team_allies.m_wins_in_a_row > m_team_axis.m_wins_in_a_row )
			{
				pDMTeam = &m_team_allies;
			}
			else if( m_team_axis.m_wins_in_a_row <= m_team_allies.m_wins_in_a_row )
			{
				// make the player's current team at top
				pDMTeam = player->GetDM_Team();

				if( pDMTeam != &m_team_allies && pDMTeam != &m_team_axis )
				{
					pDMTeam = &m_team_allies;
				}
			}
			else
			{
				pDMTeam = &m_team_axis;
			}
		}
		else
		{
			pDMTeam = &m_team_allies;
		}
	}
	else if( g_gametype->integer > GT_FFA )
	{
		if( m_team_allies.m_iKills <= m_team_axis.m_iKills )
		{
			if( m_team_axis.m_iKills > m_team_allies.m_iKills )
			{
				pDMTeam = &m_team_axis;
			}
			else if( m_team_allies.m_iDeaths > m_team_axis.m_iDeaths )
			{
				pDMTeam = &m_team_allies;
			}
			else if( m_team_axis.m_iDeaths <= m_team_allies.m_iDeaths )
			{
				pDMTeam = player->GetDM_Team();

				if( pDMTeam != &m_team_allies && pDMTeam != &m_team_axis )
				{
					pDMTeam = &m_team_allies;
				}
			}
			else
			{
				pDMTeam = &m_team_axis;
			}
		}
		else
		{
			pDMTeam = &m_team_allies;
		}
	}

	memset( iPlayerList, -1, sizeof( iPlayerList ) );

	for( i = 1, j = 0; i <= PlayerCount(); i++ )
	{
		currentPlayer = GetPlayer( i );

		if( !currentPlayer )
			continue;

		iPlayerList[ j ] = currentPlayer->client->ps.clientNum;

		j++;
	}

	// sort players by kills
	qsort( iPlayerList, j, sizeof( int ), compareScores );

	// build team info
	if( g_gametype->integer > GT_FFA )
	{
		BuildTeamInfo( pDMTeam );
		BuildPlayerTeamInfo( pDMTeam, iPlayerList );

		// insert an empty entry to not make the scoreboard tight
		InsertEmpty();

		if( pDMTeam != &m_team_allies )
		{
			BuildTeamInfo( &m_team_allies );
			BuildPlayerTeamInfo( &m_team_allies, iPlayerList );
		}
		else if( pDMTeam != &m_team_axis )
		{
			BuildTeamInfo( &m_team_axis );
			BuildPlayerTeamInfo( &m_team_axis, iPlayerList );
		}
	}
	else
	{
		// client will only show "Players" in FFA
		// BuildTeamInfo( &m_team_freeforall );

		BuildPlayerTeamInfo( NULL, iPlayerList, &m_team_spectator );
	}

	// spectator is the last team in the scoreboard
	if( m_team_spectator.m_players.NumObjects() )
	{
		InsertEmpty();

		BuildTeamInfo( &m_team_spectator );
		BuildPlayerTeamInfo( &m_team_spectator, iPlayerList );
	}

	// send the info to the client
	gi.SendServerCommand( player->client->ps.clientNum, "scores %i %s", scoreEntries, scoreString );
}

void DM_Manager::RebuildTeamConfigstrings( void )
{
	DM_TeamPtr team;
	int teamcount;

	teamcount = m_teams.NumObjects();

	for( int i = 1; i <= teamcount; i++ )
	{
		team = m_teams.ObjectAt( i );

		gi.SetConfigstring( CS_GENERAL_STRINGS + i, va( "%d %s %d player(s)", team->m_teamnumber, team->m_teamname.c_str(), team->m_players.NumObjects() ) );
	}

	gi.SetConfigstring( CS_TEAMS, va( "%d", teamcount ) );
}

bool DM_Manager::AllowRespawn() const
{
	return m_bAllowRespawns || ( g_gametype->integer > GT_TEAM
		&& ( ( !m_team_axis.m_players.NumObjects() && !m_team_axis.m_bHasSpawnedPlayers )
		|| ( !m_team_allies.m_players.NumObjects() && !m_team_allies.m_bHasSpawnedPlayers ) ) );
}

bool DM_Manager::CheckEndMatch()
{
	if( !m_bRoundBasedGame )
	{
		if( fraglimit->integer )
		{
			if( g_gametype->integer <= GT_TEAM )
			{
				if( !PlayerHitScoreLimit() )
				{
					if( timelimit->integer && level.inttime >= 60000 * timelimit->integer )
					{
						G_BeginIntermission2();
						return true;
					}
				}
			}
			else
			{
				for( int i = 1; i <= m_teams.NumObjects(); i++ )
				{
					DM_Team *pDMTeam = m_teams.ObjectAt( i );

					if( pDMTeam->m_teamwins >= fraglimit->integer )
					{
						if( timelimit->integer && level.inttime >= 60000 * timelimit->integer )
						{
							G_BeginIntermission2();
							return true;
						}
					}
				}
			}
		}
		else
		{
			if( timelimit->integer && level.inttime >= 60000 * timelimit->integer )
			{
				G_BeginIntermission2();
				return true;
			}
		}
	}

	if( m_fRoundEndTime > 0.0f ) {
		return true;
	}

	if( m_fRoundTime > 0.0f )
	{
		qboolean bCheckWin = qfalse;

		if( fraglimit->integer )
		{
			for( int i = 1; i <= m_teams.NumObjects(); i++ )
			{
				DM_Team *pDMTeam = m_teams.ObjectAt( i );

				if( pDMTeam->m_teamwins >= fraglimit->integer )
				{
					bCheckWin = qtrue;
					break;
				}
			}
		}
		else
		{
			bCheckWin = qtrue;
		}

		if( bCheckWin )
		{
			if( !AllowRespawn() )
			{
				if( m_team_axis.IsDead() || m_team_allies.IsDead() )
				{
					if( g_gametype->integer == GT_OBJECTIVE )
					{
						if( m_csTeamBombPlantSide != STRING_DRAW )
						{
							DM_Team *pBombTeam;
							DM_Team *pNonBombTeam;

							if( m_csTeamBombPlantSide == STRING_AXIS )
							{
								pBombTeam = &m_team_axis;
								pNonBombTeam = &m_team_allies;
							}
							else
							{
								pBombTeam = &m_team_allies;
								pNonBombTeam = &m_team_axis;
							}

							if( pBombTeam->IsDead() )
							{
								if( m_iNumBombsPlanted <= 0 )
								{
									m_bRoundBasedGame = qfalse;

									if( pNonBombTeam->IsDead() && m_iNumTargetsDestroyed < m_iNumTargetsToDestroy )
									{
										TeamWin( pNonBombTeam->m_teamnumber );
									}
								}
								else
								{
									if( pNonBombTeam->IsDead() )
									{
										if( m_iNumBombsPlanted >= m_iNumTargetsToDestroy - m_iNumTargetsDestroyed )
										{
											TeamWin( pBombTeam->m_teamnumber );
										}
										else
										{
											TeamWin( pNonBombTeam->m_teamnumber );
										}
									}
									else if( m_iNumBombsPlanted >= m_iNumTargetsToDestroy - m_iNumTargetsDestroyed )
									{
										if( m_bIgnoringClockForBomb )
										{
											return false;
										}
										else
										{
											G_PrintToAllClients( "A bomb is still set!" );
											m_bIgnoringClockForBomb = true;
											return false;
										}
									}
								}
							}
						}
					}

					EndRound();
					return true;
				}
			}

			int iRoundLimit = roundlimit->integer;
			if( !iRoundLimit ) {
				iRoundLimit = m_iDefaultRoundLimit;
			}

			if( iRoundLimit <= 0 || level.time < ( 60 * iRoundLimit ) + m_fRoundTime )
			{
				return false;
			}

			if( m_csTeamBombPlantSide != STRING_DRAW )
			{
				if( m_bIgnoringClockForBomb )
				{
					if( m_iNumBombsPlanted > 0 ) {
						return false;
					}

					m_bIgnoringClockForBomb = false;
				}
				else if( m_iNumBombsPlanted > 0 )
				{
					G_PrintToAllClients( "A bomb is still set!" );
					m_bIgnoringClockForBomb = true;
					return false;
				}
			}

			if( m_csTeamClockSide == STRING_ALLIES )
			{
				TeamWin( TEAM_ALLIES );
				return true;
			}
			else if( m_csTeamClockSide == STRING_AXIS )
			{
				TeamWin( TEAM_AXIS );
				return true;
			}
			else if( m_csTeamClockSide == STRING_KILLS )
			{
				if( m_team_allies.TotalPlayersKills() > m_team_axis.TotalPlayersKills() )
				{
					TeamWin( TEAM_ALLIES );
					return true;
				}
				else if( m_team_axis.TotalPlayersKills() > m_team_allies.TotalPlayersKills() )
				{
					TeamWin( TEAM_AXIS );
				}
				else
				{
					TeamWin( TEAM_NONE );
				}
			}
			else
			{
				TeamWin( TEAM_NONE );
			}
		}

		return false;
	}

	// FIXME: TODO
	return false;
}

void DM_Manager::EndRound()
{
	if( m_fRoundEndTime <= 0.0f )
	{
		m_fRoundEndTime = level.time;
		PostEvent( EV_DM_Manager_DoRoundTransition, 2.0f );
	}
}

teamtype_t DM_Manager::GetAutoJoinTeam( void )
{
	int allies = m_team_allies.m_players.NumObjects();
	int axis = m_team_axis.m_players.NumObjects();

	if( allies < axis )
	{
		return TEAM_ALLIES;
	}
	else if( allies == axis )
	{
		return ( G_Random() >= 0.5f ) ? TEAM_ALLIES : TEAM_AXIS;
	}
	else
	{
		return TEAM_AXIS;
	}
}

const_str DM_Manager::GetBombPlantTeam( void ) const
{
	return m_csTeamBombPlantSide;
}

void DM_Manager::SetBombPlantTeam( const_str s )
{
	m_csTeamBombPlantSide = s;
}

int DM_Manager::GetBombsPlanted( void ) const
{
	return m_iNumBombsPlanted;
}

void DM_Manager::SetBombsPlanted( int num )
{
	m_iNumBombsPlanted = num;
}

const_str DM_Manager::GetClockSide( void ) const
{
	return m_csTeamClockSide;
}

void DM_Manager::SetClockSide( const_str s )
{
	m_csTeamClockSide = s;
}

float DM_Manager::GetMatchStartTime( void )
{
	if( g_gametype->integer <= GT_TEAM )
	{
		return m_fRoundTime;
	}

	if( m_fRoundTime <= 0.0f )
	{
		if( !m_team_allies.m_players.NumObjects() || !m_team_allies.m_bHasSpawnedPlayers ) {
			return -1.0f;
		}

		if( !m_team_axis.m_players.NumObjects() || !m_team_axis.m_bHasSpawnedPlayers ) {
			return -1.0f;
		}

		int num = m_team_allies.NumNotReady() + m_team_axis.NumNotReady();
		if( num > 0 ) {
			return ( float )~num;
		}
	}

	return m_fRoundTime;
}

int DM_Manager::GetRoundLimit() const
{
	int round_limit = roundlimit->integer;

	if( !round_limit ) {
		round_limit = m_iDefaultRoundLimit;
	}

	return round_limit;
}

void DM_Manager::SetDefaultRoundLimit( int roundlimit )
{
	m_iDefaultRoundLimit = roundlimit;
}

int DM_Manager::GetTargetsDestroyed( void ) const
{
	return m_iNumTargetsDestroyed;
}

void DM_Manager::SetTargetsDestroyed( int targets )
{
	m_iNumTargetsDestroyed = targets;
}

int DM_Manager::GetTargetsToDestroy( void ) const
{
	return m_iNumTargetsToDestroy;
}

void DM_Manager::SetTargetsToDestroy( int targets )
{
	m_iNumTargetsToDestroy = targets;
}

bool DM_Manager::IsGameActive( void ) const
{
	return !m_bRoundBasedGame || m_fRoundTime > 0.0f;
}

int DM_Manager::PlayerCount( void ) const
{
	return m_players.NumObjects();
}

bool DM_Manager::PlayerHitScoreLimit( void )
{
	if( PlayerCount() < 1 )
	{
		return false;
	}

	if( g_gametype->integer >= GT_TEAM_ROUNDS )
	{
		return false;
	}

	for( int i = 1; i <= PlayerCount(); i++ )
	{
		if( m_players.ObjectAt( i )->GetNumKills() >= fraglimit->integer )
		{
			return true;
		}
	}

	return false;
}

void DM_Manager::PlayerKilled( Player *player )
{
	// Spawn a deadbody
	player->DeadBody();

	// Hide the model because of the dead body
	player->hideModel();

	// Don't let the player die a second time
	player->takedamage = DAMAGE_NO;

	CheckEndMatch();
}

void DM_Manager::PrintAllClients( str s )
{
	gentity_t *ent;
	int i;
	Player *player;

	if( game.maxclients <= 0 )
	{
		return;
	}

	for( i = 0, ent = g_entities; i < game.maxclients; i++, ent++ )
	{
		if( !ent->inuse || !ent->client || !ent->entity ) {
			continue;
		}

		player = ( Player * )ent->entity;
		player->HUDPrint( s );
	}
}

void DM_Manager::Reset( void )
{
	m_team_allies.Reset();
	m_team_axis.Reset();
	m_team_spectator.Reset();
	m_team_freeforall.Reset();

	m_players.ClearObjectList();
	m_teams.ClearObjectList();

	gi.Cvar_Set( "g_scoreboardpicover", "" );
}

void DM_Manager::StartRound( void )
{
	gentity_t *ent;
	int i;
	Player *player;

	m_fRoundTime = level.time;
	if( m_fRoundTime < 0.1f ) {
		m_fRoundTime = 0.1f;
	}

	m_fRoundEndTime = 0.0f;

	// respawn all players
	for( i = 0, ent = g_entities; i < game.maxclients; i++, ent++ )
	{
		if( !ent->inuse || !ent->client || !ent->entity ) {
			continue;
		}

		player = ( Player * )ent->entity;

		if( ( player->GetTeam() == TEAM_ALLIES ||
			player->GetTeam() == TEAM_AXIS )
			&& !player->IsDead()
			&& !player->IsSpectator() )
		{
			player->PostEvent( EV_Player_Respawn, 0 );
		}
	}

	level.Unregister( "roundstart" );
	gi.SetConfigstring( CS_WARMUP, va( "%.0f", GetMatchStartTime() ) );
}

bool DM_Manager::TeamHitScoreLimit( void )
{
	if( m_teams.NumObjects() < 1 )
	{
		return false;
	}

	for( int i = 1; i <= m_teams.NumObjects(); i++ )
	{
		if( m_teams.ObjectAt( i )->m_iKills >= fraglimit->integer )
		{
			return true;
		}
	}

	return false;
}


int DM_Manager::GetTeamWin( void )
{
	return m_iTeamWin;
}

void DM_Manager::TeamWin( int teamnum )
{
	DM_Team *pTeamWin;
	DM_Team *pTeamLose;

	if( m_iTeamWin ) {
		return;
	}

	if( teamnum == TEAM_AXIS )
	{
		pTeamWin = &m_team_axis;
		pTeamLose = &m_team_allies;
	}
	else if( teamnum == TEAM_ALLIES )
	{
		pTeamWin = &m_team_allies;
		pTeamLose = &m_team_axis;
	}
	else
	{
		pTeamWin = NULL;
		pTeamLose = NULL;
	}

	if( pTeamWin )
	{
		pTeamWin->TeamWin();
	}

	if( pTeamLose )
	{
		pTeamLose->TeamLoss();
	}

	m_iTeamWin = teamnum ? teamnum : -1;
	EndRound();
}

bool DM_Manager::WaitingForPlayers( void ) const
{
	if( g_gametype->integer <= GT_TEAM ) {
		return false;
	}

	if( !m_team_axis.m_players.NumObjects() || !m_team_axis.m_bHasSpawnedPlayers )
	{
		return true;
	}
	else if( !m_team_allies.m_players.NumObjects() || !m_team_allies.m_bHasSpawnedPlayers )
	{
		return true;
	}
	else if( !g_forceready->integer && m_team_axis.NumNotReady() )
	{
		return true;
	}
	else if( !g_forceready->integer && m_team_allies.NumNotReady() )
	{
		return true;
	}
	else if( m_team_axis.IsDead() || m_team_allies.IsDead() )
	{
		return true;
	}

	return false;
}

void DM_Manager::EventDoRoundTransition( Event *ev )
{
	if( !m_iTeamWin )
	{
		if( G_FindClass( NULL, "projectile" ) )
		{
			// wait for any projectile to explode
			PostEvent( EV_DM_Manager_DoRoundTransition, 1.0f, 0 );
			return;
		}

		if( !m_team_allies.IsDead() && !m_team_axis.IsDead() )
		{
			if( m_csTeamClockSide != STRING_KILLS )
			{
				if( m_csTeamClockSide != STRING_DRAW )
					gi.Printf( "WARNING: DM_Manager::EventDoRoundTransition received but no winner could be determined when there should've been\n" );

				TeamWin( TEAM_NONE );
			}
			else
			{
				if( m_team_allies.TotalPlayersKills() > m_team_axis.TotalPlayersKills() )
				{
					TeamWin( TEAM_ALLIES );
				}
				else if( m_team_axis.TotalPlayersKills() > m_team_allies.TotalPlayersKills() )
				{
					TeamWin( TEAM_AXIS );
				}
				else
				{
					TeamWin( TEAM_NONE );
				}
			}
		}
		else if( m_team_allies.IsDead() && m_team_axis.IsDead() )
		{
			TeamWin( TEAM_NONE );
		}
		else if( m_team_axis.IsDead() )
		{
			TeamWin( TEAM_ALLIES );
		}
		else
		{
			TeamWin( TEAM_AXIS );
		}
	}

	if( m_iTeamWin == TEAM_AXIS )
	{
		G_CenterPrintToAllClients( va( "\n\n\n%s\n", gi.LV_ConvertString( "Axis win!\n" ) ) );
		G_PrintToAllClients( va( "\n\n\n%s\n", gi.LV_ConvertString( "Axis win!\n" ) ) );

		// Play the axis victory sound
		world->Sound( "den_victory_v" );

		Unregister( STRING_AXISWIN );
	}
	else if( m_iTeamWin == TEAM_ALLIES )
	{
		G_CenterPrintToAllClients( va( "\n\n\n%s\n", gi.LV_ConvertString( "Allies win!\n" ) ) );
		G_PrintToAllClients( va( "\n\n\n%s\n", gi.LV_ConvertString( "Allies win!\n" ) ) );

		// Play the allies victory sound
		world->Sound( "dfr_victory_v" );

		Unregister( STRING_ALLIESWIN );
	}
	else
	{
		G_CenterPrintToAllClients( va( "\n\n\n%s\n", gi.LV_ConvertString( "It's a draw!\n" ) ) );
		G_PrintToAllClients( va( "\n\n\n%s\n", gi.LV_ConvertString( "It's a draw!\n" ) ) );

		Unregister( STRING_DRAW );
	}

	G_DisplayScoresToAllClients();
	PostEvent( EV_DM_Manager_FinishRoundTransition, 3.0f );
}

void DM_Manager::EventFinishRoundTransition( Event *ev )
{
	CancelEventsOfType( EV_DM_Manager_FinishRoundTransition );

	if( timelimit->integer && m_iTotalMapTime + level.inttime >= 60000 * timelimit->integer )
	{
		G_BeginIntermission2();
	}
	else
	{
		gi.Cvar_Set( "g_tempaxisscore", va( "%d", m_team_axis.m_teamwins ) );
		gi.Cvar_Set( "g_tempaxiswinsinrow", va( "%d", m_team_axis.m_wins_in_a_row ) );
		gi.Cvar_Set( "g_tempalliesscore", va( "%d", m_team_allies.m_teamwins ) );
		gi.Cvar_Set( "g_tempallieswinsinrow", va( "%d", m_team_allies.m_wins_in_a_row ) );
		gi.Cvar_Set( "g_tempmaptime", va( "%d", m_iTotalMapTime + level.inttime ) );

		gentity_t *ent;
		int i;

		for( i = 0, ent = g_entities; i < game.maxclients; ent++, i++ )
		{
			if( !ent->inuse || !ent->entity ) {
				continue;
			}

			Player *p = ( Player * )ent->entity;
			p->client->pers.kills = p->GetNumDeaths();
		}

		gi.SendConsoleCommand( "restart\n" );
	}
}
