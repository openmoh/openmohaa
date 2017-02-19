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
// dm_manager.h: Deathmatch Manager.

#ifndef __DM_MANAGER_H__
#define __DM_MANAGER_H__

#include "listener.h"
#include "dm_team.h"

extern cvar_t *g_tempaxisscore;
extern cvar_t *g_tempaxiswinsinrow;
extern cvar_t *g_tempalliesscore;
extern cvar_t *g_tempallieswinsinrow;

class Player;

class DM_Manager : public Listener
{
private:
	Container< Player * >			m_players;
	Container< DM_TeamPtr >			m_teams;

	DM_Team			m_team_spectator;
	DM_Team			m_team_freeforall;
	DM_Team			m_team_allies;
	DM_Team			m_team_axis;

	float			m_fRoundTime;
	float			m_fRoundEndTime;

	bool			m_bAllowRespawns;
	bool			m_bRoundBasedGame;
	int				m_iDefaultRoundLimit;

	const_str		m_csTeamClockSide;
	const_str		m_csTeamBombPlantSide;

	int				m_iNumTargetsToDestroy;
	int				m_iNumTargetsDestroyed;
	int				m_iNumBombsPlanted;
	int				m_iTotalMapTime;

	int				m_iTeamWin;
	bool			m_bIgnoringClockForBomb;

	// scoreboard data
	char			scoreString[ MAX_STRING_CHARS ];
	size_t			scoreLength;
	int				scoreEntries;

private:
	void		BuildTeamInfo( DM_Team *dmTeam );
	void		BuildPlayerTeamInfo( DM_Team *dmTeam, int *iPlayerList, DM_Team *ignoreTeam = NULL );
	void		InsertEntry( const char *entry );
	void		InsertEmpty( void );

public:
	CLASS_PROTOTYPE( DM_Manager );

	DM_Manager();
	~DM_Manager();

	void		Reset( void );

	DM_Team		*GetTeam( str name );
	DM_Team		*GetTeam( teamtype_t team );

	bool		JoinTeam( Player *player, teamtype_t teamType );
	void		LeaveTeam( Player *player );

	Player		*GetPlayer( int num );
	void		InitGame( void );

	void		AddPlayer( Player *player );
	void		RemovePlayer( Player *player );
	void		PlayerKilled( Player *player );

	void		RebuildTeamConfigstrings( void );

	static int	compareScores( const void *elem1, const void *elem2 );
	void		Score( Player *player );

	void		PrintAllClients( str s );
	bool		CheckEndMatch( void );
	bool		TeamHitScoreLimit( void );
	bool		PlayerHitScoreLimit( void );
	void		EventDoRoundTransition( Event *ev );
	void		EventFinishRoundTransition( Event *ev );
	void		TeamWin( int teamnum );
	void		StartRound( void );
	void		EndRound( void );
	bool		GameHasRounds( void ) const;
	bool		GameAllowsRespawns( void ) const;
	void		SetGameAllowsRespawns( bool bAllow );
	bool		AllowRespawn( void ) const;
	int			GetRoundLimit( void ) const;
	void		SetDefaultRoundLimit( int round_limit );
	const_str	GetClockSide( void ) const;
	void		SetClockSide( const_str s );
	const_str	GetBombPlantTeam( void ) const;
	void		SetBombPlantTeam( const_str s );
	int			GetTargetsToDestroy( void ) const;
	void		SetTargetsToDestroy( int );
	int			GetTargetsDestroyed( void ) const;
	void		SetTargetsDestroyed( int );
	int			GetBombsPlanted( void ) const;
	void		SetBombsPlanted( int );
	int			GetTeamWin( void );
	bool		WaitingForPlayers( void ) const;
	bool		IsGameActive( void ) const;
	int			PlayerCount( void ) const;
	teamtype_t	GetAutoJoinTeam( void );
	DM_Team		*GetTeamAllies( void );
	DM_Team		*GetTeamAxis( void );
	float		GetMatchStartTime( void );
};

inline
bool DM_Manager::GameHasRounds
	(
	void
	)
	const
{
	return m_bRoundBasedGame;
}

inline
bool DM_Manager::GameAllowsRespawns
	(
	void
	)
	const
{
	return m_bAllowRespawns;
}

inline
void DM_Manager::SetGameAllowsRespawns
	(
	bool bAllow
	)
{
	m_bAllowRespawns = bAllow;
}

extern DM_Manager dmManager;

#endif /* __DM_MANAGER_H__ */
