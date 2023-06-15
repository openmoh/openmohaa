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

#ifndef __DM_TEAM_H__
#define __DM_TEAM_H__

#include "listener.h"

class PlayerStart;
class Player;

class DM_Team : public Listener
{
public:
	Container< PlayerStart * > m_spawnpoints;
	Container< Player * > m_players;

	int			m_maxplayers;

	str			m_teamname;

	int			m_teamnumber;

	teamtype_t	teamType;

	int			m_countdown;

	int			m_teamwins;
	int			m_wins_in_a_row;

	int			m_iKills;
	int			m_iDeaths;

	qboolean	m_bHasSpawnedPlayers;

public:
	CLASS_PROTOTYPE( DM_Team );

	DM_Team();
	~DM_Team();

	void	Reset( void );

	void	AddDeaths( Player *player, int numDeaths );
	void	AddKills( Player *player, int numKills );
	void	AddPlayer( Player *player );

	void	RemovePlayer( Player *player );

	void	setName( str name ) { m_teamname = name; };
	str		getName( void ) { return m_teamname; };
	void	setNumber( int num ) { m_teamnumber = num; };
	int		getNumber( void ) { return m_teamnumber; };
	void	setIndex( int index );
	int		getIndex( void );

	void	TeamWin( void );
	void	TeamLoss( void );

	bool	IsDead( void ) const;
	bool	IsEmpty( void ) const;
	bool	IsReady( void ) const;

	int		NumNotReady( void ) const;
	int		NumLivePlayers( void ) const;
	int		TotalPlayersKills( void ) const;
	void	TeamInvulnerable( void );
	void	BeginFight( void );

	float	PlayersRangeFromSpot( PlayerStart *spot );

	void	UpdateTeamStatus( void );

	void	InitSpawnPoints( void );

	PlayerStart *GetRandomFfaSpawnpoint( Player *player );
	PlayerStart *GetRandomTeamSpawnpoint( Player *player );
	PlayerStart *GetRandomObjectiveSpawnpoint( Player *player );
protected:
	PlayerStart *FarthestSpawnPoint( void );
	PlayerStart *GetRandomSpawnpoint( void );
	PlayerStart *GetRandomSpawnpointWithMetric( Player *player, float( *MetricFunction ) ( float const *, DM_Team *dmTeam, const Player *player ) );
};

typedef SafePtr< DM_Team > DM_TeamPtr;

#endif /* __DM_TEAM_H__ */
