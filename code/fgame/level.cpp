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

#include "g_local.h"
#include "g_spawn.h"
#include "g_bot.h"
#include "level.h"
#include "parm.h"
#include "navigate.h"
#include "gravpath.h"
#include "entity.h"
#include "dm_manager.h"
#include "player.h"
#include "Entities.h"
#include "health.h"

#include "scriptmaster.h"
#include "scriptthread.h"
#include "scriptvariable.h"
#include "scriptexception.h"

#include <cfloat>

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
    "internal usage.",
    EV_NORMAL
);

Event EV_Level_GetAlarm
(
    "alarm",
    EV_DEFAULT,
    NULL,
    NULL,
    "zero = global level alarm off,"
    "non-zero = alarm on",
    EV_GETTER
);

Event EV_Level_SetAlarm
(
    "alarm",
    EV_DEFAULT,
    "i",
    "alarm_status",
    "zero = global level alarm off,"
    "non-zero = alarm on",
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

Event EV_Level_SetNoDropWeapons
(
    "nodropweapon",
    EV_DEFAULT,
    "i",
    "alarm_status",
    "zero = automatically drop weapons according to cvars, non-zero = don't autodrop weapons (like hard mode)",
    EV_SETTER
);

Event EV_Level_GetRoundStarted
(
    "roundstarted",
    EV_DEFAULT,
    NULL,
    NULL,
    "non-zero if round has started",
    EV_GETTER
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

Event EV_Level_SetDMRespawning2(
    "dmrespawning",
    EV_DEFAULT,
    "i",
    "allow_respawn",
    "set to 1 to turn on wave-based DM,"
    "to 0 to disable respawns within a round"
);

Event EV_Level_GetDMRoundLimit(
    "dmroundlimit",
    EV_DEFAULT,
    NULL,
    NULL,
    "gets the actual roundlimit, in minutes; may be 'roundlimit' cvar or the default round limit",
    EV_GETTER
);

Event EV_Level_SetDMRoundLimit(
    "dmroundlimit",
    EV_DEFAULT,
    "i",
    "roundlimit",
    "sets the default roundlimit, in minutes; can be overridden by 'roundlimit' cvar",
    EV_SETTER
);

Event EV_Level_SetDMRoundLimit2(
    "dmroundlimit",
    EV_DEFAULT,
    "i",
    "roundlimit",
    "sets the default roundlimit,"
    "in minutes; can be overridden by 'roundlimit' cvar"
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

Event EV_Level_SetClockSide2(
    "clockside",
    EV_DEFAULT,
    "s",
    "axis_allies_draw_kills",
    "Sets which side the clock is on... 'axis' or 'allies' win when time is up, 'kills' gives the win to the team with "
    "more live members,"
    "'draw' no one wins"
);

Event EV_Level_GetBombPlantTeam
(
    "planting_team",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets which is planting the bomb,"
    "'axis' or 'allies'",
    EV_GETTER
);

Event EV_Level_SetBombPlantTeam
(
    "planting_team",
    EV_DEFAULT,
    "s",
    "axis_or_allies",
    "Sets which is planting the bomb,"
    "'axis' or 'allies'",
    EV_SETTER
);

Event EV_Level_SetBombPlantTeam2(
    "planting_team",
    EV_DEFAULT,
    "s",
    "axis_allies_draw_kills",
    "which is planting the bomb,"
    "'axis' or 'allies'"
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
    "the number of bomb targets that must be destroyed",
    EV_NORMAL
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
    "the number of bomb targets that have been destroyed",
    EV_NORMAL
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
    "the number of bombs that are set",
    EV_NORMAL
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

Event EV_Level_GetObjectiveBased
(
    "objectivebased",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets wether or not the game is currently objective based or not",
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

Event EV_Level_AddBadPlace
(
    "badplace",
    EV_DEFAULT,
    "svfSF",
    "name origin radius [team] [duration]",
    "Enables a 'bad place' for AI of team 'american', 'german', or (default) 'both' to avoid, and optionally gives it "
    "a duration",
    EV_NORMAL
);

Event EV_Level_RemoveBadPlace
(
    "removebadplace",
    EV_DEFAULT,
    "s",
    "name",
    "Removes a 'bad place'",
    EV_NORMAL
);

Event EV_Level_IgnoreClock
(
    "ignoreclock",
    EV_DEFAULT,
    "i",
    "ignoreclock",
    "Tells a level weather or not to ignore the clock",
    EV_NORMAL
);

extern Event EV_Entity_Start;

CLASS_DECLARATION(Listener, Level, NULL) {
    {&EV_Level_GetTime,              &Level::GetTime                 },
    {&EV_Level_GetTotalSecrets,      &Level::GetTotalSecrets         },
    {&EV_Level_GetFoundSecrets,      &Level::GetFoundSecrets         },
    {&EV_Level_PreSpawnSentient,     &Level::PreSpawnSentient        },
    {&EV_Level_GetAlarm,             &Level::GetAlarm                },
    {&EV_Level_SetAlarm,             &Level::SetAlarm                },
    {&EV_Level_SetNoDropHealth,      &Level::SetNoDropHealth         },
    {&EV_Level_SetNoDropWeapons,     &Level::SetNoDropWeapons        },
    {&EV_Level_GetRoundStarted,      &Level::EventGetRoundStarted    },
    {&EV_Level_GetLoopProtection,    &Level::GetLoopProtection       },
    {&EV_Level_SetLoopProtection,    &Level::SetLoopProtection       },
    {&EV_Level_GetPapersLevel,       &Level::GetPapersLevel          },
    {&EV_Level_SetPapersLevel,       &Level::SetPapersLevel          },
    {&EV_Level_GetDMRespawning,      &Level::EventGetDMRespawning    },
    {&EV_Level_SetDMRespawning,      &Level::EventSetDMRespawning    },
    {&EV_Level_SetDMRespawning2,     &Level::EventSetDMRespawning    },
    {&EV_Level_GetDMRoundLimit,      &Level::EventGetDMRoundLimit    },
    {&EV_Level_SetDMRoundLimit,      &Level::EventSetDMRoundLimit    },
    {&EV_Level_SetDMRoundLimit2,     &Level::EventSetDMRoundLimit    },
    {&EV_Level_GetClockSide,         &Level::EventGetClockSide       },
    {&EV_Level_SetClockSide,         &Level::EventSetClockSide       },
    {&EV_Level_SetClockSide2,        &Level::EventSetClockSide       },
    {&EV_Level_GetBombPlantTeam,     &Level::EventGetBombPlantTeam   },
    {&EV_Level_SetBombPlantTeam,     &Level::EventSetBombPlantTeam   },
    {&EV_Level_SetBombPlantTeam2,    &Level::EventSetBombPlantTeam   },
    {&EV_Level_GetTargetsToDestroy,  &Level::EventGetTargetsToDestroy},
    {&EV_Level_SetTargetsToDestroy,  &Level::EventSetTargetsToDestroy},
    {&EV_Level_SetTargetsToDestroy2, &Level::EventSetTargetsToDestroy},
    {&EV_Level_GetTargetsDestroyed,  &Level::EventGetTargetsDestroyed},
    {&EV_Level_SetTargetsDestroyed,  &Level::EventSetTargetsDestroyed},
    {&EV_Level_SetTargetsDestroyed2, &Level::EventSetTargetsDestroyed},
    {&EV_Level_GetBombsPlanted,      &Level::EventGetBombsPlanted    },
    {&EV_Level_SetBombsPlanted,      &Level::EventSetBombsPlanted    },
    {&EV_Level_SetBombsPlanted2,     &Level::EventSetBombsPlanted    },
    {&EV_Level_GetRoundBased,        &Level::EventGetRoundBased      },
    {&EV_Level_GetObjectiveBased,    &Level::EventGetObjectiveBased  },
    {&EV_Level_Rain_Density_Set,     &Level::EventRainDensitySet     },
    {&EV_Level_Rain_Density_Get,     &Level::EventRainDensityGet     },
    {&EV_Level_Rain_Speed_Set,       &Level::EventRainSpeedSet       },
    {&EV_Level_Rain_Speed_Get,       &Level::EventRainSpeedGet       },
    {&EV_Level_Rain_Speed_Vary_Set,  &Level::EventRainSpeedVarySet   },
    {&EV_Level_Rain_Speed_Vary_Get,  &Level::EventRainSpeedVaryGet   },
    {&EV_Level_Rain_Slant_Set,       &Level::EventRainSlantSet       },
    {&EV_Level_Rain_Slant_Get,       &Level::EventRainSlantGet       },
    {&EV_Level_Rain_Length_Set,      &Level::EventRainLengthSet      },
    {&EV_Level_Rain_Length_Get,      &Level::EventRainLengthGet      },
    {&EV_Level_Rain_Min_Dist_Set,    &Level::EventRainMin_DistSet    },
    {&EV_Level_Rain_Min_Dist_Get,    &Level::EventRainMin_DistGet    },
    {&EV_Level_Rain_Width_Set,       &Level::EventRainWidthSet       },
    {&EV_Level_Rain_Width_Get,       &Level::EventRainWidthGet       },
    {&EV_Level_Rain_Shader_Set,      &Level::EventRainShaderSet      },
    {&EV_Level_Rain_Shader_Get,      &Level::EventRainShaderGet      },
    {&EV_Level_Rain_NumShaders_Set,  &Level::EventRainShaderSet      },
    {&EV_Level_Rain_NumShaders_Get,  &Level::EventRainShaderGet      },
    {&EV_Level_AddBadPlace,          &Level::EventAddBadPlace        },
    {&EV_Level_RemoveBadPlace,       &Level::EventRemoveBadPlace     },
    {&EV_Level_IgnoreClock,          &Level::EventIgnoreClock        },
    {NULL,                           NULL                            }
};

void Level::GetTime(Event *ev)
{
    ev->AddFloat(level.time);
}

void Level::GetTotalSecrets(Event *ev)
{
    ev->AddInteger(total_secrets);
}

void Level::GetFoundSecrets(Event *ev)
{
    ev->AddInteger(found_secrets);
}

Level::Level()
{
    Init();
}

Level::~Level() {}

void Level::Init(void)
{
    m_HeadSentient[1] = NULL;
    m_HeadSentient[0] = NULL;

    spawn_entnum = -1;

    current_map = NULL;

    framenum  = 0;
    time      = 0;
    frametime = 0;

    level_name = "";
    mapname    = "";
    spawnpoint = "";
    nextmap    = "";

    total_secrets = 0;
    found_secrets = 0;

    m_iCuriousVoiceTime    = 0;
    m_iAttackEntryAnimTime = 0;

    playerfrozen = false;

    intermissiontime = 0.0f;
    exitintermission = 0;

    memset(&impact_trace, 0, sizeof(trace_t));

    cinematic = false;
    ai_on     = true;

    near_exit      = false;
    mission_failed = false;

    m_bAlarm       = false;
    m_iPapersLevel = 0;

    died_already = false;

    water_color = vec_zero;
    lava_color  = vec_zero;

    lava_alpha  = 0.0f;
    water_alpha = 0.0f;

    saved_soundtrack   = "";
    current_soundtrack = "";

    automatic_cameras.ClearObjectList();

    m_fade_time_start = 0.0f;
    m_fade_time       = -1.0f;
    m_fade_color      = vec_zero;
    m_fade_style      = additive;

    m_fade_alpha = 0;

    m_letterbox_fraction   = 0;
    m_letterbox_time       = -1;
    m_letterbox_time_start = 0;
    m_letterbox_dir        = letterbox_out;

    m_numArenas = 1;

    m_voteTime     = 0;
    m_nextVoteTime = 0;
    m_voteYes      = 0;
    m_voteNo       = 0;
    m_numVoters    = 0;

    m_LoopProtection = true;
    m_LoopDrop       = true;

    m_letterbox_time = -1.0f;

    m_vObjectiveLocation       = vec_zero;
    m_vAlliedObjectiveLocation = vec_zero;
    m_vAxisObjectiveLocation   = vec_zero;

    svsEndTime = 0;

    earthquake_magnitude = 0;

    mHealthPopCount = 0;
    mbNoDropHealth  = false;
    mbNoDropWeapons = false;

    spawning          = false;
    m_bIgnoreClock    = false;
    svsStartFloatTime = 0;

    m_fLandmarkXDistMin = 0;
    m_fLandmarkXDistMax = 0;
    m_fLandmarkYDistMin = 0;
    m_fLandmarkYDistMax = 0;
    m_pLandmarks        = NULL;
    m_iMaxLandmarks     = 0;
    m_iLandmarksCount   = 0;

    m_badPlaces.ClearObjectList();

    m_pAIStats = NULL;

    m_bSpawnBot    = false;
    m_bScriptSpawn = false;
    m_bRejectSpawn = false;
}

void Level::CleanUp(qboolean samemap, qboolean resetConfigStrings)
{
    int i;

    DisableListenerNotify++;

    if (g_gametype->integer != GT_SINGLE_PLAYER) {
        dmManager.Reset();
    }

    Director.Reset(samemap);

    ClearCachedStatemaps();

    // clear active current bots
    G_ResetBots();

    assert(active_edicts.next);
    assert(active_edicts.next->prev == &active_edicts);
    assert(active_edicts.prev);
    assert(active_edicts.prev->next == &active_edicts);
    assert(free_edicts.next);
    assert(free_edicts.next->prev == &free_edicts);
    assert(free_edicts.prev);
    assert(free_edicts.prev->next == &free_edicts);

    while (active_edicts.next != &active_edicts) {
        assert(active_edicts.next != &free_edicts);
        assert(active_edicts.prev != &free_edicts);

        assert(active_edicts.next);
        assert(active_edicts.next->prev == &active_edicts);
        assert(active_edicts.prev);
        assert(active_edicts.prev->next == &active_edicts);
        assert(free_edicts.next);
        assert(free_edicts.next->prev == &free_edicts);
        assert(free_edicts.prev);
        assert(free_edicts.prev->next == &free_edicts);

        if (active_edicts.next->entity) {
            delete active_edicts.next->entity;
        } else {
            FreeEdict(active_edicts.next);
        }
    }

    //
    // Remove all archived entities
    //
    for (i = 1; i <= m_SimpleArchivedEntities.NumObjects(); i++) {
        delete m_SimpleArchivedEntities.ObjectAt(i);
    }

    cinematic = false;
    ai_on     = true;
    near_exit = false;

    mission_failed = false;
    died_already   = false;

    globals.num_entities = game.maxclients + 1;
    gi.LocateGameData(g_entities, game.maxclients + 1, sizeof(gentity_t), &game.clients[0].ps, sizeof(gclient_t));

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

    // reset projectile targets
    ClearProjectileTargets();

    // Reset the boss health cvar
    gi.cvar_set("bosshealth", "0");

    Actor::ResetBodyQueue();
    Health::ResetHealthQueue();

    if (world) {
        world->FreeTargetList();
    }

    num_earthquakes = 0;

    AddWaitTill(STRING_PRESPAWN);
    AddWaitTill(STRING_SPAWN);
    AddWaitTill(STRING_PLAYERSPAWN);
    AddWaitTill(STRING_SKIP);
    AddWaitTill(STRING_POSTTHINK);

    if (g_gametype->integer >= GT_TEAM_ROUNDS && g_gametype->integer <= GT_LIBERATION) {
        AddWaitTill(STRING_ROUNDSTART);
    }

    if (g_gametype->integer > GT_FFA) {
        AddWaitTill(STRING_ALLIESWIN);
        AddWaitTill(STRING_AXISWIN);
        AddWaitTill(STRING_DRAW);
    }

    if (resetConfigStrings) {
        gi.setConfigstring(CS_RAIN_DENSITY, "0");
        gi.setConfigstring(CS_RAIN_SPEED, "2048");
        gi.setConfigstring(CS_RAIN_SPEEDVARY, "512");
        gi.setConfigstring(CS_RAIN_SLANT, "50");
        gi.setConfigstring(CS_RAIN_LENGTH, "90");
        gi.setConfigstring(CS_RAIN_MINDIST, "512");
        gi.setConfigstring(CS_RAIN_WIDTH, "1");
        gi.setConfigstring(CS_RAIN_SHADER, "textures/rain");
        gi.setConfigstring(CS_RAIN_NUMSHADERS, "0");
        gi.setConfigstring(CS_CURRENT_OBJECTIVE, "");

        for (int i = CS_OBJECTIVES; i < CS_OBJECTIVES + MAX_OBJECTIVES; i++) {
            gi.setConfigstring(i, "");
        }

        gi.setConfigstring(CS_VOTE_TIME, "");
        gi.setConfigstring(CS_VOTE_STRING, "");
        gi.setConfigstring(CS_VOTE_YES, "");
        gi.setConfigstring(CS_VOTE_NO, "");
        gi.setConfigstring(CS_VOTE_UNDECIDED, "");
    }

    DisableListenerNotify--;

    svsStartFloatTime = svsFloatTime;

    FreeLandmarks();
}

/*
==============
ResetEdicts
==============
*/
void Level::ResetEdicts(void)
{
    int i;

    memset(g_entities, 0, game.maxentities * sizeof(g_entities[0]));

    // Add all the edicts to the free list
    LL_Reset(&free_edicts, next, prev);
    LL_Reset(&active_edicts, next, prev);
    for (i = 0; i < game.maxentities; i++) {
        LL_Add(&free_edicts, &g_entities[i], next, prev);
    }

    for (i = 0; i < game.maxclients; i++) {
        // set client fields on player ents
        g_entities[i].client = game.clients + i;

        G_InitClientPersistant(&game.clients[i]);
    }

    globals.num_entities = game.maxclients;
}

qboolean Level::inhibitEntity(int spawnflags)
{
    if (!detail->integer && (spawnflags & SPAWNFLAG_DETAIL)) {
        return qtrue;
    }

    if (g_gametype->integer != GT_SINGLE_PLAYER) {
        return (spawnflags & SPAWNFLAG_NOT_DEATHMATCH) ? qtrue : qfalse;
    }

    if (!developer->integer && (spawnflags & SPAWNFLAG_DEVELOPMENT)) {
        return qtrue;
    }

    if (!Q_stricmp(mapname, "t3l2")) {
        // Added in 2.0.
        //   FIXME: there should be a better way to handle
        //   specific maps
        return (spawnflags & SPAWNFLAG_NOT_EASY) ? qtrue : qfalse;
    }

    switch (skill->integer) {
    case 0:
        return (spawnflags & SPAWNFLAG_NOT_EASY) != 0;

    case 1:
        return (spawnflags & SPAWNFLAG_NOT_MEDIUM) != 0;

    case 2:
    case 3:
        return (spawnflags & SPAWNFLAG_NOT_HARD);
    }

    /*
#ifdef _CONSOLE
    if (spawnflags & SPAWNFLAG_NOCONSOLE)
#else
    if (spawnflags & SPAWNFLAG_NOPC)
#endif
    {
        return qtrue;
    }
    */

    return qfalse;
}

void Level::setSkill(int value)
{
    int skill_level;

    skill_level = floor((float)value);
    skill_level = bound(skill_level, 0, 3);

    gi.cvar_set("skill", va("%d", skill_level));
}

void Level::setTime(int levelTime)
{
    svsTime      = levelTime;
    inttime      = levelTime - svsStartTime;
    svsFloatTime = levelTime / 1000.0f;
    time         = inttime / 1000.0f;
}

void Level::setFrametime(int frametime)
{
    intframetime    = frametime;
    this->frametime = frametime / 1000.0f;
}

void Level::SpawnEntities(char *entities, int svsTime)
{
    int         inhibit, radnum = 0, count = 0;
    const char *value;
    SpawnArgs   args;
    Listener   *listener;
    Entity     *ent;
    int         t1, t2;
    int         start, end;
    char        name[128];

    Com_Printf("-------------------- Spawning Entities -----------------------\n");

    t1 = gi.Milliseconds();
    memset(skel_index, 0xff, sizeof(skel_index));

    // set up time so functions still have valid times
    setTime(svsTime);
    setFrametime(50);

    gi.LoadResource("*144");

    setSkill(skill->integer);

    // reset out count of the number of game traces
    sv_numtraces = 0;

    // parse world
    entities     = args.Parse(entities);
    spawn_entnum = ENTITYNUM_WORLD;
    args.SpawnInternal();

    gi.LoadResource("*147");

    // Set up for a new map
    PathManager.LoadNodes();

    gi.LoadResource("*147a");

    Com_Printf("-------------------- Actual Spawning Entities -----------------------\n");

    start = gi.Milliseconds();

    // parse ents
    inhibit = 0;

    for (entities = args.Parse(entities); entities != NULL; entities = args.Parse(entities)) {
        // remove things (except the world) from different skill levels or deathmatch
        spawnflags = 0;
        value      = args.getArg("spawnflags");

        if (value) {
            spawnflags = atoi(value);

            value = args.getArg("classname");
            if (!value || (Q_stricmp(value, "info_pathnode") && Q_stricmp(value, "info_patharea"))) {
                if (inhibitEntity(spawnflags)) {
                    inhibit++;
                    continue;
                }
            }
        }

        listener = args.SpawnInternal();

        if (listener) {
            radnum++;

            if (listener->isSubclassOf(Entity)) {
                count++;

                ent         = (Entity *)listener;
                ent->radnum = radnum;

                Q_strncpyz(ent->edict->entname, ent->getClassID(), sizeof(ent->edict->entname));

                ent->PostEvent(EV_Entity_Start, -1.0, 0);
                sprintf(name, "i%d", radnum);
                gi.LoadResource(name);
            }
        }
    }

    end = gi.Milliseconds();

    Com_Printf("-------------------- Actual Spawning Entities Done ------------------ %i ms\n", end - start);

    gi.LoadResource("*147b");

    world->UpdateConfigStrings();

    Event *ev = new Event(EV_Level_PreSpawnSentient);
    PostEvent(ev, EV_SPAWNENTITIES);

    L_ProcessPendingEvents();

    gi.LoadResource("*148");

    if (g_gametype->integer != GT_SINGLE_PLAYER) {
        dmManager.InitGame();
    }

    gi.LoadResource("*148a");

    // Single-player check added in OPM
    if (game.maxclients == 1 && g_gametype->integer == GT_SINGLE_PLAYER) {
        spawn_entnum = 0;
        new Player;
    }

    gi.LoadResource("*148b");

    m_LoopProtection = false;
    RemoveWaitTill(STRING_PRESPAWN);
    Unregister(STRING_PRESPAWN);
    m_LoopProtection = true;

    gi.LoadResource("*150");

    t2 = gi.Milliseconds();

    Com_Printf("%i entities spawned\n", count);
    Com_Printf("%i simple entities spawned\n", radnum);
    Com_Printf("%i entities inhibited\n", inhibit);

    Com_Printf("-------------------- Spawning Entities Done ------------------ %i ms\n", t2 - t1);

    //
    // create landmarks
    //
    ComputeDMWaypoints();
}

void Level::ComputeDMWaypoints()
{
    qboolean shouldSetDefaultLandmark;
    float    startXDistMin, startXDistMax;
    float    startYDistMin, startYDistMax;
    int      i;

    if (g_gametype->integer == GT_SINGLE_PLAYER) {
        m_fLandmarkXDistMax = 1;
        m_fLandmarkYDistMax = 1;
        m_fLandmarkXDistMin = 0;
        m_fLandmarkYDistMin = 0.0;
        return;
    }

    shouldSetDefaultLandmark = qfalse;

    //
    // calculate the world bounds from entities
    //
    if (m_fLandmarkXDistMin == m_fLandmarkXDistMax && m_fLandmarkYDistMin == m_fLandmarkYDistMax
        && m_fLandmarkXDistMax == m_fLandmarkYDistMax) {
        shouldSetDefaultLandmark = qtrue;

        for (i = 0; i < game.maxentities; i++) {
            gentity_t *ent = &g_entities[i];

            if (ent->entity) {
                AddLandmarkOrigin(ent->entity->origin);
                shouldSetDefaultLandmark = qfalse;
            }
        }
    }

    if (shouldSetDefaultLandmark) {
        startXDistMax = 1;
        startYDistMax = 1;
        startXDistMin = 0;
        startYDistMin = 0;
    } else {
        startYDistMax = m_fLandmarkYDistMax;
        startYDistMin = m_fLandmarkYDistMin;
        startXDistMax = m_fLandmarkXDistMax;
        startXDistMin = m_fLandmarkXDistMin;
    }

    m_fLandmarkXDistMin = startXDistMin + (startXDistMax - startXDistMin) / 3.f;
    m_fLandmarkXDistMax = startXDistMin + (startXDistMax - startXDistMin) * 2.f / 3.f;
    m_fLandmarkYDistMin = startYDistMin + (startYDistMax - startYDistMin) / 3.f;
    m_fLandmarkYDistMax = startYDistMin + (startYDistMax - startYDistMin) * 2.f / 3.f;
}

void Level::AddLandmarkOrigin(const Vector& origin)
{
    float  yaw;
    vec3_t angles;
    vec3_t dir;
    float  dist;

    yaw = origin.toYaw();

    angles[0] = angles[2] = 0;
    angles[1]             = yaw + 90 - world->m_fNorth;
    AngleVectors(angles, dir, NULL, NULL);

    dist = origin.lengthXY();
    VectorScale(dir, dist, dir);

    if (m_fLandmarkYDistMax == m_fLandmarkYDistMin && m_fLandmarkXDistMin == m_fLandmarkXDistMax
        && m_fLandmarkYDistMax == m_fLandmarkXDistMin) {
        m_fLandmarkYDistMin = dir[1];
        m_fLandmarkXDistMax = dir[0];
        m_fLandmarkYDistMax = dir[1] + 1.0;
        m_fLandmarkXDistMin = dir[0] - 1.0;
    } else {
        if (m_fLandmarkYDistMin > dir[1]) {
            m_fLandmarkYDistMin = dir[1];
        }
        if (m_fLandmarkYDistMax < dir[1]) {
            m_fLandmarkYDistMax = dir[1];
        }
        if (m_fLandmarkXDistMax < dir[0]) {
            m_fLandmarkXDistMax = dir[0];
        }
        if (m_fLandmarkXDistMin > dir[0]) {
            m_fLandmarkXDistMin = dir[0];
        }
    }
}

void Level::AddLandmarkName(const str& name, const Vector& origin)
{
    landmark_t *landmark;
    int         i;

    if (m_pLandmarks) {
        if (m_iLandmarksCount == m_iMaxLandmarks) {
            // reallocate the landmark list with twice the size
            landmark_t **oldLandmarks = m_pLandmarks;

            m_iMaxLandmarks *= 2;
            m_pLandmarks = new landmark_t *[m_iMaxLandmarks];

            for (i = 0; i < m_iLandmarksCount; i++) {
                m_pLandmarks[i] = oldLandmarks[i];
            }

            delete[] oldLandmarks;
        }
    } else {
        // create the landmark list for the first time
        m_iMaxLandmarks   = 8;
        m_iLandmarksCount = 0;
        m_pLandmarks      = new landmark_t *[8];
    }

    //
    // create a new landmark
    //
    landmark = m_pLandmarks[m_iLandmarksCount] = new landmark_t();
    landmark->m_sName                          = name;
    landmark->m_vOrigin                        = origin;
}

void Level::FreeLandmarks()
{
    landmark_t *landmark;

    if (m_pLandmarks) {
        int i;

        for (i = 0; i < m_iLandmarksCount; i++) {
            landmark = m_pLandmarks[i];

            if (landmark) {
                delete landmark;
            }
        }

        delete[] m_pLandmarks;
        m_pLandmarks      = NULL;
        m_iLandmarksCount = 0;
        m_iMaxLandmarks   = 0;
    }
}

str Level::GetDynamicDMLocations(const Vector& origin)
{
    str   name = "nothing";
    int   i;
    float shortestDistSqr = 0;

    for (i = 0; i < m_iLandmarksCount; i++) {
        landmark_t *landmark = m_pLandmarks[i];
        Vector      delta    = origin - landmark->m_vOrigin;
        float       distSqr  = delta.lengthSquared();

        if (i == 0 || distSqr < shortestDistSqr) {
            shortestDistSqr = distSqr;
            name            = landmark->m_sName;
        }
    }

    return name;
}

str Level::GetDMLocation(const Vector& origin)
{
    float  yaw;
    vec3_t angles;
    vec3_t dir;
    float  dist;

    if (m_pLandmarks) {
        //
        // use the dynamic dm locations
        //
        return GetDynamicDMLocations(origin);
    }

    yaw = origin.toYaw();

    angles[0] = angles[2] = 0;
    angles[1]             = yaw + 90 - world->m_fNorth;
    AngleVectors(angles, dir, NULL, NULL);

    dist = origin.lengthXY();
    VectorScale(dir, dist, dir);

    if (dir[0] >= m_fLandmarkXDistMin) {
        if (dir[0] > m_fLandmarkXDistMax) {
            if (dir[1] >= m_fLandmarkYDistMin) {
                if (dir[1] > m_fLandmarkYDistMax) {
                    return "North East corner";
                } else {
                    return "East side";
                }
            } else {
                return "South East corner";
            }
        } else if (dir[1] >= m_fLandmarkYDistMin) {
            if (dir[1] > m_fLandmarkYDistMax) {
                return "North side";
            } else {
                return "center";
            }
        } else {
            return "South side";
        }
    } else if (dir[1] >= m_fLandmarkYDistMin) {
        if (dir[1] > m_fLandmarkYDistMax) {
            return "North West corner";
        } else {
            return "West side";
        }
    } else {
        return "South West corner";
    }
}

void Level::PreSpawnSentient(Event *ev)
{
    GameScript *script;

    // general initialization
    FindTeams();

    script = Director.GetScript(m_mapscript);
    if (script) {
        gi.DPrintf("Adding script: '%s'\n", m_mapscript.c_str());

        m_LoopProtection = false;
        Director.ExecuteThread(m_mapscript);
        m_LoopProtection = true;
    }

    PathManager.CreatePaths();
}

bool Level::RoundStarted()
{
    return !WaitTillDefined(STRING_ROUNDSTART);
}

bool Level::PreSpawned(void)
{
    return !WaitTillDefined(STRING_PRESPAWN);
}

bool Level::Spawned(void)
{
    return !WaitTillDefined(STRING_SPAWN);
}

void Level::ServerSpawned(void)
{
    int        i;
    gclient_t *client;
    gentity_t *ent;

    for (i = 0, client = game.clients; i < game.maxclients; i++, client++) {
        client->ps.commandTime = svsTime;
    }

    if (!Spawned()) {
        RemoveWaitTill(STRING_SPAWN);

        Director.Pause();

        for (ent = active_edicts.next; ent != &active_edicts; ent = ent->next) {
            ent->entity->Unregister(STRING_SPAWN);
        }

        Director.Unpause();

        Unregister(STRING_SPAWN);
    } else {
        Director.LoadMenus();
    }

    spawning = false;
}

void Level::SetMap(const char *themapname)
{
    char *spawnpos;
    int   i;
    str   text;

    Init();

    spawning = true;

    // set a specific spawnpoint if the map was started with a $
    spawnpos = strchr((char *)themapname, '$');
    if (spawnpos) {
        mapname    = (const char *)(spawnpos - themapname);
        spawnpoint = mapname;
    } else {
        mapname    = themapname;
        spawnpoint = "";
    }

    current_map = (char *)themapname;

    level_name = mapname;
    for (i = 0; i < level_name.length(); i++) {
        if (level_name[i] == '.') {
            level_name[i] = 0;
            break;
        }
    }

    m_mapscript      = "maps/" + level_name + ".scr";
    m_precachescript = "maps/" + level_name + "_precache.scr";
    m_pathfile       = "maps/" + level_name + ".pth";
    m_mapfile        = "maps/" + level_name + ".bsp";
}

void Level::LoadAllScripts(const char *name, const char *extension)
{
    char **scriptFiles;
    char   filename[MAX_QPATH];
    int    numScripts;

    scriptFiles = gi.FS_ListFiles(name, extension, qfalse, &numScripts);

    if (!scriptFiles || !numScripts) {
        return;
    }

    for (int i = 0; i < numScripts; i++) {
        Com_sprintf(filename, sizeof(filename), "%s/%s", name, scriptFiles[i]);

        // Compile the script
        Director.GetScript(filename);
    }

    gi.FS_FreeFileList(scriptFiles);
}

void Level::Precache(void)
{
    setTime(svsStartTime);
    setFrametime(50);

    if (gi.FS_ReadFile(m_precachescript, NULL, qtrue) != -1) {
        gi.DPrintf("Adding script: '%s'\n", m_precachescript.c_str());

        // temporarily disable the loop protection
        // because caching models require time
        m_LoopProtection = false;
        Director.ExecuteThread(m_precachescript);
        m_LoopProtection = true;
    }

    if (g_gametype->integer == GT_SINGLE_PLAYER) {
        LoadAllScripts("anim", ".scr");
    }

    LoadAllScripts("global", ".scr");
    InitVoteOptions();
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
    gentity_t *ent, *ent2;
    Entity    *e, *e2;
    int        i, j;
    int        c, c2;

    c  = 0;
    c2 = 0;
    for (i = 1, ent = g_entities + i; i < globals.num_entities; i++, ent++) {
        if (!ent->inuse) {
            continue;
        }

        e = ent->entity;

        if (!e->moveteam.length()) {
            continue;
        }

        if (e->flags & FL_TEAMSLAVE) {
            continue;
        }

        e->teammaster = e;
        c++;
        c2++;

        for (j = i + 1, ent2 = ent + 1; j < globals.num_entities; j++, ent2++) {
            if (!ent2->inuse) {
                continue;
            }

            e2 = ent->entity;

            if (!e2->moveteam.length()) {
                continue;
            }

            if (e2->flags & FL_TEAMSLAVE) {
                continue;
            }

            if (!strcmp(e->moveteam, e2->moveteam)) {
                c2++;
                e2->teamchain  = e->teamchain;
                e->teamchain   = e2;
                e2->teammaster = e;
                e2->flags |= FL_TEAMSLAVE;

                // make sure that targets only point at the master
                if (e2->targetname) {
                    e->targetname  = e2->targetname;
                    e2->targetname = NULL;
                }
            }
        }
    }

    G_Printf("%i teams with %i entities\n", c, c2);
}

gentity_t *Level::AllocEdict(Entity *entity)
{
    int        i;
    gentity_t *edict;

    if (spawn_entnum >= 0) {
        edict        = &g_entities[spawn_entnum];
        spawn_entnum = -1;

        assert(!edict->inuse && !edict->entity);

        // free up the entity pointer in case we took one that still exists
        if (edict->inuse && edict->entity) {
            delete edict->entity;
        }
    } else {
        edict = &g_entities[game.maxclients];

        for (i = game.maxclients; i < globals.num_entities; i++, edict++) {
            // the first couple seconds of server time can involve a lot of
            // freeing and allocating, so relax the replacement policy
            if (!edict->inuse && ((edict->freetime < 2.0f) || (time - edict->freetime > 0.5f))) {
                break;
            }
        }

        // allow two spots for none and world
        if (i == globals.max_entities - 2.0f) {
            // Try one more time before failing, relax timing completely

            edict = &g_entities[game.maxclients];

            for (i = game.maxclients; i < globals.num_entities; i++, edict++) {
                if (!edict->inuse) {
                    break;
                }
            }

            if (i == globals.max_entities - 2.0f) {
                gi.Error(ERR_DROP, "Level::AllocEdict: no free edicts");
            }
        }
    }

    LL_Remove(edict, next, prev);

    InitEdict(edict);

    LL_Add(&active_edicts, edict, next, prev);

    // Tell the server about our data since we just spawned something
    if ((edict->s.number < ENTITYNUM_WORLD) && (globals.num_entities <= edict->s.number)) {
        globals.num_entities = edict->s.number + 1;
        gi.LocateGameData(
            g_entities, globals.num_entities, sizeof(gentity_t), &game.clients[0].ps, sizeof(game.clients[0])
        );
    }

    edict->entity = entity;

    return edict;
}

void Level::FreeEdict(gentity_t *ed)
{
    gclient_t *client;

    // unlink from world
    gi.unlinkentity(ed);

    LL_Remove(ed, next, prev);

    client = ed->client;

    memset(ed, 0, sizeof(*ed));

    ed->client   = client;
    ed->freetime = time;
    ed->inuse    = false;
    ed->s.number = ed - g_entities;

    LL_Add(&free_edicts, ed, next, prev);
}

void Level::InitEdict(gentity_t *e)
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
    e->spawntime  = level.time;

    for (i = 0; i < NUM_BONE_CONTROLLERS; i++) {
        e->s.bone_tag[i] = -1;
        VectorClear(e->s.bone_angles[i]);
        EulerToQuat(e->s.bone_angles[i], e->s.bone_quat[i]);
    }
}

void Level::AddAutomaticCamera(Camera *cam)
{
    automatic_cameras.AddUniqueObject(cam);
}

void Level::InitVoteOptions()
{
    if (g_gametype->integer == GT_SINGLE_PLAYER) {
        return;
    }

    m_voteOptions.SetupVoteOptions("callvote.cfg");
    // clear the vote time
    gi.setConfigstring(CS_VOTE_TIME, "");
}

void Level::SendVoteOptionsFile(gentity_t *ent)
{
    const char *voteBuffer;
    int         voteLength;
    int         clientNum;
    int         i, j;
    char        buffer[2068];

    clientNum = ent - g_entities;

    if (clientNum < 0 || clientNum >= game.maxclients) {
        return;
    }

    voteBuffer = m_voteOptions.GetVoteOptionsFile(&voteLength);
    if (voteLength < MAX_VOTEOPTIONS_UPLOAD_BUFFER_LENGTH) {
        Q_strncpyz(buffer, voteBuffer, voteLength + 1);

        for (i = 0; i < voteLength; i++) {
            if (buffer[i] == '"') {
                buffer[i] = 1;
            }
        }

        gi.SendServerCommand(clientNum, "vo0 \"\"\n");
        gi.SendServerCommand(clientNum, "vo2 \"%s\"\n", buffer);
    } else {
        const char *cmd;
        int         destLength;
        int         offset;

        offset = 0;
        for (i = voteLength; i > 0; i -= MAX_VOTEOPTIONS_UPLOAD_BUFFER_LENGTH - 1) {
            if (offset == 0) {
                cmd        = "vo0";
                destLength = MAX_VOTEOPTIONS_UPLOAD_BUFFER_LENGTH;
            } else if (i >= MAX_VOTEOPTIONS_UPLOAD_BUFFER_LENGTH) {
                cmd        = "vo1";
                destLength = MAX_VOTEOPTIONS_UPLOAD_BUFFER_LENGTH;
            } else {
                cmd        = "vo2";
                destLength = i;
            }

            Q_strncpyz(buffer, &voteBuffer[offset], MAX_VOTEOPTIONS_UPLOAD_BUFFER_LENGTH);

            for (j = 0; j < destLength; j++) {
                if (buffer[j] == '"') {
                    buffer[j] = 1;
                }
            }

            gi.SendServerCommand(clientNum, "%s \"%s\"\n", cmd, buffer);
            offset += MAX_VOTEOPTIONS_UPLOAD_BUFFER_LENGTH - 1;
        }
    }
}

bool Level::GetVoteOptionMain(int index, str *outOptionCommand, voteoptiontype_t *outOptionType)
{
    return m_voteOptions.GetVoteOptionsMain(index, outOptionCommand, outOptionType);
}

bool Level::GetVoteOptionSub(int index, int listIndex, str *outCommand)
{
    return m_voteOptions.GetVoteOptionSub(index, listIndex, outCommand);
}

bool Level::GetVoteOptionMainName(int index, str *outVoteName)
{
    return m_voteOptions.GetVoteOptionMainName(index, outVoteName);
}

bool Level::GetVoteOptionSubName(int index, int listIndex, str *outName)
{
    return m_voteOptions.GetVoteOptionSubName(index, listIndex, outName);
}

void Level::CheckVote(void)
{
    gentity_t *ent;
    int        i;
    int        numVoters;
    int        oldVoteYes, oldVoteNo, oldNumVoters;

    if (m_nextVoteTime && m_nextVoteTime < inttime) {
        m_nextVoteTime = 0;

        gi.SendConsoleCommand(va("%s\n", m_voteString.c_str()));

        if (sv_sprinton->integer == 1) {
            if (sv_runspeed->integer == 250) {
                gi.cvar_set("sv_runspeed", "287");
            }
        } else if (sv_runspeed->integer == 287) {
            gi.cvar_set("sv_runspeed", "250");
        }

        SetupMaplist();
    }

    if (!m_voteTime) {
        return;
    }

    oldVoteYes   = level.m_voteYes;
    oldVoteNo    = level.m_voteNo;
    oldNumVoters = level.m_numVoters;

    level.m_voteYes = 0;
    level.m_voteNo  = 0;
    numVoters       = 0;

    for (i = 0; i < game.maxclients; i++) {
        Player *p;

        ent = &g_entities[i];
        if (!ent->inuse || !ent->client || !ent->entity) {
            continue;
        }

        p = static_cast<Player *>(ent->entity);
        if (p->client->ps.voted) {
            if (p->HasVotedYes()) {
                level.m_voteYes++;
            } else {
                level.m_voteNo++;
            }
        }
    }

    level.m_numVoters = numVoters;

    if ((svsFloatTime - svsStartFloatTime) * 1000 - m_voteTime >= 30000) {
        G_PrintToAllClients(va("%s: %s\n", gi.CL_LV_ConvertString("Vote Failed"), m_voteName.c_str()));
        m_voteTime = 0;
        gi.setConfigstring(CS_VOTE_TIME, "");
    } else if (m_voteYes > m_numVoters / 2) {
        // Pass arguments to console
        G_PrintToAllClients(va("%s: %s\n", gi.CL_LV_ConvertString("Vote Passed"), m_voteName.c_str()));
        m_nextVoteTime = level.inttime + 3000;
        m_voteTime     = 0;
        gi.setConfigstring(CS_VOTE_TIME, "");
    } else if (m_voteNo >= m_numVoters / 2) {
        G_PrintToAllClients(va("%s: %s\n", gi.CL_LV_ConvertString("Vote Failed"), m_voteName.c_str()));
        m_voteTime = 0;
        gi.setConfigstring(CS_VOTE_TIME, "");
    } else {
        if (oldVoteYes != level.m_voteYes) {
            gi.setConfigstring(CS_VOTE_YES, va("%i", level.m_voteYes));
        }

        if (oldVoteNo != level.m_voteNo) {
            gi.setConfigstring(CS_VOTE_NO, va("%i", level.m_voteNo));
        }

        if (oldNumVoters != level.m_numVoters || oldVoteYes != level.m_voteYes || oldVoteNo != level.m_voteNo) {
            gi.setConfigstring(CS_VOTE_UNDECIDED, va("%i", level.m_numVoters - (level.m_voteYes + level.m_voteNo)));
        }
    }
}

void Level::SetupMaplist()
{
    const char *p;
    const char  delim[2] = {';', 0};
    cvar_t     *maplistVar;
    int         gameTypeNum;
    char        buffer[1024];
    char        gameTypeBuffer[12];

    if (!strstr(m_voteString.c_str(), "g_gametype")) {
        return;
    }

    strcpy(buffer, m_voteString.c_str());

    for (p = strtok(buffer, delim); p; p = strtok(NULL, delim)) {
        if (strstr(p, "g_gametype")) {
            gameTypeBuffer[0] = p[strlen(p) - 1];
            gameTypeBuffer[1] = 0;
            gameTypeNum       = atoi(gameTypeBuffer);
        }
    }

    switch (gameTypeNum) {
    case GT_FFA:
        gi.ExecuteConsoleCommand(EXEC_NOW, "exec maplist_ffa.cfg");
        maplistVar = gi.Cvar_Get("ui_maplist_ffa", "", 0);
        break;
    case GT_TEAM:
        gi.ExecuteConsoleCommand(EXEC_NOW, "exec maplist_team.cfg");
        maplistVar = gi.Cvar_Get("ui_maplist_team", "", 0);
        break;
    case GT_TEAM_ROUNDS:
        gi.ExecuteConsoleCommand(EXEC_NOW, "exec maplist_round.cfg");
        maplistVar = gi.Cvar_Get("ui_maplist_round", "", 0);
        break;
    case GT_OBJECTIVE:
        gi.ExecuteConsoleCommand(EXEC_NOW, "exec maplist_obj.cfg");
        maplistVar = gi.Cvar_Get("ui_maplist_obj", "", 0);
        break;
    case GT_TOW:
        gi.ExecuteConsoleCommand(EXEC_NOW, "exec maplist_tow.cfg");
        maplistVar = gi.Cvar_Get("ui_maplist_tow", "", 0);
        break;
    case GT_LIBERATION:
        gi.ExecuteConsoleCommand(EXEC_NOW, "exec maplist_lib.cfg");
        maplistVar = gi.Cvar_Get("ui_maplist_lib", "", 0);
        break;
    }

    gi.cvar_set("sv_maplist", maplistVar->string);
}

void Level::GetAlarm(Event *ev)
{
    ev->AddInteger(m_bAlarm);
}

void Level::SetAlarm(Event *ev)
{
    m_bAlarm = ev->GetInteger(1);
}

void Level::SetNoDropHealth(Event *ev)
{
    mbNoDropHealth = ev->GetInteger(1);
}

void Level::SetNoDropWeapons(Event *ev)
{
    mbNoDropWeapons = ev->GetInteger(1);
}

void Level::GetLoopProtection(Event *ev)
{
    ev->AddInteger(m_LoopProtection);
}

void Level::SetLoopProtection(Event *ev)
{
    m_LoopProtection = ev->GetInteger(1);
}

void Level::GetPapersLevel(Event *ev)
{
    ev->AddInteger(m_iPapersLevel);
}

void Level::SetPapersLevel(Event *ev)
{
    m_iPapersLevel = ev->GetInteger(1);
}

void Level::EventGetRoundStarted(Event *ev)
{
    ev->AddInteger(RoundStarted());
}

void Level::EventGetDMRespawning(Event *ev)
{
    ev->AddInteger(dmManager.GameAllowsRespawns());
}

void Level::EventSetDMRespawning(Event *ev)
{
    dmManager.SetGameAllowsRespawns(ev->GetBoolean(1));
}

void Level::EventGetDMRoundLimit(Event *ev)
{
    ev->AddInteger(dmManager.GetRoundLimit());
}

void Level::EventSetDMRoundLimit(Event *ev)
{
    int round_limit = ev->GetInteger(1);

    if (round_limit < 0) {
        ScriptError("round limit must be greater than 0");
    }

    dmManager.SetDefaultRoundLimit(round_limit);
}

void Level::EventGetClockSide(Event *ev)
{
    ev->AddConstString(dmManager.GetClockSide());
}

void Level::EventSetClockSide(Event *ev)
{
    const_str clockside = ev->GetConstString(1);

    if (clockside < STRING_ALLIES || clockside > STRING_KILLS) {
        ScriptError("clockside must be 'axis', 'allies', 'kills', or 'draw'");
    }

    dmManager.SetClockSide(clockside);
}

void Level::EventGetBombPlantTeam(Event *ev)
{
    ev->AddConstString(dmManager.GetBombPlantTeam());
}

void Level::EventSetBombPlantTeam(Event *ev)
{
    const_str plant_team = ev->GetConstString(1);

    if (plant_team < STRING_ALLIES || plant_team > STRING_AXIS) {
        ScriptError("bombplantteam must be 'axis' or 'allies'");
    }

    dmManager.SetBombPlantTeam(plant_team);
}

void Level::EventGetTargetsToDestroy(Event *ev)
{
    ev->AddInteger(dmManager.GetTargetsToDestroy());
}

void Level::EventSetTargetsToDestroy(Event *ev)
{
    dmManager.SetTargetsToDestroy(ev->GetInteger(1));
}

void Level::EventGetTargetsDestroyed(Event *ev)
{
    ev->AddInteger(dmManager.GetTargetsDestroyed());
}

void Level::EventSetTargetsDestroyed(Event *ev)
{
    dmManager.SetTargetsDestroyed(ev->GetInteger(1));
}

void Level::EventGetBombsPlanted(Event *ev)
{
    ev->AddInteger(dmManager.GetBombsPlanted());
}

void Level::EventSetBombsPlanted(Event *ev)
{
    dmManager.SetBombsPlanted(ev->GetInteger(1));
}

void Level::EventGetRoundBased(Event *ev)
{
    ev->AddInteger(g_gametype->integer >= GT_TEAM_ROUNDS);
}

void Level::EventGetObjectiveBased(Event *ev)
{
    ev->AddInteger(g_gametype->integer >= GT_OBJECTIVE);
}

str Level::GetRandomHeadModel(const char *model)
{
    char s[MAX_STRING_TOKENS];

    gi.GetHeadModel(model, G_Random(gi.NumHeadModels(model)), s);
    return s;
}

str Level::GetRandomHeadSkin(const char *model)
{
    char s[MAX_STRING_TOKENS];

    gi.GetHeadSkin(model, G_Random(gi.NumHeadSkins(model)), s);
    return s;
}

void Level::AddEarthquake(earthquake_t *e)
{
    if (num_earthquakes == MAX_EARTHQUAKES) {
        if (earthquakes[0].m_Thread) {
            earthquakes[0].m_Thread->Wait(e->endtime - inttime);
        }

        num_earthquakes--;

        for (int i = 0; i < num_earthquakes; i++) {
            earthquakes[i] = earthquakes[i + 1];
        }
    }

    earthquakes[num_earthquakes] = *e;
    num_earthquakes++;

    e->m_Thread->Pause();
}

void Level::DoEarthquakes(void)
{
    int           i, j;
    earthquake_t *e;
    int           timedelta;
    int           rampuptime;
    int           rampdowntime;
    float         test_magnitude;

    if (num_earthquakes <= 0) {
        return;
    }

    earthquake_magnitude = 0.0f;

    for (i = num_earthquakes; i > 0; i--) {
        e = &earthquakes[i - 1];

        if (inttime >= e->endtime || !e->m_Thread) {
            if (e->m_Thread) {
                e->m_Thread->Wait(0);
            }

            num_earthquakes--;

            for (j = 0; j < num_earthquakes; j++) {
                earthquakes[j] = earthquakes[j + 1];
            }
        } else {
            test_magnitude = e->magnitude;

            timedelta = inttime - e->starttime;

            if (timedelta >= e->duration / 2) {
                rampdowntime = 2 * e->duration / 3 + e->starttime;

                if (!e->no_rampdown && inttime > rampdowntime) {
                    test_magnitude *= 1.0f - (inttime - rampdowntime) * 3.0f / e->duration;
                }
            } else {
                rampuptime = e->duration / 3 + e->starttime;

                if (!e->no_rampup && inttime < rampuptime) {
                    test_magnitude *= (inttime - e->starttime) * 3.0f / e->duration;
                }
            }

            if (test_magnitude > earthquake_magnitude) {
                earthquake_magnitude = test_magnitude;
            }
        }
    }
}

void Level::EventRainDensitySet(Event *ev)
{
    gi.setConfigstring(CS_RAIN_DENSITY, ev->GetString(1));
}

void Level::EventRainDensityGet(Event *ev)
{
    ev->AddString(gi.getConfigstring(CS_RAIN_DENSITY));
}

void Level::EventRainSpeedSet(Event *ev)
{
    gi.setConfigstring(CS_RAIN_SPEED, ev->GetString(1));
}

void Level::EventRainSpeedGet(Event *ev)
{
    ev->AddString(gi.getConfigstring(CS_RAIN_SPEED));
}

void Level::EventRainSpeedVarySet(Event *ev)
{
    gi.setConfigstring(CS_RAIN_SPEEDVARY, ev->GetString(1));
}

void Level::EventRainSpeedVaryGet(Event *ev)
{
    ev->AddString(gi.getConfigstring(CS_RAIN_SPEEDVARY));
}

void Level::EventRainSlantSet(Event *ev)
{
    gi.setConfigstring(CS_RAIN_SLANT, ev->GetString(1));
}

void Level::EventRainSlantGet(Event *ev)
{
    ev->AddString(gi.getConfigstring(CS_RAIN_SLANT));
}

void Level::EventRainLengthSet(Event *ev)
{
    gi.setConfigstring(CS_RAIN_LENGTH, ev->GetString(1));
}

void Level::EventRainLengthGet(Event *ev)
{
    ev->AddString(gi.getConfigstring(CS_RAIN_LENGTH));
}

void Level::EventRainMin_DistSet(Event *ev)
{
    gi.setConfigstring(CS_RAIN_MINDIST, ev->GetString(1));
}

void Level::EventRainMin_DistGet(Event *ev)
{
    ev->AddString(gi.getConfigstring(CS_RAIN_MINDIST));
}

void Level::EventRainWidthSet(Event *ev)
{
    gi.setConfigstring(CS_RAIN_WIDTH, ev->GetString(1));
}

void Level::EventRainWidthGet(Event *ev)
{
    ev->AddString(gi.getConfigstring(CS_RAIN_WIDTH));
}

void Level::EventRainShaderSet(Event *ev)
{
    gi.setConfigstring(CS_RAIN_SHADER, ev->GetString(1));
}

void Level::EventRainShaderGet(Event *ev)
{
    ev->AddString(gi.getConfigstring(CS_RAIN_SHADER));
}

void Level::EventRainNumShadersSet(Event *ev)
{
    gi.setConfigstring(CS_RAIN_NUMSHADERS, ev->GetString(1));
}

void Level::EventRainNumShadersGet(Event *ev)
{
    ev->AddString(gi.getConfigstring(CS_RAIN_NUMSHADERS));
}

void Level::EventAddBadPlace(Event *ev)
{
    badplace_t bp;
    int        nArgs;

    nArgs = ev->NumArgs();
    if (nArgs != 3 && nArgs != 4 && nArgs != 5) {
        throw ScriptException("badplace requires 3, 4, or 5 arguments");
    }

    if (ev->NumArgs() >= 5) {
        bp.m_fLifespan = ev->GetFloat(5);
        if (bp.m_fLifespan <= 0) {
            throw ScriptException("life span must be greater than 0");
        }
    }

    if (ev->NumArgs() >= 4) {
        switch (ev->GetConstString(4)) {
        case STRING_ALLIES:
        case STRING_AMERICAN:
            bp.m_iTeamSide = TEAM_ALLIES;
            break;
        case STRING_AXIS:
        case STRING_GERMAN:
            bp.m_iTeamSide = TEAM_AXIS;
            break;
        default:
            throw ScriptException("badplace must belong to 'american' or 'german' or 'both'");
        }
    }

    bp.m_fRadius = ev->GetFloat(3);
    bp.m_vOrigin = ev->GetVector(2);
    bp.m_name    = ev->GetConstString(1);

    if (bp.m_name == STRING_EMPTY && bp.m_fLifespan == FLT_MAX) {
        throw ScriptException("unnamed badplaces must have a specified duration");
    }

    if (bp.m_name != STRING_EMPTY) {
        int i;

        // replace badplaces with the same name
        for (i = 1; i <= m_badPlaces.NumObjects(); i++) {
            const badplace_t& existing = m_badPlaces.ObjectAt(i);

            if (existing.m_name == bp.m_name) {
                // remove the existing bad place at the pathway
                PathSearch::UpdatePathwaysForBadPlace(existing.m_vOrigin, existing.m_fRadius, -1, existing.m_iTeamSide);
                m_badPlaces.SetObjectAt(i, bp);
                // add the new bad place
                PathSearch::UpdatePathwaysForBadPlace(bp.m_vOrigin, bp.m_fRadius, 1, bp.m_iTeamSide);
                G_BroadcastAIEvent(NULL, vec_zero, 12, -1);
                return;
            }
        }
    }

    // add the new bad place
    m_badPlaces.AddObject(bp);
    PathSearch::UpdatePathwaysForBadPlace(bp.m_vOrigin, bp.m_fRadius, 1, bp.m_iTeamSide);
    // notify AI
    G_BroadcastAIEvent(NULL, vec_zero, 12, -1);
}

void Level::EventRemoveBadPlace(Event *ev)
{
    const_str name;
    int       i;

    if (ev->NumArgs() != 1) {
        throw ScriptException("removebadplace requires exactly 1 argument");
    }

    name = ev->GetConstString(1);
    for (i = 1; i <= m_badPlaces.NumObjects(); i++) {
        badplace_t& bp = m_badPlaces.ObjectAt(i);

        if (bp.m_name == name) {
            PathSearch::UpdatePathwaysForBadPlace(bp.m_vOrigin, bp.m_fRadius, -1, bp.m_iTeamSide);
            m_badPlaces.RemoveObjectAt(i);
            G_BroadcastAIEvent(NULL, vec_zero, 12, -1);
            return;
        }
    }

    Com_Printf("removebadplace: name '%s' not found", ev->GetString(1).c_str());
}

void Level::EventIgnoreClock(Event *ev)
{
    m_bIgnoreClock = ev->GetBoolean(1);
}

void Level::UpdateBadPlaces()
{
    qboolean removed;
    int      i;

    removed = qfalse;

    for (i = 1; i <= m_badPlaces.NumObjects(); i++) {
        badplace_t& bp = m_badPlaces.ObjectAt(i);
        if (level.time < bp.m_fLifespan) {
            continue;
        } else {
            PathSearch::UpdatePathwaysForBadPlace(bp.m_vOrigin, bp.m_fRadius, -1, bp.m_iTeamSide);
            m_badPlaces.RemoveObjectAt(i);
            removed = qtrue;
            i       = 0;
        }
    }

    if (removed) {
        // tell AI that a bad place was removed
        G_BroadcastAIEvent(NULL, vec_zero, 12, -1);
    }
}

int Level::GetNearestBadPlace(const Vector& org, float radius, int team) const
{
    float bestDistSqr;
    int   bestBpIndex;
    int   i;

    bestDistSqr = FLT_MAX;
    bestBpIndex = 0;

    for (i = 1; i <= m_badPlaces.NumObjects(); i++) {
        badplace_t& bp = m_badPlaces.ObjectAt(i);

        if (team & bp.m_iTeamSide) {
            const Vector delta   = bp.m_vOrigin - org;
            float        distSqr = delta.lengthSquared();
            float        rad     = bp.m_fRadius + radius;

            if (distSqr < bestDistSqr && distSqr < rad * rad) {
                bestDistSqr = distSqr;
                bestBpIndex = i;
            }
        }
    }

    return bestBpIndex;
}

static void ArchiveBadPlace(Archiver& arc, badplace_t *bp)
{
    Director.ArchiveString(arc, bp->m_name);
    arc.ArchiveVector(&bp->m_vOrigin);
    arc.ArchiveFloat(&bp->m_fRadius);
    arc.ArchiveFloat(&bp->m_fLifespan);
}

void Level::Archive(Archiver& arc)
{
    bool prespawn;
    bool spawn;

    Listener::Archive(arc);

    if (arc.Saving()) {
        prespawn = classinfo()->WaitTillDefined("prespawn");
        spawn    = classinfo()->WaitTillDefined("spawn");
    }

    arc.ArchiveBool(&prespawn);
    arc.ArchiveBool(&spawn);

    if (arc.Loading()) {
        if (prespawn) {
            RemoveWaitTill(STRING_PRESPAWN);
        }

        if (spawn) {
            RemoveWaitTill(STRING_SPAWN);
        }
    }

    arc.ArchiveInteger(&framenum);

    arc.ArchiveString(&level_name);
    arc.ArchiveString(&mapname);
    arc.ArchiveString(&spawnpoint);
    arc.ArchiveString(&nextmap);

    arc.ArchiveBoolean(&playerfrozen);

    arc.ArchiveFloat(&intermissiontime);
    ArchiveEnum(intermissiontype, INTTYPE_e);
    arc.ArchiveInteger(&exitintermission);

    arc.ArchiveInteger(&total_secrets);
    arc.ArchiveInteger(&found_secrets);

    arc.ArchiveFloat(&earthquake_magnitude);
    arc.ArchiveInteger(&num_earthquakes);

    for (int i = 0; i < num_earthquakes; i++) {
        arc.ArchiveInteger(&earthquakes[i].duration);
        arc.ArchiveFloat(&earthquakes[i].magnitude);
        arc.ArchiveBool(&earthquakes[i].no_rampup);
        arc.ArchiveBool(&earthquakes[i].no_rampdown);

        arc.ArchiveInteger(&earthquakes[i].starttime);
        arc.ArchiveInteger(&earthquakes[i].endtime);

        arc.ArchiveSafePointer(&earthquakes[i].m_Thread);
    }

    arc.ArchiveBoolean(&cinematic);
    arc.ArchiveBoolean(&ai_on);
    arc.ArchiveBoolean(&mission_failed);
    arc.ArchiveBoolean(&died_already);

    arc.ArchiveVector(&water_color);
    arc.ArchiveVector(&lava_color);

    arc.ArchiveFloat(&water_alpha);
    arc.ArchiveFloat(&lava_alpha);

    arc.ArchiveString(&current_soundtrack);
    arc.ArchiveString(&saved_soundtrack);

    arc.ArchiveVector(&m_fade_color);
    arc.ArchiveFloat(&m_fade_alpha);
    arc.ArchiveFloat(&m_fade_time);
    arc.ArchiveFloat(&m_fade_time_start);
    ArchiveEnum(m_fade_style, fadestyle_t);
    ArchiveEnum(m_fade_type, fadetype_t);

    arc.ArchiveFloat(&m_letterbox_fraction);
    arc.ArchiveFloat(&m_letterbox_time);
    arc.ArchiveFloat(&m_letterbox_time_start);

    ArchiveEnum(m_letterbox_dir, letterboxdir_t);
    m_badPlaces.Archive(arc, &ArchiveBadPlace);
    arc.ArchiveInteger(&m_iCuriousVoiceTime);
    arc.ArchiveInteger(&m_iAttackEntryAnimTime);
    arc.ArchiveInteger(&mHealthPopCount);

    arc.ArchiveBoolean(&m_bAlarm);
    arc.ArchiveBoolean(&mbNoDropHealth);
    arc.ArchiveBoolean(&mbNoDropWeapons);
    arc.ArchiveInteger(&m_iPapersLevel);
    arc.ArchiveInteger(&m_LoopProtection);

    // clear skel indexes
    memset(skel_index, 0xff, sizeof(skel_index));

    if (arc.Loading()) {
        str saved = saved_soundtrack;
        ChangeSoundtrack(current_soundtrack);
        saved_soundtrack = saved;

        memset(&impact_trace, 0, sizeof(trace_t));
    }

    for (int i = 0; i < MAX_HEAD_SENTIENTS; i++) {
        arc.ArchiveObjectPointer((Class **)&m_HeadSentient[i]);
    }

    arc.ArchiveVector(&m_vObjectiveLocation);
    arc.ArchiveVector(&m_vAlliedObjectiveLocation);
    arc.ArchiveVector(&m_vAxisObjectiveLocation);

    for (int i = 0; i < MAX_BODYQUEUE; i++) {
        arc.ArchiveSafePointer(&Actor::mBodyQueue[i]);
    }

    arc.ArchiveInteger(&Actor::mCurBody);
    Health::ArchiveStatic(arc);

    arc.ArchiveConfigString(CS_CURRENT_OBJECTIVE);

    for (int i = CS_OBJECTIVES; i < CS_OBJECTIVES + MAX_OBJECTIVES; i++) {
        arc.ArchiveConfigString(i);
    }

    arc.ArchiveConfigString(CS_RAIN_DENSITY);
    arc.ArchiveConfigString(CS_RAIN_SPEED);
    arc.ArchiveConfigString(CS_RAIN_SPEEDVARY);
    arc.ArchiveConfigString(CS_RAIN_SLANT);
    arc.ArchiveConfigString(CS_RAIN_LENGTH);
    arc.ArchiveConfigString(CS_RAIN_MINDIST);
    arc.ArchiveConfigString(CS_RAIN_WIDTH);
    arc.ArchiveConfigString(CS_RAIN_SHADER);
    arc.ArchiveConfigString(CS_RAIN_NUMSHADERS);

    arc.ArchiveFloat(&svsStartFloatTime);
    arc.ArchiveFloat(&m_fLandmarkYDistMax);
    arc.ArchiveFloat(&m_fLandmarkYDistMin);
    arc.ArchiveFloat(&m_fLandmarkXDistMin);
    arc.ArchiveFloat(&m_fLandmarkXDistMax);
}

void WriteStatS(FILE* pFile, const char* value) {
    fprintf(pFile, "%s", value);
}

void WriteStatI(FILE* pFile, int value) {
    fprintf(pFile, "%d", value);
}

void WriteStatF(FILE* pFile, float value) {
    fprintf(pFile, "%.2f", value);
}

void Level::OpenActorStats()
{
    str filename;

    if (!g_aistats->integer) {
        return;
    }

    filename = "aistats_" + mapname + ".csv";
    m_pAIStats = fopen(filename, "wt");

    if (m_pAIStats) {
        fprintf((FILE*)m_pAIStats, "targetname,weapon,grenade ammo,health,accuracy,hearing,sight,fov,mindist,maxdist,leash,sound awareness,noticescale,enemyshare,grenade aware,model\n");
    }
}

void Level::WriteActorStats(Actor* actor)
{
    const char* name;
    str itemName;
    Weapon* weapon;
    Ammo* ammo;
    int ammoAmount;
    FILE* pFile;

    if (!g_aistats->integer) {
        return;
    }

    if (!m_pAIStats) {
        OpenActorStats();
    }

    pFile = (FILE*)m_pAIStats;
    if (!pFile) {
        return;
    }

    name = actor->TargetName().c_str();
    itemName = "none";

    weapon = actor->GetActiveWeapon(WEAPON_MAIN);
    if (weapon) {
        itemName = weapon->GetItemName();
    }

    ammoAmount = 0;
    ammo = actor->FindAmmoByName("grenade");
    if (ammo) {
        ammoAmount = ammo->getAmount();
    }

    WriteStatS(pFile, name);
    WriteStatS(pFile, itemName);
    WriteStatI(pFile, ammoAmount);
    WriteStatF(pFile, actor->health);
    WriteStatF(pFile, actor->mAccuracy * 100);
    WriteStatF(pFile, actor->m_fHearing);
    WriteStatF(pFile, actor->m_fSight);
    WriteStatF(pFile, actor->m_fFov);
    WriteStatF(pFile, actor->m_fMinDistance);
    WriteStatF(pFile, actor->m_fMaxDistance);
    WriteStatF(pFile, actor->m_fLeash);
    WriteStatF(pFile, actor->m_fSoundAwareness);
    WriteStatF(pFile, actor->m_fMaxNoticeTimeScale * 100);

    WriteStatI(pFile, sqrt(actor->m_fMaxShareDistSquared));
    WriteStatF(pFile, actor->m_fGrenadeAwareness);
    WriteStatS(pFile, actor->model);

    fprintf(pFile, "\n");
}

badplace_t::badplace_t()
    : m_fLifespan(FLT_MAX)
    , m_iTeamSide(TEAM_ALLIES)
{}
