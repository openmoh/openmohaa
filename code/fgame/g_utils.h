/*
===========================================================================
Copyright (C) 2023 the OpenMoHAA team

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

#pragma once

#include "../qcommon/str.h"

class Archiver;
class Entity;
class SimpleEntity;
class SimpleArchivedEntity;
class ScriptThread;

void G_ArchiveEdict(Archiver& arc, gentity_t *edict);

void G_AllocDebugLines(void);
void G_DeAllocDebugLines(void);

void G_TouchTriggers(Entity *ent);
void G_TouchSolids(Entity *ent);

Entity *G_FindClass(Entity *ent, const char *classname);
Entity *G_NextEntity(Entity *ent);

void G_CalcBoundsOfMove(Vector& start, Vector& end, Vector& mins, Vector& maxs, Vector *minbounds, Vector *maxbounds);

void G_ShowTrace(trace_t *trace, const gentity_t *passent, const char *reason);
void G_ShowSightTrace(trace_t *trace, const gentity_t *passent, const char *reason);

bool G_SightTrace(
    const Vector& start,
    const Vector& mins,
    const Vector& maxs,
    const Vector& end,
    Entity       *passent,
    Entity       *passent2,
    int           contentmask,
    qboolean      cylindrical,
    const char   *reason
);
bool G_SightTrace(
    const Vector& start,
    const Vector& mins,
    const Vector& maxs,
    const Vector& end,
    gentity_t    *passent,
    gentity_t    *passent2,
    int           contentmask,
    qboolean      cylindrical,
    const char   *reason
);

void G_PMDrawTrace(
    trace_t     *results,
    const vec3_t start,
    const vec3_t mins,
    const vec3_t maxs,
    const vec3_t end,
    int          passEntityNum,
    int          contentMask,
    qboolean     cylinder,
    qboolean     traceDeep
);

trace_t G_Trace(
    const Vector& start,
    const Vector& mins,
    const Vector& maxs,
    const Vector& end,
    const Entity *passent,
    int           contentmask,
    qboolean      cylindrical,
    const char   *reason,
    qboolean      tracedeep = qfalse
);
trace_t G_Trace(
    vec3_t           start,
    vec3_t           mins,
    vec3_t           maxs,
    vec3_t           end,
    const gentity_t *passent,
    int              contentmask,
    qboolean         cylindrical,
    const char      *reason,
    qboolean         tracedeep = qfalse
);
void G_TraceEntities(
    Vector             & start,
    Vector             & mins,
    Vector             & maxs,
    Vector             & end,
    Container<Entity *> *victimlist,
    int                  contentmask,
    qboolean             bIncludeTriggers = qfalse
);

void SelectSpawnPoint(Vector& org, Vector& angles, str& thread);

Entity       *G_FindTarget(Entity *ent, const char *name);
Entity       *G_FindRandomTarget(const char *name);
SimpleEntity *G_FindRandomSimpleTarget(const char *name);
Entity       *G_NextEntity(Entity *ent);

qboolean M_CheckBottom(Entity *ent);

Vector   G_GetMovedir(float angle);
qboolean KillBox(Entity *ent);
qboolean IsNumeric(const char *str);

Entity *findradius(Entity *startent, Vector org, float rad);
Entity *findclientsinradius(Entity *startent, Vector org, float rad);

Vector G_CalculateImpulse(Vector start, Vector end, float speed, float gravity);
Vector G_PredictPosition(Vector start, Vector target, Vector targetvelocity, float speed);

qboolean      G_LoadAndExecScript(const char *filename, const char *label = NULL, qboolean quiet = qfalse);
ScriptThread *ExecuteThread(str thread_name, qboolean start = true);

int      MOD_string_to_int(const str     &immune_string);
qboolean MOD_matches(int incoming_damage, int damage_type);

void G_MissionFailed(void);
void G_FadeOut(float delaytime);
void G_FadeSound(float delaytime);
void G_RestartLevelWithDelay(float delaytime);
void G_PlayerDied(float delaytime);
void G_AutoFadeIn(void);
void G_ClearFade(void);
void G_StartCinematic(void);
void G_StopCinematic(void);

// copy the entity exactly including all its children
void CloneEntity(Entity *dest, Entity *src);

qboolean OnSameTeam(Entity *ent1, Entity *ent2);

//
// caching commands
//
void CacheResource(const char *stuff);
int  modelIndex(const char *mdl);

void G_SetTrajectory(gentity_t *ent, vec3_t org);
void G_SetConstantLight(
    int *constantlight, float *red, float *green, float *blue, float *radius, int *lightstyle = NULL
);

void ChangeMusic(const char *current, const char *fallback, qboolean force);

void ChangeMusicVolume(float volume, float fade_time);

void RestoreMusicVolume(float fade_time);

void ChangeSoundtrack(const char *soundtrack);

void RestoreSoundtrack(void);

void G_BroadcastSound(Entity *ent, Vector origin, float radius = SOUND_RADIUS);

SimpleArchivedEntity *G_FindArchivedClass(SimpleArchivedEntity *ent, const char *classname);
Entity               *G_FindClass(Entity *ent, const char *classname);
int                   G_GetClientNumber(gentity_t *ent);
int                   G_GetClientNumber(Entity *ent);
Entity               *G_GetEntityByClient(int clientNum);
int                   G_GetEntityIndex(int number);
int                   G_GetEntityIndex(gentity_t *ent);
gentity_t            *G_GetGEntity(int id);

//==================================================================
//
// Inline functions
//
//==================================================================

/*
=================
G_GetEntity

Takes an index to an entity and returns pointer to it.
=================
*/

inline Entity *G_GetEntity(int entnum)

{
    if ((entnum < 0) || (entnum >= globals.max_entities)) {
        gi.Error(ERR_DROP, "G_GetEntity: %d out of valid range.", entnum);
    }

    return (Entity *)g_entities[entnum].entity;
}

/*
=================
G_Random

Returns a number from 0<= num < 1

random()
=================
*/

inline float G_Random(void)

{
    return ((float)(rand() & 0x7fff)) / ((float)0x8000);
}

/*
=================
G_Random

Returns a number from 0 <= num < n

random()
=================
*/

inline float G_Random(float n)

{
    return G_Random() * n;
}

/*
=================
G_CRandom

Returns a number from -1 <= num < 1

crandom()
=================
*/

inline float G_CRandom(void)

{
    return G_Random(2) - 1;
}

/*
=================
G_CRandom

Returns a number from -n <= num < n

crandom()
=================
*/

inline float G_CRandom(float n)

{
    return G_CRandom() * n;
}

/*
=================
G_FixSlashes

Converts all backslashes in a string to forward slashes.
Used to make filenames consistant.
=================
*/

inline str G_FixSlashes(const char *filename)

{
    int    i;
    size_t len;
    str    text;

    if (filename) {
        // Convert all forward slashes to back slashes
        text = filename;
        len  = text.length();
        for (i = 0; i < len; i++) {
            if (text[i] == '\\') {
                text[i] = '/';
            }
        }
    }

    return text;
}

typedef enum {
    WEAPON_MAIN,
    WEAPON_OFFHAND,
    WEAPON_ERROR
} weaponhand_t;

#define NUM_ACTIVE_WEAPONS WEAPON_ERROR

typedef enum {
    FIRE_PRIMARY,
    FIRE_SECONDARY,
    MAX_FIREMODES,
    FIRE_ERROR
} firemode_t;

firemode_t   WeaponModeNameToNum(str mode);
const char  *WeaponHandNumToName(weaponhand_t hand);
weaponhand_t WeaponHandNameToNum(str side);
void         G_DebugTargets(Entity *e, str from);
void         G_DebugDamage(float damage, Entity *victim, Entity *attacker, Entity *inflictor);
