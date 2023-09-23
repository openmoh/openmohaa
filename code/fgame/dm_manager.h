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

#pragma once

#include "listener.h"
#include "Tow_Entities.h"

extern cvar_t *g_tempaxisscore;
extern cvar_t *g_tempaxiswinsinrow;
extern cvar_t *g_tempalliesscore;
extern cvar_t *g_tempallieswinsinrow;

class PlayerStart;
class Player;

class DM_Team : public Listener
{
public:
    Container<PlayerStart *> m_spawnpoints;
    Container<Player *>      m_players;
    int                      m_maxplayers;

    str m_teamname;
    int m_teamnumber;
    int m_index;

    int m_countdown;
    int m_teamwins;
    int m_wins_in_a_row;

    int m_iKills;
    int m_iDeaths;

    qboolean m_bHasSpawnedPlayers;

public:
    CLASS_PROTOTYPE(DM_Team);

    DM_Team();
    ~DM_Team();

    void Reset(void);

    void AddPlayer(Player *player);
    void RemovePlayer(Player *player);

    void setName(str name);
    str  getName(void);
    void setNumber(int num);
    int  getNumber(void);
    void setIndex(int index);
    int  getIndex(void);

    void TeamWin(void);
    void TeamLoss(void);
    void AddDeaths(Player *player, int numDeaths);
    void AddKills(Player *player, int numKills);
    void UpdateTeamStatus(void);

    void TeamInvulnerable(void);
    void BeginFight(void);

    float PlayersRangeFromSpot(PlayerStart *spot);

    bool IsDead(void) const;
    bool IsEmpty(void) const;
    bool IsReady(void) const;

    int NumNotReady(void) const;
    int NumLivePlayers(void) const;
    int TotalPlayersKills(void) const;

    PlayerStart *GetRandomFfaSpawnpoint(Player *player);
    PlayerStart *GetRandomTeamSpawnpoint(Player *player);
    PlayerStart *GetRandomObjectiveSpawnpoint(Player *player);

    void InitSpawnPoints(void);

protected:
    PlayerStart *FarthestSpawnPoint(void);
    PlayerStart *GetRandomSpawnpoint(void);
    PlayerStart *GetRandomSpawnpointWithMetric(
        Player *player, float (*MetricFunction)(const float *, DM_Team *dmTeam, const Player *player)
    );
};

using DM_TeamPtr = SafePtr<DM_Team>;

inline void DM_Team::setName(str name)
{
    m_teamname = name;
};

inline str DM_Team::getName(void)
{
    return m_teamname;
};

inline void DM_Team::setNumber(int num)
{
    m_teamnumber = num;
}

inline int DM_Team::getNumber(void)
{
    return m_teamnumber;
}

inline void DM_Team::setIndex(int index)
{
    m_index = index;
}

inline int DM_Team::getIndex(void)
{
    return m_index;
}

inline bool DM_Team::IsReady(void) const
{
    return !NumNotReady();
}

class SimpleAmmoType : public Class
{
public:
    str type;
    int amount;

public:
    SimpleAmmoType();
};

class Player;

class DM_Manager : public Listener
{
private:
    Container<Player *>   m_players;
    Container<DM_TeamPtr> m_teams;

    DM_Team m_team_spectator;
    DM_Team m_team_freeforall;
    DM_Team m_team_allies;
    DM_Team m_team_axis;

    float m_fRoundTime;
    float m_fRoundEndTime;

    bool m_bAllowRespawns;
    bool m_bRoundBasedGame;
    bool m_bIgnoringClockForBomb;
    int  m_iTeamWin;
    int  m_iDefaultRoundLimit;

    const_str m_csTeamClockSide;
    const_str m_csTeamBombPlantSide;

    int m_iNumTargetsToDestroy;
    int m_iNumTargetsDestroyed;
    int m_iNumBombsPlanted;
    int m_iTotalMapTime;

    bool m_bAllowAxisRespawn;
    bool m_bAllowAlliedRespawn;
    bool m_bRoundActive;

    // scoreboard data
    char   scoreString[MAX_STRING_CHARS];
    size_t scoreLength;
    int    scoreEntries;

private:
    void BuildTeamInfo(DM_Team *dmTeam);
    void BuildTeamInfo_ver6(DM_Team *dmTeam);
    void BuildTeamInfo_ver15(DM_Team *dmTeam);
    void BuildPlayerTeamInfo(DM_Team *dmTeam, int *iPlayerList, DM_Team *ignoreTeam = NULL);
    void InsertEntry(const char *entry);
    void InsertEntryNoCount(const char *entry);
    void InsertEmpty(void);
    bool IsAlivePlayer(Player* player) const;

public:
    CLASS_PROTOTYPE(DM_Manager);

    DM_Manager();
    ~DM_Manager();

    void InitGame(void);
    void AddPlayer(Player *player);
    bool JoinTeam(Player *player, teamtype_t teamType);
    void LeaveTeam(Player *player);
    void RebuildTeamConfigstrings(void);
    void RemovePlayer(Player *player);
    void PlayerKilled(Player *player);

    void Countdown(Event *ev);
    void Reset(void);
    void Score(Player *player);

    void       PrintAllClients(str s);
    bool       CheckEndMatch(void);
    bool       TeamHitScoreLimit(void);
    bool       PlayerHitScoreLimit(void);
    void       EventDoRoundTransition(Event *ev);
    void       EventFinishRoundTransition(Event *ev);
    void       TeamWin(int teamnum);
    void       StartRound(void);
    void       EndRound(void);
    bool       RoundActive(void) const;
    bool       GameHasRounds(void) const;
    bool       GameAllowsRespawns(void) const;
    void       SetGameAllowsRespawns(bool bAllow);
    bool       AllowRespawn(void) const;
    int        GetRoundLimit(void) const;
    void       SetDefaultRoundLimit(int round_limit);
    const_str  GetClockSide(void) const;
    void       SetClockSide(const_str s);
    const_str  GetBombPlantTeam(void) const;
    void       SetBombPlantTeam(const_str s);
    int        GetTargetsToDestroy(void) const;
    void       SetTargetsToDestroy(int);
    int        GetTargetsDestroyed(void) const;
    void       SetTargetsDestroyed(int);
    int        GetBombsPlanted(void) const;
    void       SetBombsPlanted(int);
    int        GetTeamWin(void);
    bool       WaitingForPlayers(void) const;
    bool       IsGameActive(void) const;
    int        PlayerCount(void) const;
    Player    *GetPlayer(int index) const;
    teamtype_t GetAutoJoinTeam(void);

    DM_Team *GetTeamAllies(void);
    DM_Team *GetTeamAxis(void);

    float GetMatchStartTime(void);
    void  StopTeamRespawn(eController controller);
    bool  AllowTeamRespawn(int teamnum) const;
    int   GetTeamSpawnTimeLeft() const;

    DM_Team *GetTeam(str name);
    DM_Team *GetTeam(teamtype_t team);

protected:
    static int compareScores(const void *elem1, const void *elem2);
};

inline int DM_Manager::GetTeamWin(void)
{
    return m_iTeamWin;
}

inline DM_Team *DM_Manager::GetTeamAllies(void)
{
    return &m_team_allies;
}

inline DM_Team *DM_Manager::GetTeamAxis(void)
{
    return &m_team_axis;
}

inline bool DM_Manager::RoundActive(void) const
{
    return m_bRoundActive;
}

inline bool DM_Manager::GameHasRounds(void) const
{
    return m_bRoundBasedGame;
}

inline bool DM_Manager::GameAllowsRespawns(void) const
{
    return m_bAllowRespawns;
}

inline void DM_Manager::SetGameAllowsRespawns(bool bAllow)
{
    m_bAllowRespawns = bAllow;
}

extern DM_Manager dmManager;
