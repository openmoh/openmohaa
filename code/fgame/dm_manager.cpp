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
#include "playerstart.h"
#include "scriptexception.h"

cvar_t *g_tempaxisscore;
cvar_t *g_tempaxiswinsinrow;
cvar_t *g_tempalliesscore;
cvar_t *g_tempallieswinsinrow;

DM_Manager dmManager;

static CTeamSpawnClock g_teamSpawnClock;

typedef struct spawnsort_s {
    PlayerStart *spawnpoint;
    float        fMetric;
} spawnsort_t;

static qboolean SpotWouldTelefrag(float *origin)
{
    static Vector mins = Vector(-16, -16, 1);
    static Vector maxs = Vector(16, 16, 97);
    trace_t       trace;

    trace = G_Trace(Vector(origin), mins, maxs, Vector(origin), NULL, MASK_PLAYERSTART, qfalse, "SpotWouldTelefrag");

    if (trace.startsolid || trace.allsolid) {
        return qtrue;
    } else {
        return qfalse;
    }
}

static int compare_spawnsort(const void *pe1, const void *pe2)
{
    float fDelta = ((spawnsort_t *)pe1)->fMetric - ((spawnsort_t *)pe2)->fMetric;

    if (fDelta >= -0.001f) {
        if (fDelta <= 0.001f) {
            return 0;
        } else {
            return -1;
        }
    } else {
        return 1;
    }
}

static PlayerStart *GetRandomSpawnpointFromList(spawnsort_t *pSpots, int nSpots)
{
    int   i = nSpots;
    float fTotalMetric;
    float fMinPosMetric;
    float fChosen;

    if (nSpots <= 0) {
        return NULL;
    }

    qsort(pSpots, nSpots, sizeof(spawnsort_t), compare_spawnsort);

    if (pSpots[0].fMetric > 0.0f) {
        if (nSpots > 5) {
            nSpots = 5;
        }

        fMinPosMetric = pSpots[0].fMetric * nSpots;
        fTotalMetric  = fMinPosMetric;

        if (nSpots <= 1) {
            fChosen = fMinPosMetric;
        } else {
            i            = 0;
            fTotalMetric = 0.0f;

            for (i = 0; i < nSpots; i++) {
                if (pSpots[i].fMetric <= 0.0f) {
                    break;
                }

                fChosen = pSpots[i].fMetric * (nSpots - i);
                fTotalMetric += fChosen;

                i++;
            }

            if (i < nSpots) {
                fChosen = fMinPosMetric;
            }

            fMinPosMetric = fTotalMetric;
        }

        fTotalMetric = (fMinPosMetric - i * fChosen * 0.90f) * G_Random();
        for (i = 0; i < nSpots - 1; i++) {
            fTotalMetric -= (nSpots - i) * pSpots[i].fMetric - (fChosen * 0.90f);
            if (fTotalMetric <= 0.0f) {
                break;
            }
        }

        return pSpots[i].spawnpoint;
    } else {
        // return the spot anyway
        return pSpots[0].spawnpoint;
    }
}

float SpawnpointMetric_Ffa(const float *origin, DM_Team *dmTeam, const Player *player)
{
    float fMinEnemyDistSquared = 23170.0f * 23170.0f;
    int   i;
    int   nPlayers = dmManager.PlayerCount();
    float fDist;

    for (i = 1; i < nPlayers; i++) {
        Player *teammate = dmManager.GetPlayer(i);
        if (teammate == player || teammate->IsDead() || teammate->IsSpectator()) {
            continue;
        }

        fDist = (teammate->origin - Vector(origin)).lengthSquared();

        if (fMinEnemyDistSquared > fDist) {
            fMinEnemyDistSquared = fDist;
        }
    }

    return fMinEnemyDistSquared - (G_Random(0.25f) + 1.0f) * (1024.0f * 1024.0f);
}

float SpawnpointMetric_Team(const float *origin, DM_Team *dmTeam, const Player *player)
{
    float fMinEnemyDistSquared  = 23170.0f * 23170.0f;
    float fSumFriendDistSquared = 0.0f;
    float fDistSquared;
    float fMetric;
    int   i;
    int   nPlayers = dmManager.PlayerCount();
    int   nFriends = 0;

    for (i = 1; i < nPlayers; i++) {
        Player *teammate = dmManager.GetPlayer(i);
        if (teammate == player || teammate->IsDead() || teammate->IsSpectator()) {
            continue;
        }

        fDistSquared = (teammate->origin - Vector(origin)).lengthSquared();

        if (teammate->GetDM_Team() == dmTeam) {
            nFriends++;
            fSumFriendDistSquared += fDistSquared;
        } else {
            if (fMinEnemyDistSquared > fDistSquared) {
                fMinEnemyDistSquared = fDistSquared;
            }
        }
    }

    fMetric = fMinEnemyDistSquared - (G_Random(0.25f) + 1.0f) * (1024.0f * 1024.0f);

    if (nFriends) {
        fMetric += 0.25f * ((23170.0f * 23170.0f) - fSumFriendDistSquared / nFriends);
    }

    return fMetric;
}

float SpawnpointMetric_Objective(const float *origin, DM_Team *dmTeam, const Player *player)
{
    return rand() * 0.0000000005f;
}

CLASS_DECLARATION(Listener, DM_Team, NULL) {
    {NULL, NULL}
};

DM_Team::DM_Team()
{
    m_teamwins           = 0;
    m_wins_in_a_row      = 0;
    m_teamnumber         = -1;
    m_iKills             = 0;
    m_iDeaths            = 0;
    m_bHasSpawnedPlayers = false;
}

DM_Team::~DM_Team() {}

void DM_Team::Reset(void)
{
    m_spawnpoints.ClearObjectList();
    m_players.ClearObjectList();

    if (g_gametype->integer == GT_TEAM) {
        m_teamwins = 0;
    }
}

void DM_Team::AddPlayer(Player *player)
{
    m_players.AddUniqueObject(player);
}

void DM_Team::RemovePlayer(Player *player)
{
    m_players.RemoveObject(player);
}

void DM_Team::TeamWin(void)
{
    m_teamwins++;
    m_wins_in_a_row++;

    for (int i = 1; i <= m_players.NumObjects(); i++) {
        m_players.ObjectAt(i)->WonMatch();
    }

    UpdateTeamStatus();

    if (m_teamnumber == TEAM_ALLIES) {
        gi.cvar_set("g_scoreboardpicover", "textures/hud/allieswin");
    } else if (m_teamnumber == TEAM_AXIS) {
        gi.cvar_set("g_scoreboardpicover", "textures/hud/axiswin");
    }
}

void DM_Team::TeamLoss(void)
{
    m_wins_in_a_row = 0;

    for (int i = 1; i <= m_players.NumObjects(); i++) {
        m_players.ObjectAt(i)->LostMatch();
    }

    UpdateTeamStatus();
}

void DM_Team::AddKills(Player *player, int numKills)
{
    if (level.intermissiontime || dmManager.GetTeamWin()) {
        return;
    }

    player->AddKills(numKills);

    if (m_teamnumber > TEAM_FREEFORALL) {
        m_iKills += numKills;

        if ((g_gametype->integer >= GT_TEAM_ROUNDS && g_gametype->integer <= GT_TOW) || g_gametype->integer == GT_LIBERATION) {
            player->AddDeaths(numKills);
        } else {
            m_teamwins += numKills;
        }
    }
}

void DM_Team::AddDeaths(Player *player, int numDeaths)
{
    if (level.intermissiontime || dmManager.GetTeamWin()) {
        return;
    }

    if ((g_gametype->integer >= GT_TEAM_ROUNDS && g_gametype->integer <= GT_TOW) || g_gametype->integer == GT_LIBERATION) {
        return;
    }

    player->AddDeaths(numDeaths);

    if (m_teamnumber > TEAM_FREEFORALL) {
        m_iDeaths += numDeaths;
    }
}

void DM_Team::TeamInvulnerable(void)
{
    for (int i = 1; i <= m_players.NumObjects(); i++) {
        m_players.ObjectAt(i)->takedamage = DAMAGE_NO;
    }
}

void DM_Team::BeginFight(void)
{
    for (int i = 1; i <= m_players.NumObjects(); i++) {
        m_players.ObjectAt(i)->BeginFight();
    }
}

float DM_Team::PlayersRangeFromSpot(PlayerStart *spot)
{
    float      bestplayerdistance = 9999999.0f;
    Vector     vDist;
    int        i;
    gentity_t *ent;
    Player    *player;

    if (g_gametype->integer > GT_FFA) {
        return bestplayerdistance;
    }

    // find the nearest player from the post
    for (i = 0, ent = g_entities; i < game.maxclients; i++, ent++) {
        if (!ent->client || !ent->entity) {
            continue;
        }

        player = (Player *)ent->entity;
        if (!player->IsDead() && !player->IsSpectator() && m_teamnumber != player->GetTeam()) {
            vDist = spot->origin - player->origin;
            if (bestplayerdistance > vDist.length()) {
                bestplayerdistance = vDist.length();
            }
        }
    }

    return bestplayerdistance;
}

PlayerStart *DM_Team::FarthestSpawnPoint(void)
{
    int          i;
    int          iNumPoints;
    float        bestdistance;
    float        bestplayerdistance;
    PlayerStart *pSpot;
    PlayerStart *pBestSpot;
    PlayerStart *pSpot2;
    PlayerStart *pSpot3;

    bestplayerdistance = 0.0f;
    pBestSpot          = NULL;
    pSpot2             = NULL;
    pSpot3             = NULL;
    iNumPoints         = m_spawnpoints.NumObjects();

    for (i = 1; i <= iNumPoints; i++) {
        pSpot = m_spawnpoints.ObjectAt(i);

        bestdistance = PlayersRangeFromSpot(m_spawnpoints.ObjectAt(i));
        if (bestdistance > bestplayerdistance) {
            bestplayerdistance = bestdistance;
            pSpot3             = pSpot2;
            pSpot2             = pBestSpot;
            pBestSpot          = pSpot;
        }
    }

    if (pSpot3 && G_Random() < 0.2f) {
        return pSpot3;
    } else if (pSpot2 && G_Random() < 0.3f) {
        return pSpot2;
    } else {
        return pBestSpot;
    }
}

PlayerStart *DM_Team::GetRandomSpawnpoint(void)
{
    PlayerStart *spot      = NULL;
    int          numPoints = m_spawnpoints.NumObjects();

    if (numPoints) {
        spot = FarthestSpawnPoint();
        if (!spot) {
            spot = m_spawnpoints.ObjectAt((int)(G_Random(numPoints) + 1.0f));
        }
    } else {
        warning("DM_Team::GetRandomSpawnpoint", "No spawnpoints found\n");
        spot = NULL;
    }

    return spot;
}

PlayerStart *DM_Team::GetRandomSpawnpointWithMetric(
    Player *player, float (*MetricFunction)(const float *origin, DM_Team *dmTeam, const Player *player)
)
{
    static float offset[4][3];
    spawnsort_t  points[1024];
    PlayerStart *spot     = NULL;
    int          numSpots = 0;
    int          iPoint   = 0;

    for (int i = 1; i <= m_spawnpoints.NumObjects(); i++) {
        spot = m_spawnpoints.ObjectAt(i);
        if (spot->m_bForbidSpawns || player->GetLastSpawnpoint() == spot) {
            continue;
        }

        if (!SpotWouldTelefrag(spot->origin)) {
            points[numSpots].spawnpoint = spot;
            points[numSpots].fMetric    = MetricFunction(spot->origin, this, player);
            numSpots++;

            if (numSpots >= (sizeof(points) / sizeof(points[0]))) {
                break;
            }
        }
    }

    spot = GetRandomSpawnpointFromList(points, numSpots);
    if (spot) {
        return spot;
    }

    numSpots = 0;

    for (int i = 1; i <= m_spawnpoints.NumObjects(); i++) {
        spot = m_spawnpoints.ObjectAt(i);
        if (spot->m_bForbidSpawns) {
            continue;
        }

        for (int j = 0; j < sizeof(offset) / sizeof(offset[0]); j++) {
            Vector vNewSpawn = spot->origin + offset[j];

            if (G_SightTrace(
                    spot->origin,
                    player->mins,
                    player->maxs,
                    vNewSpawn,
                    (Entity *)NULL,
                    (Entity *)NULL,
                    MASK_PLAYERSOLID,
                    qfalse,
                    "DM_Team::GetRandomSpawnpointWithMetric"
                )
                == 1) {
                Vector vEnd = vNewSpawn - Vector(0, 0, 64);

                trace_t trace = G_Trace(
                    vNewSpawn, player->mins, player->maxs, vEnd, player, MASK_PLAYERSOLID, qfalse, "TempSpawnPoint"
                );

                if (!trace.allsolid && !trace.startsolid && trace.fraction != 1.0f && trace.fraction != 1.0f
                    && trace.plane.dist >= 0.8f) {
                    points[numSpots].spawnpoint = new PlayerStart;
                    points[numSpots].spawnpoint->setOrigin(trace.endpos);
                    points[numSpots].spawnpoint->setAngles(spot->angles);
                    points[numSpots].fMetric = MetricFunction(vNewSpawn, this, player);

                    if (numSpots >= (sizeof(points) / sizeof(points[0]))) {
                        break;
                    }
                }
            }
        }
    }

    spot = GetRandomSpawnpointFromList(points, numSpots);
    for (int i = 0; i < numSpots; i++) {
        // delete all created spawnpoint
        if (points[numSpots].spawnpoint != spot) {
            delete points[numSpots].spawnpoint;
        }
    }

    if (spot) {
        return spot;
    }

    numSpots = 0;

    for (int i = 1; i <= m_spawnpoints.NumObjects(); i++) {
        spot = m_spawnpoints.ObjectAt(i);
        if (!spot->m_bForbidSpawns && player->GetLastSpawnpoint() != spot) {
            continue;
        }

        if (!SpotWouldTelefrag(spot->origin)) {
            points[numSpots].spawnpoint = spot;
            points[numSpots].fMetric    = MetricFunction(spot->origin, this, player);

            if (numSpots >= (sizeof(points) / sizeof(points[0]))) {
                break;
            }
        }
    }

    return GetRandomSpawnpointFromList(points, numSpots);
}

PlayerStart *DM_Team::GetRandomFfaSpawnpoint(Player *player)
{
    return GetRandomSpawnpointWithMetric(player, SpawnpointMetric_Ffa);
}

PlayerStart *DM_Team::GetRandomTeamSpawnpoint(Player *player)
{
    return GetRandomSpawnpointWithMetric(player, SpawnpointMetric_Team);
}

PlayerStart *DM_Team::GetRandomObjectiveSpawnpoint(Player *player)
{
    return GetRandomSpawnpointWithMetric(player, SpawnpointMetric_Objective);
}

bool DM_Team::IsDead(void) const
{
    Player *player;

    if (m_players.NumObjects() == 0) {
        return dmManager.IsGameActive();
    }

    if (!m_bHasSpawnedPlayers) {
        return false;
    }

    if (g_gametype->integer == GT_TOW) {
        if (dmManager.AllowTeamRespawn(m_teamnumber)) {
            return false;
        }
    } else if (dmManager.AllowRespawn()) {
        return false;
    }

    for (int i = m_players.NumObjects(); i > 0; i--) {
        player = m_players.ObjectAt(i);

        if (player->IsDead()) {
            continue;
        }

        if (!player->IsSpectator()) {
            return false;
        }
    }

    return true;
}

int DM_Team::NumLivePlayers(void) const
{
    Player *player;
    int     num = 0;

    for (int i = 1; i <= m_players.NumObjects(); i++) {
        player = m_players.ObjectAt(i);

        if (!player->IsDead() && !player->IsSpectator()) {
            num++;
        }
    }

    return num;
}

int DM_Team::TotalPlayersKills(void) const
{
    Player *player;
    int     iKills = 0;

    for (int i = 1; i <= m_players.NumObjects(); i++) {
        player = m_players.ObjectAt(i);
        iKills += player->GetNumKills();
    }

    return iKills;
}

bool DM_Team::IsEmpty(void) const
{
    return !m_players.NumObjects() || !m_bHasSpawnedPlayers;
}

int DM_Team::NumNotReady(void) const
{
    Player *player;
    int     num = 0;

    for (int i = 1; i <= m_players.NumObjects(); i++) {
        player = m_players.ObjectAt(i);

        if (player->IsReady()) {
            i++;
        }
    }

    return num;
}

void DM_Team::UpdateTeamStatus(void)
{
    for (int i = 1; i <= m_players.NumObjects(); i++) {
        m_players.ObjectAt(i)->UpdateStatus(va("%d wins (%d in a row)", m_teamwins, m_wins_in_a_row));
    }
}

SimpleAmmoType::SimpleAmmoType()
    : amount(0)
{}

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
    "finishroundtransition", EV_DEFAULT, NULL, NULL, "delayed function call to do the actual restart for the next round"
);

CLASS_DECLARATION(Listener, DM_Manager, NULL) {
    {&EV_DM_Manager_DoRoundTransition,     &DM_Manager::EventDoRoundTransition    },
    {&EV_DM_Manager_FinishRoundTransition, &DM_Manager::EventFinishRoundTransition},
    {NULL,                                 NULL                                   }
};

DM_Manager::DM_Manager()
{
    m_team_spectator.m_maxplayers = MAX_CLIENTS;
    m_team_spectator.setName("spectator");
    m_team_spectator.setNumber(TEAM_SPECTATOR);
    m_team_spectator.setIndex(TEAM_SPECTATOR);

    m_team_freeforall.m_maxplayers = MAX_CLIENTS;
    m_team_freeforall.setName("free-for-all");
    m_team_freeforall.setNumber(TEAM_FREEFORALL);
    m_team_freeforall.setIndex(TEAM_FREEFORALL);

    m_team_allies.m_maxplayers = MAX_CLIENTS;
    m_team_allies.setName("allies");
    m_team_allies.setNumber(TEAM_ALLIES);
    m_team_allies.setIndex(TEAM_ALLIES);

    m_team_axis.m_maxplayers = MAX_CLIENTS;
    m_team_axis.setName("axis");
    m_team_axis.setNumber(TEAM_AXIS);
    m_team_axis.setIndex(TEAM_AXIS);

    m_fRoundTime           = 0;
    m_fRoundEndTime        = 0;
    m_bAllowRespawns       = true;
    m_bRoundBasedGame      = false;
    m_iTeamWin             = 0;
    m_iDefaultRoundLimit   = 0;
    m_csTeamClockSide      = STRING_AXIS;
    m_csTeamBombPlantSide  = STRING_DRAW;
    m_iNumTargetsToDestroy = 1;
    m_iNumTargetsDestroyed = 0;
    m_iNumBombsPlanted     = 0; 
    m_bAllowAxisRespawn    = true;
    m_bAllowAlliedRespawn  = true;
    m_bRoundActive         = false;
}

DM_Manager::~DM_Manager() {}

void DM_Manager::Reset(void)
{
    m_team_allies.Reset();
    m_team_axis.Reset();
    m_team_spectator.Reset();
    m_team_freeforall.Reset();

    m_players.ClearObjectList();
    m_teams.ClearObjectList();

    gi.cvar_set("g_scoreboardpicover", "");

    //
    // Added in 2.0
    //
    m_bAllowAxisRespawn   = true;
    m_bAllowAlliedRespawn = true;

    // Reset the team spawn clock
    g_teamSpawnClock.Reset();
    level.m_bIgnoreClock = false;

    if (g_gametype->integer == GT_TOW) {
        g_TOWObjectiveMan.Reset();
        gi.cvar_set("g_TOW_winstate", "0");
    } else if (g_gametype->integer == GT_LIBERATION) {
        gi.cvar_set("scoreboard_toggle1", "0");
        gi.cvar_set("scoreboard_toggle2", "0");
    }
}

void DM_Manager::AddPlayer(Player *player)
{
    m_players.AddUniqueObject(player);
}

void DM_Manager::RemovePlayer(Player *player)
{
    DM_Team *pDMTeam;

    m_players.RemoveObject(player);

    for (int i = m_teams.NumObjects(); i > 0; i--) {
        pDMTeam = m_teams.ObjectAt(i);

        if (pDMTeam->m_players.IndexOfObject(player)) {
            pDMTeam->RemovePlayer(player);

            if (!pDMTeam->m_players.NumObjects()) {
                pDMTeam->m_bHasSpawnedPlayers = qfalse;
            }
        }
    }

    player->SetDM_Team(NULL);

    RebuildTeamConfigstrings();
}

bool DM_Manager::JoinTeam(Player *player, teamtype_t teamType)
{
    DM_Team *team    = player->GetDM_Team();
    DM_Team *pDMTeam = GetTeam(teamType);

    if (!pDMTeam) {
        return false;
    }

    if (pDMTeam->m_players.NumObjects() >= pDMTeam->m_maxplayers) {
        gi.centerprintf(player->edict, gi.LV_ConvertString("That team is full"));
        return false;
    }

    if (team) {
        LeaveTeam(player);
    }

    pDMTeam->AddPlayer(player);
    AddPlayer(player);
    player->SetDM_Team(pDMTeam);

    if (teamType == TEAM_SPECTATOR) {
        player->EndFight();
    } else {
        player->BeginFight();
    }

    return true;
}

void DM_Manager::PlayerKilled(Player *player)
{
    // Spawn a deadbody
    player->DeadBody(NULL);

    // Hide the model because of the dead body
    player->hideModel();

    // Don't let the player die a second time
    player->takedamage = DAMAGE_NO;

    CheckEndMatch();
}

void DM_Manager::LeaveTeam(Player *player)
{
    DM_Team *team = player->GetDM_Team();

    if (team) {
        if (team->m_players.IndexOfObject(player)) {
            team->RemovePlayer(player);
            RemovePlayer(player);
            player->SetDM_Team(NULL);
            RebuildTeamConfigstrings();
        } else {
            warning("DM_Manager::LeaveTeam", "Could not find team in the arena\n");
        }
    } else {
        warning("DM_Manager::LeaveTeam", "Could not find a team for this player\n");
    }
}

void DM_Manager::RebuildTeamConfigstrings(void)
{
    DM_TeamPtr team;
    int        teamcount;

    teamcount = m_teams.NumObjects();

    for (int i = 1; i <= teamcount; i++) {
        team = m_teams.ObjectAt(i);

        gi.setConfigstring(
            CS_GENERAL_STRINGS + i,
            va("%d %s %d player(s)", team->m_teamnumber, team->m_teamname.c_str(), team->m_players.NumObjects())
        );
    }

    gi.setConfigstring(CS_TEAMS, va("%d", teamcount));
}

int DM_Manager::compareScores(const void *elem1, const void *elem2)
{
    if (*(int *)elem1 < -1 || *(int *)elem2 < 0) {
        return 0;
    }

    Player *p1 = (Player *)G_GetEntity(*(int *)elem1);
    Player *p2 = (Player *)G_GetEntity(*(int *)elem2);

    if (p1->GetNumKills() < p2->GetNumKills()) {
        return 1;
    } else if (p1->GetNumKills() == p2->GetNumKills()) {
        // sort by death if they have the same number of kills
        if (p1->GetNumDeaths() < p2->GetNumDeaths()) {
            return 1;
        } else if (p1->GetNumDeaths() == p2->GetNumDeaths()) {
            // sort by netname if they have the same number of deaths
            return Q_stricmp(p1->client->pers.netname, p2->client->pers.netname);
        } else if (p1->GetNumDeaths() > p2->GetNumDeaths()) {
            return -1;
        }
    } else if (p1->GetNumKills() > p2->GetNumKills()) {
        return -1;
    }

    // just to avoid the compiler warning
    // shouldn't go there
    return 0;
}

void DM_Manager::Score(Player *player)
{
    int      i, j;
    int      count        = 0;
    int      stringlength = 0;
    Player  *currentPlayer;
    int      iPlayerList[MAX_CLIENTS];
    DM_Team *pDMTeam;

    //
    // Since 2.0, added the player count for individual team, and added new indices for GT_TOW
    // Since 2.30, added booleans for liberation scoreboard toggle
    //

    assert(player);

    scoreString[0] = 0;
    scoreLength    = 0;
    scoreEntries   = 0;

    pDMTeam = NULL;

    // make the winning team at top
    if (g_gametype->integer >= GT_TEAM_ROUNDS) {
        if (m_team_allies.m_teamwins <= m_team_axis.m_teamwins) {
            if (m_team_axis.m_teamwins > m_team_allies.m_teamwins) {
                pDMTeam = &m_team_axis;
            } else if (m_team_allies.m_wins_in_a_row > m_team_axis.m_wins_in_a_row) {
                pDMTeam = &m_team_allies;
            } else if (m_team_axis.m_wins_in_a_row <= m_team_allies.m_wins_in_a_row) {
                // make the player's current team at top
                pDMTeam = player->GetDM_Team();

                if (pDMTeam != &m_team_allies && pDMTeam != &m_team_axis) {
                    pDMTeam = &m_team_allies;
                }
            } else {
                pDMTeam = &m_team_axis;
            }
        } else {
            pDMTeam = &m_team_allies;
        }
    } else if (g_gametype->integer > GT_FFA) {
        if (m_team_allies.m_iKills <= m_team_axis.m_iKills) {
            if (m_team_axis.m_iKills > m_team_allies.m_iKills) {
                pDMTeam = &m_team_axis;
            } else if (m_team_allies.m_iDeaths > m_team_axis.m_iDeaths) {
                pDMTeam = &m_team_allies;
            } else if (m_team_axis.m_iDeaths <= m_team_allies.m_iDeaths) {
                pDMTeam = player->GetDM_Team();

                if (pDMTeam != &m_team_allies && pDMTeam != &m_team_axis) {
                    pDMTeam = &m_team_allies;
                }
            } else {
                pDMTeam = &m_team_axis;
            }
        } else {
            pDMTeam = &m_team_allies;
        }
    }

    memset(iPlayerList, -1, sizeof(iPlayerList));

    for (i = 1, j = 0; i <= PlayerCount(); i++) {
        currentPlayer = GetPlayer(i);

        if (!currentPlayer) {
            continue;
        }

        iPlayerList[j] = currentPlayer->client->ps.clientNum;

        j++;
    }

    // sort players by kills
    qsort(iPlayerList, j, sizeof(int), compareScores);

    switch (g_gametype->integer) {
    case GT_TOW:
        {
            char buffer[1024];

            // send the number for all tow objectives
            Com_sprintf(
                buffer,
                sizeof(buffer),
                "%i %i %i %i %i %i %i %i %i %i ",
                gi.Cvar_Get("tow_allied_obj1", "", 0)->integer,
                gi.Cvar_Get("tow_allied_obj2", "", 0)->integer,
                gi.Cvar_Get("tow_allied_obj3", "", 0)->integer,
                gi.Cvar_Get("tow_allied_obj4", "", 0)->integer,
                gi.Cvar_Get("tow_allied_obj5", "", 0)->integer,
                gi.Cvar_Get("tow_axis_obj1", "", 0)->integer,
                gi.Cvar_Get("tow_axis_obj2", "", 0)->integer,
                gi.Cvar_Get("tow_axis_obj3", "", 0)->integer,
                gi.Cvar_Get("tow_axis_obj4", "", 0)->integer,
                gi.Cvar_Get("tow_axis_obj5", "", 0)->integer
            );

            InsertEntryNoCount(buffer);
            break;
        }
    case GT_LIBERATION:
        {
            char buffer[1024];

            // scoreboard index
            Com_sprintf(
                buffer,
                sizeof(buffer),
                "%i %i ",
                gi.Cvar_Get("scoreboard_toggle1", "", 0)->integer,
                gi.Cvar_Get("scoreboard_toggle2", "", 0)->integer
            );

            InsertEntryNoCount(buffer);
            break;
        }
    }

    // build team info
    if (g_gametype->integer > GT_FFA) {
        BuildTeamInfo(pDMTeam);
        BuildPlayerTeamInfo(pDMTeam, iPlayerList);

        // insert an empty entry to not make the scoreboard tight
        InsertEmpty();

        if (pDMTeam != &m_team_allies) {
            BuildTeamInfo(&m_team_allies);
            BuildPlayerTeamInfo(&m_team_allies, iPlayerList);
        } else if (pDMTeam != &m_team_axis) {
            BuildTeamInfo(&m_team_axis);
            BuildPlayerTeamInfo(&m_team_axis, iPlayerList);
        }
    } else {
        // client will only show "Players" in FFA
        // BuildTeamInfo( &m_team_freeforall );

        BuildPlayerTeamInfo(NULL, iPlayerList, &m_team_spectator);
    }

    // spectator is the last team in the scoreboard
    if (m_team_spectator.m_players.NumObjects()) {
        InsertEmpty();

        BuildTeamInfo(&m_team_spectator);
        BuildPlayerTeamInfo(&m_team_spectator, iPlayerList);
    }

    // send the info to the client
    gi.SendServerCommand(player->client->ps.clientNum, "scores %i %s", scoreEntries, scoreString);
}

void DM_Manager::PrintAllClients(str s)
{
    gentity_t *ent;
    int        i;
    Player    *player;

    if (game.maxclients <= 0) {
        return;
    }

    for (i = 0, ent = g_entities; i < game.maxclients; i++, ent++) {
        if (!ent->inuse || !ent->client || !ent->entity) {
            continue;
        }

        player = (Player *)ent->entity;
        player->HUDPrint(s);
    }
}

void DM_Manager::InitGame(void)
{
    int i;

    if (fraglimit) {
        if (fraglimit->integer < 0) {
            gi.cvar_set("fraglimit", "0");
        }
        if (fraglimit->integer > 10000) {
            gi.cvar_set("fraglimit", "10000");
        }
        fraglimit = gi.Cvar_Get("fraglimit", "0", CVAR_SERVERINFO);
    }

    if (timelimit) {
        if (timelimit->integer < 0) {
            gi.cvar_set("timelimit", "0");
        }
        // 180 minutes maximum
        if (timelimit->integer > 10800) {
            gi.cvar_set("timelimit", "10800");
        }
        timelimit = gi.Cvar_Get("timelimit", "0", CVAR_SERVERINFO);
    }

    for (i = 1; i <= level.m_SimpleArchivedEntities.NumObjects(); i++) {
        SimpleArchivedEntity* const ent = level.m_SimpleArchivedEntities.ObjectAt(i);
        const char* const classname = ent->getClassID();

        if (!Q_stricmp(classname, "info_player_deathmatch")) {
            PlayerStart* const spawnpoint = static_cast<PlayerStart*>(ent);
            m_team_spectator.m_spawnpoints.AddObject(spawnpoint);
            m_team_freeforall.m_spawnpoints.AddObject(spawnpoint);

            if (g_gametype->integer == GT_FFA) {
                m_team_allies.m_spawnpoints.AddObject(spawnpoint);
                m_team_axis.m_spawnpoints.AddObject(spawnpoint);
            }
        } else if (!Q_stricmp(classname, "info_player_allied")) {
            if (g_gametype->integer >= GT_TEAM) {
                PlayerStart* const spawnpoint = static_cast<PlayerStart*>(ent);
                m_team_allies.m_spawnpoints.AddObject(spawnpoint);
            }
        } else if (!Q_stricmp(classname, "info_player_axis")) {
            if (g_gametype->integer >= GT_TEAM) {
                PlayerStart* const spawnpoint = static_cast<PlayerStart*>(ent);
                m_team_axis.m_spawnpoints.AddObject(spawnpoint);
            }
        } else if (!Q_stricmp(classname, "info_player_intermission")) {
            PlayerStart* const spawnpoint = static_cast<PlayerStart*>(ent);
            m_team_freeforall.m_spawnpoints.AddObject(spawnpoint);
        }
    }

    if (g_gametype->integer > GT_SINGLE_PLAYER) {
        if (g_gametype->integer < GT_MAX_GAME_TYPE) {
            m_teams.ClearObjectList();
            m_teams.AddObject(&m_team_spectator);
            m_teams.AddObject(&m_team_allies);
            m_teams.AddObject(&m_team_axis);
        } else {
            Com_Printf("Unknown game mode");
        }
    }

    m_fRoundTime            = 0;
    m_fRoundEndTime         = 0;
    m_iTeamWin              = 0;
    m_bIgnoringClockForBomb = false;
    m_iNumTargetsDestroyed  = 0;
    m_iNumBombsPlanted      = 0;

    if (g_gametype->integer >= 0 && g_gametype->integer < GT_MAX_GAME_TYPE) {
        if (g_gametype->integer <= GT_TEAM) {
            m_bAllowRespawns  = true;
            m_bRoundBasedGame = false;
        } else {
            if (g_gametype->integer == GT_TOW || g_gametype->integer == GT_LIBERATION) {
                m_bAllowRespawns = true;
                m_bRoundBasedGame = true;
            } else {
                m_bAllowRespawns = false;
                m_bRoundBasedGame = true;
            }

            g_tempaxisscore       = gi.Cvar_Get("g_tempaxisscore", "0", 0);
            g_tempaxiswinsinrow   = gi.Cvar_Get("g_tempaxiswinsinrow", "0", 0);
            g_tempalliesscore     = gi.Cvar_Get("g_tempalliesscore", "0", 0);
            g_tempallieswinsinrow = gi.Cvar_Get("g_tempallieswinsinrow", "0", 0);

            m_team_axis.m_teamwins        = g_tempaxisscore->integer;
            m_team_axis.m_wins_in_a_row   = g_tempaxiswinsinrow->integer;
            m_team_allies.m_teamwins      = g_tempalliesscore->integer;
            m_team_allies.m_wins_in_a_row = g_tempallieswinsinrow->integer;

            gi.cvar_set("g_tempaxisscore", "0");
            gi.cvar_set("g_tempaxiswinsinrow", "0");
            gi.cvar_set("g_tempalliesscore", "0");
            gi.cvar_set("g_tempaxiswinsinrow", "0");

            m_iTotalMapTime = gi.Cvar_Get("g_tempmaptime", "0", 0)->integer;
            gi.cvar_set("g_tempmaptime", "0");
        }
    }
}

bool DM_Manager::CheckEndMatch()
{
    if (fraglimit) {
        if (fraglimit->integer < 0) {
            gi.cvar_set("fraglimit", "0");
        }
        if (fraglimit->integer > 10000) {
            gi.cvar_set("fraglimit", "10000");
        }
        fraglimit = gi.Cvar_Get("fraglimit", "0", 0);
    }

    if (timelimit) {
        if (timelimit->integer < 0) {
            gi.cvar_set("timelimit", "0");
        }
        // 180 minutes maximum
        if (timelimit->integer > 10800) {
            gi.cvar_set("timelimit", "10800");
        }
        timelimit = gi.Cvar_Get("timelimit", "0", 0);
    }

    if (!m_bRoundBasedGame || g_gametype->integer == GT_TOW || g_gametype->integer == GT_LIBERATION) {
        if (g_gametype->integer == GT_TOW) {
            cvar_t *g_TOW_winstate = gi.Cvar_Get("g_TOW_winstate", "", 0);
            if (!g_TOW_winstate || !g_TOW_winstate->integer) {
                int roundLimit = GetRoundLimit();

                if (!level.m_bIgnoreClock && roundLimit > 0 && level.time >= m_iDefaultRoundLimit * 60 + m_fRoundTime) {
                    switch (m_csTeamClockSide) {
                    case STRING_AXIS:
                        gi.cvar_set("g_TOW_winstate", "1");
                        TeamWin(TEAM_AXIS);
                        break;
                    case STRING_ALLIES:
                        gi.cvar_set("g_TOW_winstate", "2");
                        TeamWin(TEAM_ALLIES);
                        break;
                    default:
                        gi.cvar_set("g_TOW_winstate", "3");
                        TeamWin(TEAM_NONE);
                    }

                    return true;
                }

                if (m_team_allies.IsDead()) {
                    gi.cvar_set("g_TOW_winstate", "1");
                    TeamWin(TEAM_AXIS);
                    return true;
                }

                if (m_team_axis.IsDead()) {
                    gi.cvar_set("g_TOW_winstate", "2");
                    TeamWin(TEAM_ALLIES);
                    return true;
                }
            }

            if (fraglimit->integer && TeamHitScoreLimit()) {
                G_BeginIntermission2();
                return true;
            } else {
                return false;
            }
        } else if (g_gametype->integer == GT_LIBERATION) {
            if (fraglimit->integer && TeamHitScoreLimit()) {
                G_BeginIntermission2();
                return true;
            } else {
                return false;
            }
        }

        if (fraglimit->integer) {
            if (g_gametype->integer >= GT_TEAM) {
                if (TeamHitScoreLimit()) {
                    G_BeginIntermission2();
                    return true;
                }
            } else if (PlayerHitScoreLimit()) {
                G_BeginIntermission2();
                return true;
            }
        } else if (timelimit->integer && level.inttime >= 60000 * timelimit->integer) {
            G_BeginIntermission2();
            return true;
        } else {
            return false;
        }
    }

    if (m_fRoundEndTime > 0.0f) {
        return true;
    }

    if (m_fRoundTime <= 0.f) {
        return false;
    }

    if (fraglimit->integer && TeamHitScoreLimit()) {
        G_BeginIntermission2();
        return true;
    }

    if (AllowRespawn() || (!m_team_axis.IsDead() && !m_team_allies.IsDead())) {
        int roundLimit = GetRoundLimit();

        if (roundLimit > 0 && level.time >= m_iDefaultRoundLimit * 60 + m_fRoundTime) {
            if (m_csTeamBombPlantSide != STRING_DRAW) {
                if (m_bIgnoringClockForBomb) {
                    if (m_iNumBombsPlanted > 0) {
                        return false;
                    }

                    m_bIgnoringClockForBomb = false;
                } else if (m_iNumBombsPlanted > 0) {
                    G_PrintToAllClients("A bomb is still set!");
                    m_bIgnoringClockForBomb = true;
                    return false;
                }
            }

            switch (m_csTeamClockSide) {
            case STRING_ALLIES:
                TeamWin(TEAM_ALLIES);
                return true;
            case STRING_AXIS:
                TeamWin(TEAM_AXIS);
                return true;
            case STRING_KILLS:
                if (m_team_allies.TotalPlayersKills() > m_team_axis.TotalPlayersKills()) {
                    TeamWin(TEAM_ALLIES);
                } else if (m_team_axis.TotalPlayersKills() > m_team_allies.TotalPlayersKills()) {
                    TeamWin(TEAM_AXIS);
                } else {
                    TeamWin(TEAM_NONE);
                }
                return true;
            default:
                TeamWin(TEAM_NONE);
                return true;
            }
        }
    } else {
        DM_Team *pBombTeam;
        DM_Team *pNonBombTeam;

        if (g_gametype->integer != GT_OBJECTIVE) {
            EndRound();
            return true;
        }

        if (m_csTeamBombPlantSide == STRING_DRAW) {
            EndRound();
            return true;
        }

        if (m_csTeamBombPlantSide == STRING_AXIS) {
            pBombTeam    = &m_team_axis;
            pNonBombTeam = &m_team_allies;
        } else {
            pBombTeam    = &m_team_allies;
            pNonBombTeam = &m_team_axis;
        }

        if (!pBombTeam->IsDead()) {
            EndRound();
            return true;
        }

        if (m_iNumBombsPlanted <= 0) {
            m_bIgnoringClockForBomb = qfalse;

            if (pNonBombTeam->IsDead() && m_iNumTargetsDestroyed < m_iNumTargetsToDestroy) {
                TeamWin(pNonBombTeam->m_teamnumber);
                return true;
            } else {
                EndRound();
                return true;
            }

        } else if (pNonBombTeam->IsDead()) {
            if (m_iNumBombsPlanted >= m_iNumTargetsToDestroy - m_iNumTargetsDestroyed) {
                TeamWin(pBombTeam->m_teamnumber);
            } else {
                TeamWin(pNonBombTeam->m_teamnumber);
            }
            return true;
        } else if (m_iNumBombsPlanted >= m_iNumTargetsToDestroy - m_iNumTargetsDestroyed) {
            if (!m_bIgnoringClockForBomb) {
                G_PrintToAllClients("A bomb is still set!");
                m_bIgnoringClockForBomb = true;
            }
        } else {
            EndRound();
            return true;
        }
    }

    return false;
}

bool DM_Manager::TeamHitScoreLimit(void)
{
    if (m_teams.NumObjects() < 1) {
        return false;
    }

    for (int i = 1; i <= m_teams.NumObjects(); i++) {
        if (m_teams.ObjectAt(i)->m_teamwins >= fraglimit->integer) {
            return true;
        }
    }

    return false;
}

bool DM_Manager::PlayerHitScoreLimit(void)
{
    if (PlayerCount() < 1) {
        return false;
    }

    if (g_gametype->integer >= GT_TEAM_ROUNDS) {
        return false;
    }

    for (int i = 1; i <= PlayerCount(); i++) {
        if (m_players.ObjectAt(i)->GetNumKills() >= fraglimit->integer) {
            return true;
        }
    }

    return false;
}

void DM_Manager::EventDoRoundTransition(Event *ev)
{
    if (!m_iTeamWin) {
        if (G_FindClass(NULL, "projectile")) {
            // wait for any projectile to explode
            PostEvent(EV_DM_Manager_DoRoundTransition, 1.0f, 0);
            return;
        }

        if (!m_team_allies.IsDead() && !m_team_axis.IsDead()) {
            if (m_csTeamClockSide != STRING_KILLS) {
                if (m_csTeamClockSide != STRING_DRAW) {
                    gi.Printf(
                        "WARNING: DM_Manager::EventDoRoundTransition received but no winner could be determined when "
                        "there should've been\n"
                    );
                }

                TeamWin(TEAM_NONE);
            } else {
                if (m_team_allies.TotalPlayersKills() > m_team_axis.TotalPlayersKills()) {
                    TeamWin(TEAM_ALLIES);
                } else if (m_team_axis.TotalPlayersKills() > m_team_allies.TotalPlayersKills()) {
                    TeamWin(TEAM_AXIS);
                } else {
                    TeamWin(TEAM_NONE);
                }
            }
        } else if (m_team_allies.IsDead() && m_team_axis.IsDead()) {
            TeamWin(TEAM_NONE);
        } else if (m_team_axis.IsDead()) {
            TeamWin(TEAM_ALLIES);
        } else {
            TeamWin(TEAM_AXIS);
        }
    }

    if (m_iTeamWin == TEAM_AXIS) {
        G_CenterPrintToAllClients(va("\n\n\n%s\n", gi.LV_ConvertString("Axis win!\n")));
        G_PrintToAllClients(va("%s\n", gi.LV_ConvertString("Axis win!\n")));

        // Play the axis victory sound
        world->Sound("den_victory_v");

        Unregister(STRING_AXISWIN);
    } else if (m_iTeamWin == TEAM_ALLIES) {
        G_CenterPrintToAllClients(va("\n\n\n%s\n", gi.LV_ConvertString("Allies win!\n")));
        G_PrintToAllClients(va("%s\n", gi.LV_ConvertString("Allies win!\n")));

        // Play the allies victory sound
        world->Sound("dfr_victory_v");

        Unregister(STRING_ALLIESWIN);
    } else {
        G_CenterPrintToAllClients(va("\n\n\n%s\n", gi.LV_ConvertString("It's a draw!\n")));
        G_PrintToAllClients(va("%s\n", gi.LV_ConvertString("It's a draw!\n")));

        Unregister(STRING_DRAW);
    }

    G_DisplayScoresToAllClients();
    PostEvent(EV_DM_Manager_FinishRoundTransition, 3.0f);
}

void DM_Manager::EventFinishRoundTransition(Event *ev)
{
    gentity_t *ent;
    int        i;

    CancelEventsOfType(EV_DM_Manager_FinishRoundTransition);

    if (timelimit->integer && m_iTotalMapTime + level.inttime >= 60000 * timelimit->integer) {
        G_BeginIntermission2();
        return;
    }

    gi.cvar_set("g_tempaxisscore", va("%d", m_team_axis.m_teamwins));
    gi.cvar_set("g_tempaxiswinsinrow", va("%d", m_team_axis.m_wins_in_a_row));
    gi.cvar_set("g_tempalliesscore", va("%d", m_team_allies.m_teamwins));
    gi.cvar_set("g_tempallieswinsinrow", va("%d", m_team_allies.m_wins_in_a_row));
    gi.cvar_set("g_tempmaptime", va("%d", m_iTotalMapTime + level.inttime));

    for (i = 0, ent = g_entities; i < game.maxclients; ent++, i++) {
        if (!ent->inuse || !ent->entity) {
            continue;
        }

        Player *p                   = (Player *)ent->entity;
        p->client->pers.round_kills = p->GetNumDeaths();
    }

    if (g_gametype->integer == GT_TOW) {
        g_TOWObjectiveMan.Reset();
    }

    gi.SendConsoleCommand("restart\n");

    g_teamSpawnClock.Reset();
}

int DM_Manager::PlayerCount(void) const
{
    return m_players.NumObjects();
}

Player *DM_Manager::GetPlayer(int index) const
{
    return m_players.ObjectAt(index);
}

teamtype_t DM_Manager::GetAutoJoinTeam(void)
{
    int allies = m_team_allies.m_players.NumObjects();
    int axis   = m_team_axis.m_players.NumObjects();

    if (allies < axis) {
        return TEAM_ALLIES;
    } else if (allies == axis) {
        return (G_Random() >= 0.5f) ? TEAM_ALLIES : TEAM_AXIS;
    } else {
        return TEAM_AXIS;
    }
}

void DM_Manager::TeamWin(int teamnum)
{
    DM_Team *pTeamWin;
    DM_Team *pTeamLose;

    if (m_iTeamWin) {
        return;
    }

    if (teamnum == TEAM_AXIS) {
        pTeamWin  = &m_team_axis;
        pTeamLose = &m_team_allies;
    } else if (teamnum == TEAM_ALLIES) {
        pTeamWin  = &m_team_allies;
        pTeamLose = &m_team_axis;
    } else {
        pTeamWin  = NULL;
        pTeamLose = NULL;
    }

    if (pTeamWin) {
        pTeamWin->TeamWin();
    }

    if (pTeamLose) {
        pTeamLose->TeamLoss();
    }

    m_iTeamWin = teamnum ? teamnum : -1;
    EndRound();
}

void DM_Manager::StartRound(void)
{
    gentity_t *ent;
    int        i;
    Player    *player;

    m_fRoundTime = level.time;
    if (m_fRoundTime < 0.1f) {
        m_fRoundTime = 0.1f;
    }

    m_fRoundEndTime = 0.0f;
    m_bRoundActive = true;

    // respawn all players
    for (i = 0, ent = g_entities; i < game.maxclients; i++, ent++) {
        if (!ent->inuse || !ent->client || !ent->entity) {
            continue;
        }

        player = (Player *)ent->entity;

        if ((player->GetTeam() == TEAM_ALLIES || player->GetTeam() == TEAM_AXIS) && !player->IsDead()
            && !player->IsSpectator()) {
            player->PostEvent(EV_Player_Respawn, 0);
        }
    }

    level.RemoveWaitTill(STRING_ROUNDSTART);
    level.Unregister(STRING_ROUNDSTART);
    gi.setConfigstring(CS_WARMUP, va("%.0f", GetMatchStartTime()));
}

void DM_Manager::EndRound()
{
    m_bRoundActive = false;

    if (m_fRoundEndTime <= 0) {
        m_fRoundEndTime = level.time;
        PostEvent(EV_DM_Manager_DoRoundTransition, 2);
    }
}

bool DM_Manager::AllowRespawn() const
{
    if (GameAllowsRespawns()) {
        return true;
    }

    if (g_gametype->integer <= GT_TEAM) {
        return false;
    }

    if (!m_team_axis.m_players.NumObjects() && !m_team_axis.m_bHasSpawnedPlayers) {
        return true;
    }

    if (!m_team_allies.m_players.NumObjects() && !m_team_allies.m_bHasSpawnedPlayers) {
        return true;
    }

    return false;
}

bool DM_Manager::WaitingForPlayers(void) const
{
    if (g_gametype->integer <= GT_TEAM) {
        return false;
    }

    if (m_team_axis.IsEmpty() || m_team_allies.IsEmpty()) {
        return true;
    }

    if (!m_team_axis.IsReady() || !m_team_allies.IsReady()) {
        return true;
    }

    if (m_team_axis.IsDead() || m_team_allies.IsDead()) {
        return true;
    }

    return false;
}

bool DM_Manager::IsGameActive(void) const
{
    return !GameHasRounds() || m_fRoundTime > 0;
}

float DM_Manager::GetMatchStartTime(void)
{
    int totalnotready;

    if (g_gametype->integer <= GT_TEAM) {
        return m_fRoundTime;
    }

    if (g_gametype->integer == GT_TEAM_ROUNDS || g_gametype->integer == GT_OBJECTIVE || g_gametype->integer == GT_LIBERATION) {
        if (m_fRoundTime > 0 && (m_team_allies.IsEmpty() || m_team_allies.IsEmpty())) {
            m_fRoundTime = 0;
            return -1;
        }
    }

    if (m_fRoundTime > 0) {
        return m_fRoundTime;
    }

    if (m_team_allies.IsEmpty() || m_team_axis.IsEmpty()) {
        return -1;
    }

    totalnotready = m_team_allies.NumNotReady() + m_team_axis.NumNotReady();
    if (totalnotready > 0) {
        return -1 - totalnotready;
    } else {
        return m_fRoundTime;
    }
}

int DM_Manager::GetRoundLimit() const
{
    int round_limit = roundlimit->integer;

    if (!round_limit) {
        round_limit = m_iDefaultRoundLimit;
    }

    return round_limit;
}

void DM_Manager::SetDefaultRoundLimit(int roundlimit)
{
    m_iDefaultRoundLimit = roundlimit;
}

const_str DM_Manager::GetClockSide(void) const
{
    return m_csTeamClockSide;
}

void DM_Manager::SetClockSide(const_str s)
{
    m_csTeamClockSide = s;
}

const_str DM_Manager::GetBombPlantTeam(void) const
{
    return m_csTeamBombPlantSide;
}

void DM_Manager::SetBombPlantTeam(const_str s)
{
    m_csTeamBombPlantSide = s;
}

int DM_Manager::GetTargetsToDestroy(void) const
{
    return m_iNumTargetsToDestroy;
}

void DM_Manager::SetTargetsToDestroy(int targets)
{
    m_iNumTargetsToDestroy = targets;
}

int DM_Manager::GetTargetsDestroyed(void) const
{
    return m_iNumTargetsDestroyed;
}

void DM_Manager::SetTargetsDestroyed(int targets)
{
    m_iNumTargetsDestroyed = targets;
}

int DM_Manager::GetBombsPlanted(void) const
{
    return m_iNumBombsPlanted;
}

void DM_Manager::SetBombsPlanted(int num)
{
    m_iNumBombsPlanted = num;
}

void DM_Manager::StopTeamRespawn(eController controller)
{
    if (controller == CONTROLLER_AXIS) {
        m_bAllowAxisRespawn = false;
    } else if (controller == CONTROLLER_ALLIES) {
        m_bAllowAlliedRespawn = false;
    }
}

bool DM_Manager::AllowTeamRespawn(int teamnum) const
{
    if (teamnum == TEAM_ALLIES) {
        return m_bAllowAlliedRespawn;
    } else if (teamnum == TEAM_AXIS) {
        return m_bAllowAxisRespawn;
    }

    return false;
}

int DM_Manager::GetTeamSpawnTimeLeft() const
{
    if (sv_team_spawn_interval->integer <= 0) {
        return -1;
    }

    if (g_gametype->integer != GT_TOW && (g_gametype->integer > GT_TOW || g_gametype->integer != GT_TEAM)) {
        return 0;
    }

    if (g_gametype->integer == GT_TEAM || g_gametype->integer == GT_TOW || g_gametype->integer == GT_LIBERATION) {
        return g_teamSpawnClock.GetSecondsLeft();
    }

    return 0;
}

DM_Team *DM_Manager::GetTeam(str name)
{
    if (name.icmp("spectator") == 0) {
        return &m_team_spectator;
    } else if (name.icmp("freeforall") == 0) {
        return &m_team_freeforall;
    } else if (name.icmp("allies") == 0) {
        return &m_team_allies;
    } else if (name.icmp("axis") == 0) {
        return &m_team_axis;
    } else {
        ScriptError("Invalid team %s !\n", name.c_str());
    }

    return NULL;
}

DM_Team *DM_Manager::GetTeam(teamtype_t team)
{
    switch (team) {
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

void DM_Manager::InsertEntry(const char *entry)
{
    size_t len = strlen(entry);

    if (scoreLength + len < MAX_STRING_CHARS) {
        strcpy(scoreString + scoreLength, entry);

        scoreLength += len;
        scoreEntries++;
    }
}

void DM_Manager::InsertEntryNoCount(const char* entry)
{
	size_t len = strlen(entry);

	if (scoreLength + len < MAX_STRING_CHARS) {
		strcpy(scoreString + scoreLength, entry);

		scoreLength += len;
	}
}

void DM_Manager::InsertEmpty(void)
{
    if (g_gametype->integer > GT_FFA) {
        InsertEntry("-2 \"\" \"\" \"\" \"\" \"\" ");
    } else {
        InsertEntry("-1 \"\" \"\" \"\" \"\" ");
    }
}

void DM_Manager::BuildTeamInfo(DM_Team *dmTeam)
{
    if (g_protocol >= protocol_e::PROTOCOL_MOHTA_MIN) {
        BuildTeamInfo_ver15(dmTeam);
    } else {
        BuildTeamInfo_ver6(dmTeam);
    }
}

void DM_Manager::BuildTeamInfo_ver6(DM_Team *dmTeam)
{
    int     iPing = 0;
    int     iKills;
    int     iDeaths;
    int     iNumPlayers = 0;
    Player *pTeamPlayer;
    char    entry[MAX_STRING_TOKENS];

    for (int i = iNumPlayers; i > 0; i--) {
        pTeamPlayer = dmTeam->m_players.ObjectAt(i);

        if (pTeamPlayer->IsSubclassOfBot()) {
            continue;
        }

        iNumPlayers++;
        iPing += pTeamPlayer->client->ps.ping;
    }

    if (iNumPlayers > 0) {
        iPing /= iNumPlayers;
    }

    if (g_gametype->integer >= GT_TEAM_ROUNDS) {
        iKills  = dmTeam->m_wins_in_a_row;
        iDeaths = dmTeam->m_teamwins;
    } else {
        iKills  = dmTeam->m_iKills;
        iDeaths = dmTeam->m_iDeaths;
    }

    if (g_gametype->integer > GT_FFA) {
        if (dmTeam->m_teamnumber > TEAM_FREEFORALL) {
            Com_sprintf(entry, sizeof(entry), "%i %i %i %i \"\" %i ", -1, dmTeam->m_teamnumber, iKills, iDeaths, iPing);
        } else {
            Com_sprintf(entry, sizeof(entry), "%i %i \"\" \"\" \"\" \"\" ", -1, dmTeam->m_teamnumber);
        }
    } else {
        Com_sprintf(entry, sizeof(entry), "%i \"\" \"\" \"\" \"\" \"\" ", -1 - dmTeam->m_teamnumber);
    }

    InsertEntry(entry);
}

void DM_Manager::BuildTeamInfo_ver15(DM_Team *dmTeam)
{
    int     iPing = 0;
    int     iKills;
    int     iDeaths;
    int     iNumPlayers = 0;
    Player *pTeamPlayer;
    char    entry[MAX_STRING_TOKENS];

    for (int i = iNumPlayers; i > 0; i--) {
        pTeamPlayer = dmTeam->m_players.ObjectAt(i);

        if (pTeamPlayer->IsSubclassOfBot()) {
            continue;
        }

        iNumPlayers++;
        iPing += pTeamPlayer->client->ps.ping;
    }

    if (iNumPlayers > 0) {
        iPing /= iNumPlayers;
    }

    if (g_gametype->integer >= GT_TEAM_ROUNDS) {
        iKills  = dmTeam->m_wins_in_a_row;
        iDeaths = dmTeam->m_teamwins;
    } else {
        iKills  = dmTeam->m_iKills;
        iDeaths = dmTeam->m_iDeaths;
    }

    if (g_gametype->integer >= GT_TEAM) {
        if (dmTeam->m_teamnumber > TEAM_FREEFORALL) {
            Com_sprintf(
                entry,
                sizeof(entry),
                "%i %i %i %i %i \"\" %i ",
                -1,
                dmTeam->m_teamnumber,
                dmTeam->m_players.NumObjects(),
                iKills,
                iDeaths,
                iPing
            );
        } else {
            Com_sprintf(entry, sizeof(entry), "%i %i \"\" \"\" \"\" \"\" \"\" ", -1, dmTeam->m_teamnumber);
        }
    } else {
        Com_sprintf(entry, sizeof(entry), "%i \"\" \"\" \"\" \"\" ", -1 - dmTeam->m_teamnumber);
    }

    InsertEntry(entry);
}

void DM_Manager::BuildPlayerTeamInfo(DM_Team *dmTeam, int *iPlayerList, DM_Team *ignoreTeam)
{
    char    entry[MAX_STRING_CHARS];
    Player *pTeamPlayer;

    for (int i = 0; i < game.maxclients; i++) {
        if (iPlayerList[i] == -1) {
            break;
        }

        pTeamPlayer = (Player *)G_GetEntity(iPlayerList[i]);

        if (dmTeam != NULL && pTeamPlayer->GetDM_Team() != dmTeam) {
            continue;
        }

        if (ignoreTeam != NULL && pTeamPlayer->GetDM_Team() == ignoreTeam) {
            continue;
        }

        if (g_gametype->integer >= GT_TEAM) {
            Com_sprintf(
                entry,
                sizeof(entry),
                "%i %i %i %i %s %s ",
                pTeamPlayer->client->ps.clientNum,
                IsAlivePlayer(pTeamPlayer) ? pTeamPlayer->GetTeam()
                                           : -pTeamPlayer->GetTeam(), // negative team means death
                pTeamPlayer->GetNumKills(),
                pTeamPlayer->GetNumDeaths(),
                G_TimeString(level.svsFloatTime - pTeamPlayer->edict->client->pers.enterTime),
                pTeamPlayer->IsSubclassOfBot() ? "bot" : va("%d", pTeamPlayer->client->ps.ping)
            );
        } else {
            Com_sprintf(
                entry,
                sizeof(entry),
                "%i %i %i %s %s ",
                pTeamPlayer->client->ps.clientNum,
                pTeamPlayer->GetNumKills(),
                pTeamPlayer->GetNumDeaths(),
                G_TimeString(level.svsFloatTime - pTeamPlayer->edict->client->pers.enterTime),
                pTeamPlayer->IsSubclassOfBot() ? "bot" : va("%d", pTeamPlayer->client->ps.ping)
            );
        }

        InsertEntry(entry);
    }
}

bool DM_Manager::IsAlivePlayer(Player *player) const
{
    return !player->IsDead() && !player->IsSpectator() && !player->IsInJail() || player->GetDM_Team() == &m_team_spectator;
}

CTeamSpawnClock::CTeamSpawnClock()
{
    nextSpawnTime = 0;
}

void CTeamSpawnClock::Reset()
{
    nextSpawnTime = sv_team_spawn_interval->value;
}

void CTeamSpawnClock::Restart()
{
    nextSpawnTime = level.time + sv_team_spawn_interval->value;
}

int CTeamSpawnClock::GetSecondsLeft()
{
    int timeLeft;

    timeLeft = ceil(nextSpawnTime - level.time);
    if (timeLeft <= -1) {
        Restart();
    }

    if (timeLeft < 0) {
        return 0;
    }

    return timeLeft;
}
