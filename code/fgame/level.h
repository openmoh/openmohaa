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

// level.h: General Level Info

#pragma once

#include "listener.h"
#include "g_public.h"

#define MAX_HEAD_SENTIENTS 2
#define MAX_EARTHQUAKES    10

enum INTTYPE_e {
    TRANS_BSP,
    TRANS_LEVEL,
    TRANS_MISSION,
    TRANS_MISSION_FAILED
};

enum fadetype_t {
    fadein,
    fadeout
};

enum fadestyle_t {
    alphablend,
    additive
};

enum letterboxdir_t {
    letterbox_in,
    letterbox_out
};

typedef struct earthquake_s {
    int   duration;
    float magnitude;
    bool  no_rampup;
    bool  no_rampdown;

    int                   starttime;
    int                   endtime;
    SafePtr<ScriptThread> m_Thread;
} earthquake_t;

extern gclient_t *spawn_client;

class Camera;
class SimpleArchivedEntity;

class Level : public Listener
{
public:
    char *current_map;

    int mHealthPopCount;

    // Map name stuff
    str m_mapscript;
    str m_precachescript;
    str m_pathfile;
    str m_mapfile;

    // Spawning variables
    int spawn_entnum;
    int spawnflags;

    // Level time
    int framenum;
    int inttime;
    int intframetime;

    float time;
    float frametime;

    // Server time
    int   svsTime;
    float svsFloatTime;
    int   svsStartTime;
    int   svsEndTime;

    // Level name variables
    str level_name;
    str mapname;
    str spawnpoint;
    str nextmap;

    // AI variables
    int m_iCuriousVoiceTime;
    int m_iAttackEntryAnimTime;

    // Global players variables
    qboolean playerfrozen;

    // Intermission variables
    float     intermissiontime;
    INTTYPE_e intermissiontype;
    int       exitintermission;

    // Secrets
    int total_secrets;
    int found_secrets;

    // Trace
    trace_t impact_trace;

    // Earthquake stuff
    float earthquake_magnitude;

    // Cinematic
    qboolean cinematic;

    // AI variables
    qboolean ai_on;
    qboolean m_bAlarm;
    qboolean mbNoDropHealth;
    int      m_iPapersLevel;

    // Mission state
    qboolean mission_failed;
    qboolean died_already;
    qboolean near_exit;

    // Water variables
    Vector water_color;
    Vector lava_color;
    float  water_alpha;
    float  lava_alpha;

    // Soundtrack variables
    str current_soundtrack;
    str saved_soundtrack;

    // Screen state
    Vector      m_fade_color;
    float       m_fade_alpha;
    float       m_fade_time;
    float       m_fade_time_start;
    fadetype_t  m_fade_type;
    fadestyle_t m_fade_style;

    // Letterbox state
    float          m_letterbox_fraction;
    float          m_letterbox_time;
    float          m_letterbox_time_start;
    letterboxdir_t m_letterbox_dir;

    Container<Camera *> automatic_cameras;

    // Quake remnants ?
    int m_numArenas;

    // Voting variables
    float m_voteTime;
    int   m_voteYes;
    int   m_voteNo;
    int   m_numVoters;
    str   m_voteString;

    // Intermission locations
    Vector m_intermission_origin;
    Vector m_intermission_angle;

    // Skeleton variables
    int frame_skel_index;
    int skel_index[MAX_GENTITIES];

    // script variables
    qboolean m_LoopProtection;
    qboolean m_LoopDrop; // kill the server in loop exception

    Container<SimpleArchivedEntity *> m_SimpleArchivedEntities;
    class Sentient                   *m_HeadSentient[MAX_HEAD_SENTIENTS];

    // Earthquake stuff
    earthquake_t earthquakes[MAX_EARTHQUAKES];
    int          num_earthquakes;

    // Objective stuff
    Vector m_vObjectiveLocation;

    // Current level state
    bool spawning;
    bool m_bIgnoreClock;

    // New Stuff

    // don't put in the game class because it can be changed in another level
    qboolean reborn;

    bool m_bSpawnBot;

    // Script stuff
    bool m_bScriptSpawn;
    bool m_bRejectSpawn;

public:
    CLASS_PROTOTYPE(Level);

    Level();
    virtual ~Level();

    void       Init();
    void       CleanUp(qboolean samemap = qfalse, qboolean resetConfigStrings = qfalse);
    void       ResetEdicts();
    gentity_t *AllocEdict(Entity *ent);
    void       FreeEdict(gentity_t *ent);
    void       InitEdict(gentity_t *ent);
    void       LoadAllScripts(const char *name, const char *extension);
    void       Precache();
    void       SetMap(const char *themapname);
    void       FindTeams();
    void       SpawnEntities(char *entities, int _svsTime_);
    void       PreSpawnSentient(Event *ev);
    void       ServerSpawned();
    qboolean   inhibitEntity(int spawnflags);
    void       setSkill(int value);
    void       setTime(int _svsTime_);
    void       setFrametime(int frameTime);
    void       AddAutomaticCamera(Camera *cam);
    void       GetTime(Event *ev);
    void       GetTotalSecrets(Event *ev);
    void       GetFoundSecrets(Event *ev);
    void       CheckVote();
    void       GetAlarm(Event *ev);
    void       SetAlarm(Event *ev);
    void       SetNoDropHealth(Event *ev);
    void       GetLoopProtection(Event *ev);
    void       SetLoopProtection(Event *ev);
    void       GetPapersLevel(Event *ev);
    void       SetPapersLevel(Event *ev);
    void       EventGetDMRespawning(Event *ev);
    void       EventSetDMRespawning(Event *ev);
    void       EventGetDMRoundLimit(Event *ev);
    void       EventSetDMRoundLimit(Event *ev);
    void       EventGetClockSide(Event *ev);
    void       EventSetClockSide(Event *ev);
    void       EventGetBombPlantTeam(Event *ev);
    void       EventSetBombPlantTeam(Event *ev);
    void       EventGetTargetsToDestroy(Event *ev);
    void       EventSetTargetsToDestroy(Event *ev);
    void       EventGetTargetsDestroyed(Event *ev);
    void       EventSetTargetsDestroyed(Event *ev);
    void       EventGetBombsPlanted(Event *ev);
    void       EventSetBombsPlanted(Event *ev);
    void       EventGetRoundBased(Event *ev);
    void       EventRainDensitySet(Event *ev);
    void       EventRainDensityGet(Event *ev);
    void       EventRainSpeedSet(Event *ev);
    void       EventRainSpeedGet(Event *ev);
    void       EventRainSpeedVarySet(Event *ev);
    void       EventRainSpeedVaryGet(Event *ev);
    void       EventRainSlantSet(Event *ev);
    void       EventRainSlantGet(Event *ev);
    void       EventRainLengthSet(Event *ev);
    void       EventRainLengthGet(Event *ev);
    void       EventRainMin_DistSet(Event *ev);
    void       EventRainMin_DistGet(Event *ev);
    void       EventRainWidthSet(Event *ev);
    void       EventRainWidthGet(Event *ev);
    void       EventRainShaderSet(Event *ev);
    void       EventRainShaderGet(Event *ev);
    void       EventRainNumShadersSet(Event *ev);
    void       EventRainNumShadersGet(Event *ev);
    str        GetRandomHeadModel(const char *model);
    str        GetRandomHeadSkin(const char *model);

    bool RoundStarted();
    bool PreSpawned(void);
    bool Spawned(void);

    void AddEarthquake(earthquake_t *);
    void DoEarthquakes(void);

    qboolean Reborn(void);

    void Archive(Archiver& arc) override;
};

extern Level level;

inline void Level::GetTime(Event *ev)
{
    ev->AddFloat(level.time);
}

inline void Level::GetTotalSecrets(Event *ev)
{
    ev->AddInteger(total_secrets);
}

inline void Level::GetFoundSecrets(Event *ev)
{
    ev->AddInteger(found_secrets);
}

inline qboolean Level::Reborn(void)
{
    return reborn;
}
