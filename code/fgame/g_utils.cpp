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

#include "g_local.h"
#include "g_utils.h"
#include "ctype.h"
#include "world.h"
#include "scriptmaster.h"
#include "scriptthread.h"
#include "player.h"
#include "playerbot.h"
#include "playerstart.h"
#include "debuglines.h"
#include "../qcommon/tiki.h"

const char *means_of_death_strings[MOD_TOTAL_NUMBER] = {
    "none",
    "suicide",
    "crush",
    "crush_every_frame",
    "telefrag",
    "lava",
    "slime",
    "falling",
    "last_self_inflicted",
    "explosion",
    "explodewall",
    "electric",
    "electric_water",
    "thrown_object",
    "grenade",
    "beam",
    "rocket",
    "impact",
    "bullet",
    "fast_bullet",
    "vehicle",
    "fire",
    "flashbang",
    "on_fire",
    "gib",
    "impale",
    "bash",
    "shotgun",
    "aagun",
    "landmine"};

int MOD_string_to_int(const str& immune_string)
{
    int i;

    for (i = 0; i < MOD_TOTAL_NUMBER; i++) {
        if (!immune_string.icmp(means_of_death_strings[i])) {
            return i;
        }
    }

    gi.DPrintf("Unknown means of death - %s\n", immune_string.c_str());
    return -1;
}

qboolean MOD_matches(int incoming_damage, int damage_type)
{
    if (damage_type == -1) {
        return true;
    } else {
        return incoming_damage == damage_type;
    }
}

/*
============
G_TouchTriggers

============
*/
void G_TouchTriggers(Entity *ent)

{
    int        i;
    int        num;
    int        touch[MAX_GENTITIES];
    gentity_t *hit;
    Event     *ev;

    // dead things don't activate triggers!
    if ((ent->client || (ent->edict->r.svFlags & SVF_MONSTER)) && (ent->IsDead())) {
        return;
    }

    num = gi.AreaEntities(ent->absmin, ent->absmax, touch, MAX_GENTITIES);

    // be careful, it is possible to have an entity in this
    // list removed before we get to it (killtriggered)
    for (i = 0; i < num; i++) {
        hit = &g_entities[touch[i]];
        if (!hit->inuse || (hit->entity == ent) || (hit->solid != SOLID_TRIGGER)) {
            continue;
        }

        // Added in 2.0
        // check if the ent is inside edict
        if (hit->r.bmodel && gi.HitEntity(ent->edict, hit)) {
            continue;
        }

        assert(hit->entity);

        ev = new Event(EV_Touch);
        ev->AddEntity(ent);
        hit->entity->ProcessEvent(ev);
    }
}

/*
============
G_TouchSolids

Call after linking a new trigger in during gameplay
to force all entities it covers to immediately touch it
============
*/
void G_TouchSolids(Entity *ent)

{
    int        i;
    int        num;
    int        touch[MAX_GENTITIES];
    gentity_t *hit;
    Event     *ev;

    num = gi.AreaEntities(ent->absmin, ent->absmax, touch, MAX_GENTITIES);

    // be careful, it is possible to have an entity in this
    // list removed before we get to it (killtriggered)
    for (i = 0; i < num; i++) {
        hit = &g_entities[touch[i]];
        if (!hit->inuse) {
            continue;
        }

        assert(hit->entity);

        //FIXME
        // should we post the events so that we don't have to worry about any entities going away
        ev = new Event(EV_Touch);
        ev->AddEntity(ent);
        hit->entity->ProcessEvent(ev);
    }
}

void G_ShowTrace(trace_t *trace, const gentity_t *passent, const char *reason)

{
    str text;
    str pass;
    str hit;

    assert(reason);
    assert(trace);

    if (passent) {
        pass = va("'%s'(%d)", passent->entname, passent->s.number);
    } else {
        pass = "NULL";
    }

    if (trace->ent) {
        hit = va("'%s'(%d)", trace->ent->entname, trace->ent->s.number);
    } else {
        hit = "NULL";
    }

    text =
        va("%0.2f : Pass %s Frac %f Hit %s : '%s'\n",
           level.time,
           pass.c_str(),
           trace->fraction,
           hit.c_str(),
           reason ? reason : "");

    if (sv_traceinfo->integer == 3) {
        gi.DebugPrintf(text.c_str());
    } else {
        gi.DPrintf("%s", text.c_str());
    }
}

void G_ShowSightTrace(gentity_t *passent1, gentity_t *passent2, const char *reason)
{
    str text;
    str pass1;
    str pass2;

    assert(reason);

    if (passent1) {
        pass1 = va("'%s'(%d)", passent1->entname, passent1->s.number);
    } else {
        pass1 = "NULL";
    }

    if (passent2) {
        pass2 = va("'%s'(%d)", passent2->entname, passent2->s.number);
    } else {
        pass2 = "NULL";
    }

    text = va("%0.2f : Pass1 %s Pass2 %s : '%s'\n", level.time, pass1.c_str(), pass2.c_str(), reason ? reason : "");

    if (sv_traceinfo->integer == 3) {
        gi.DebugPrintf(text.c_str());
    } else {
        gi.DPrintf("%s", text.c_str());
    }
}

void G_CalcBoundsOfMove(Vector& start, Vector& end, Vector& mins, Vector& maxs, Vector *minbounds, Vector *maxbounds)

{
    Vector bmin;
    Vector bmax;

    ClearBounds(bmin, bmax);
    AddPointToBounds(start, bmin, bmax);
    AddPointToBounds(end, bmin, bmax);
    bmin += mins;
    bmax += maxs;

    if (minbounds) {
        *minbounds = bmin;
    }

    if (maxbounds) {
        *maxbounds = bmax;
    }
}

bool G_SightTrace(
    const Vector& start,
    const Vector& mins,
    const Vector& maxs,
    const Vector& end,
    gentity_t    *passent,
    gentity_t    *passent2,
    int           contentmask,
    qboolean      cylinder,
    const char   *reason
)
{
    int  entnum, entnum2;
    bool result;

    assert(reason);

    if (passent == NULL) {
        entnum = ENTITYNUM_NONE;
    } else {
        entnum = passent->s.number;
    }

    if (passent2 == NULL) {
        entnum2 = ENTITYNUM_NONE;
    } else {
        entnum2 = passent2->s.number;
    }

    result = gi.SightTrace(start, mins, maxs, end, entnum, entnum2, contentmask, cylinder) ? true : false;

    if (sv_traceinfo->integer > 1) {
        G_ShowSightTrace(passent, passent2, reason);
    }

    sv_numtraces++;

    if (sv_drawtrace->integer) {
        G_DebugLine(start, end, 1, 1, 0, 1);
    }

    return result;
}

bool G_SightTrace(
    const Vector& start,
    const Vector& mins,
    const Vector& maxs,
    const Vector& end,
    Entity       *passent,
    Entity       *passent2,
    int           contentmask,
    qboolean      cylinder,
    const char   *reason
)
{
    gentity_t *ent, *ent2;
    int        entnum, entnum2;
    bool       result;

    assert(reason);

    if (passent == NULL || !passent->isSubclassOf(Entity)) {
        ent    = NULL;
        entnum = ENTITYNUM_NONE;
    } else {
        ent    = passent->edict;
        entnum = ent->s.number;
    }

    if (passent2 == NULL || !passent2->isSubclassOf(Entity)) {
        ent2    = NULL;
        entnum2 = ENTITYNUM_NONE;
    } else {
        ent2    = passent2->edict;
        entnum2 = ent2->s.number;
    }

    result = gi.SightTrace(start, mins, maxs, end, entnum, entnum2, contentmask, cylinder) ? true : false;

    if (sv_traceinfo->integer > 1) {
        G_ShowSightTrace(ent, ent2, reason);
    }

    sv_numtraces++;

    if (sv_drawtrace->integer) {
        G_DebugLine(start, end, 1, 1, 0, 1);
    }

    return result;
}

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
)
{
    gi.trace(results, start, mins, maxs, end, passEntityNum, contentMask, cylinder, traceDeep);
    sv_numpmtraces++;
    G_DebugLine(start, end, 1.f, 0.75f, 0.5f, 1.f);
}

trace_t G_Trace(
    vec3_t           start,
    vec3_t           mins,
    vec3_t           maxs,
    vec3_t           end,
    const gentity_t *passent,
    int              contentmask,
    qboolean         cylinder,
    const char      *reason,
    qboolean         tracedeep
)

{
    int     entnum;
    trace_t trace;

    if (passent) {
        entnum = passent->s.number;
    } else {
        entnum = ENTITYNUM_NONE;
    }

    gi.trace(&trace, start, mins, maxs, end, entnum, contentmask, cylinder, tracedeep);

    if (trace.entityNum == ENTITYNUM_NONE) {
        trace.ent = NULL;
    } else {
        trace.ent = &g_entities[trace.entityNum];
    }

    if (sv_traceinfo->integer > 1) {
        G_ShowTrace(&trace, passent, reason);
    }
    sv_numtraces++;

    if (sv_drawtrace->integer) {
        G_DebugLine(Vector(start), Vector(end), 1, 1, 0, 1);
    }

    return trace;
}

trace_t G_Trace(
    const Vector& start,
    const Vector& mins,
    const Vector& maxs,
    const Vector& end,
    const Entity *passent,
    int           contentmask,
    qboolean      cylinder,
    const char   *reason,
    qboolean      tracedeep
)

{
    gentity_t *ent;
    int        entnum;
    trace_t    trace;

    assert(reason);

    if (passent == NULL) {
        ent    = NULL;
        entnum = ENTITYNUM_NONE;
    } else {
        ent    = passent->edict;
        entnum = ent->s.number;
    }

    gi.trace(&trace, start, mins, maxs, end, entnum, contentmask, cylinder, tracedeep);

    if (trace.entityNum == ENTITYNUM_NONE) {
        trace.ent = NULL;
    } else {
        trace.ent = &g_entities[trace.entityNum];
    }

    if (sv_traceinfo->integer > 1) {
        G_ShowTrace(&trace, ent, reason);
    }

    sv_numtraces++;

    if (sv_drawtrace->integer) {
        G_DebugLine(start, end, 1, 1, 0, 1);
    }

    return trace;
}

void G_TraceEntities(
    Vector             & start,
    Vector             & mins,
    Vector             & maxs,
    Vector             & end,
    Container<Entity *> *victimlist,
    int                  contentmask,
    qboolean             bIncludeTriggers
)

{
    trace_t    trace;
    vec3_t     boxmins;
    vec3_t     boxmaxs;
    int        num;
    int        touchlist[MAX_GENTITIES];
    gentity_t *touch;
    int        i;

    // Find the bounding box

    for (i = 0; i < 3; i++) {
        if (end[i] > start[i]) {
            boxmins[i] = start[i] + mins[i] - 1;
            boxmaxs[i] = end[i] + maxs[i] + 1;
        } else {
            boxmins[i] = end[i] + mins[i] - 1;
            boxmaxs[i] = start[i] + maxs[i] + 1;
        }
    }

    // Find the list of entites

    num = gi.AreaEntities(boxmins, boxmaxs, touchlist, MAX_GENTITIES);

    for (i = 0; i < num; i++) {
        touch = &g_entities[touchlist[i]];

        // see if we should ignore this entity
        if (touch->solid == SOLID_NOT) {
            continue;
        }
        if (touch->solid == SOLID_TRIGGER && !bIncludeTriggers) {
            continue;
        }

        gi.ClipToEntity(&trace, start, mins, maxs, end, touchlist[i], contentmask);

        if (trace.entityNum == touchlist[i]) {
            victimlist->AddObject(touch->entity);
        }
    }
}

/*
=======================================================================

  SelectSpawnPoint

=======================================================================
*/

/*
================
PlayersRangeFromSpot

Returns the distance to the nearest player from the given spot
================
*/
float PlayersRangeFromSpot(Entity *spot)

{
    Entity *player;
    float   bestplayerdistance;
    Vector  v;
    int     n;
    float   playerdistance;

    bestplayerdistance = 9999999;
    for (n = 0; n < maxclients->integer; n++) {
        if (!g_entities[n].inuse || !g_entities[n].entity) {
            continue;
        }

        player = g_entities[n].entity;
        if (player->health <= 0) {
            continue;
        }

        v              = spot->origin - player->origin;
        playerdistance = v.length();

        if (playerdistance < bestplayerdistance) {
            bestplayerdistance = playerdistance;
        }
    }

    return bestplayerdistance;
}

/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point, but NOT the two points closest
to other players
================
*/
Entity *SelectRandomDeathmatchSpawnPoint(void)

{
    Entity *spot, *spot1, *spot2;
    int     count = 0;
    int     selection;
    float   range, range1, range2;

    spot   = NULL;
    range1 = range2 = 99999;
    spot1 = spot2 = NULL;

    for (spot = G_FindClass(spot, "info_player_deathmatch"); spot; spot = G_FindClass(spot, "info_player_deathmatch")) {
        count++;
        range = PlayersRangeFromSpot(spot);
        if (range < range1) {
            range1 = range;
            spot1  = spot;
        } else if (range < range2) {
            range2 = range;
            spot2  = spot;
        }
    }

    if (!count) {
        return NULL;
    }

    if (count <= 2) {
        spot1 = spot2 = NULL;
    } else {
        count -= 2;
    }

    selection = rand() % count;

    spot = NULL;
    do {
        spot = G_FindClass(spot, "info_player_deathmatch");

        // if there are no more, break out
        if (!spot) {
            break;
        }

        if (spot == spot1 || spot == spot2) {
            selection++;
        }
    } while (selection--);

    return spot;
}

/*
================
SelectFarthestDeathmatchSpawnPoint

================
*/
Entity *SelectFarthestDeathmatchSpawnPoint(void)

{
    Entity *bestspot;
    float   bestdistance;
    float   bestplayerdistance;
    Entity *spot;

    spot         = NULL;
    bestspot     = NULL;
    bestdistance = 0;
    for (spot = G_FindClass(spot, "info_player_deathmatch"); spot; spot = G_FindClass(spot, "info_player_deathmatch")) {
        bestplayerdistance = PlayersRangeFromSpot(spot);
        if (bestplayerdistance > bestdistance) {
            bestspot     = spot;
            bestdistance = bestplayerdistance;
        }
    }

    if (bestspot) {
        return bestspot;
    }

    // if there is a player just spawned on each and every start spot
    // we have no choice to turn one into a telefrag meltdown
    spot = G_FindClass(NULL, "info_player_deathmatch");

    return spot;
}

Entity *SelectDeathmatchSpawnPoint(void)

{
    if (DM_FLAG(DF_SPAWN_FARTHEST)) {
        return SelectFarthestDeathmatchSpawnPoint();
    } else {
        return SelectRandomDeathmatchSpawnPoint();
    }
}

/*
=============
M_CheckBottom

Returns false if any part of the bottom of the entity is off an edge that
is not a staircase.

=============
*/
int c_yes, c_no;

qboolean M_CheckBottom(Entity *ent)

{
    Vector  mins, maxs, start, stop;
    trace_t trace;
    int     x, y;
    float   mid, bottom;

    mins = ent->origin + ent->mins * 0.5;
    maxs = ent->origin + ent->maxs * 0.5;

    // if all of the points under the corners are solid world, don't bother
    // with the tougher checks
    // the corners must be within 16 of the midpoint
    start[2] = mins[2] - 1;

    for (x = 0; x <= 1; x++) {
        for (y = 0; y <= 1; y++) {
            start[0] = x ? maxs[0] : mins[0];
            start[1] = y ? maxs[1] : mins[1];
            if (gi.pointcontents(start, 0) != CONTENTS_SOLID) {
                goto realcheck;
            }
        }
    }

    c_yes++;
    return true; // we got out easy

realcheck:

    c_no++;

    //
    // check it for real...
    //
    start[2] = mins[2];

    // the midpoint must be within 16 of the bottom
    start[0] = stop[0] = (mins[0] + maxs[0]) * 0.5;
    start[1] = stop[1] = (mins[1] + maxs[1]) * 0.5;
    stop[2]            = start[2] - 3 * STEPSIZE; //2 * STEPSIZE;

    trace = G_Trace(start, vec_zero, vec_zero, stop, ent, MASK_MONSTERSOLID, false, "M_CheckBottom 1");

    if (trace.fraction == 1.0) {
        return false;
    }

    mid = bottom = trace.endpos[2];

    // the corners must be within 16 of the midpoint
    for (x = 0; x <= 1; x++) {
        for (y = 0; y <= 1; y++) {
            start[0] = stop[0] = x ? maxs[0] : mins[0];
            start[1] = stop[1] = y ? maxs[1] : mins[1];

            trace = G_Trace(start, vec_zero, vec_zero, stop, ent, MASK_MONSTERSOLID, false, "M_CheckBottom 2");

            if (trace.fraction != 1.0 && trace.endpos[2] > bottom) {
                bottom = trace.endpos[2];
            }

            if (trace.fraction == 1.0 || mid - trace.endpos[2] > STEPSIZE) {
                return false;
            }
        }
    }

    c_yes++;
    return true;
}

Entity *G_FindClass(Entity *ent, const char *classname)

{
    int        entnum;
    gentity_t *from;

    if (ent) {
        entnum = ent->entnum;
    } else {
        entnum = -1;
    }

    for (from = &g_entities[entnum + 1]; from < &g_entities[globals.num_entities]; from++) {
        if (!from->inuse) {
            continue;
        }
        if (!Q_stricmp(from->entity->getClassID(), classname)) {
            return from->entity;
        }
    }

    return NULL;
}

Entity *G_FindTarget(Entity *ent, const char *name)

{
    SimpleEntity *next;

    if (name && name[0]) {
        next = world->GetNextEntity(str(name), ent);
        if (next && next->IsSubclassOfEntity()) {
            return static_cast<Entity *>(next);
        }
    }

    return NULL;
}

SimpleEntity *G_FindRandomSimpleTarget(const char *name)
{
    SimpleEntity *found = NULL, *ent = world;
    int           nFound = 0;

    if (name && *name) {
        while (true) {
            ent = world->GetNextEntity(name, ent);
            if (!ent) {
                break;
            }

            if (++nFound * rand() <= 0x7FFF) {
                found = ent;
            }
        }
    }
    return found;
}

Entity *G_FindRandomTarget(const char *name)
{
    SimpleEntity *found = NULL, *ent = world;
    int           nFound = 0;

    if (name && *name) {
        while (true) {
            ent = world->GetNextEntity(name, ent);
            if (!ent) {
                break;
            }

            if (ent->IsSubclassOfEntity() && ++nFound * rand() <= 0x7FFF) {
                found = ent;
            }
        }
    }
    return (Entity *)found;
}

Entity *G_NextEntity(Entity *ent)

{
    gentity_t *from;

    if (!g_entities) {
        return NULL;
    }

    if (!ent) {
        from = g_entities;
    } else {
        from = ent->edict + 1;
    }

    if (!from) {
        return NULL;
    }

    for (; from < &g_entities[globals.num_entities]; from++) {
        if (!from->inuse || !from->entity) {
            continue;
        }

        return from->entity;
    }

    return NULL;
}

//
// QuakeEd only writes a single float for angles (bad idea), so up and down are
// just constant angles.
//
Vector G_GetMovedir(float angle)

{
    if (angle == -1) {
        return Vector(0, 0, 1);
    } else if (angle == -2) {
        return Vector(0, 0, -1);
    }

    angle *= (M_PI * 2 / 360);
    return Vector(cos(angle), sin(angle), 0);
}

/*
===============
G_SetMovedir

The editor only specifies a single value for angles (yaw),
but we have special constants to generate an up or down direction.
Angles will be cleared, because it is being used to represent a direction
instead of an orientation.
===============
*/
void G_SetMovedir(vec3_t angles, vec3_t movedir)
{
    static vec3_t VEC_UP       = {0, -1, 0};
    static vec3_t MOVEDIR_UP   = {0, 0, 1};
    static vec3_t VEC_DOWN     = {0, -2, 0};
    static vec3_t MOVEDIR_DOWN = {0, 0, -1};

    if (VectorCompare(angles, VEC_UP)) {
        VectorCopy(MOVEDIR_UP, movedir);
    } else if (VectorCompare(angles, VEC_DOWN)) {
        VectorCopy(MOVEDIR_DOWN, movedir);
    } else {
        AngleVectors(angles, movedir, NULL, NULL);
    }
    VectorClear(angles);
}

/*
=================
KillBox

Kills all entities that would touch the proposed new positioning
of ent.  Ent should be unlinked before calling this!
=================
*/
qboolean KillBox(Entity *ent)

{
    int        i;
    int        num;
    int        touch[MAX_GENTITIES];
    gentity_t *hit;
    Vector     min;
    Vector     max;
    int        fail;

    fail = 0;

    min = ent->origin + ent->mins;
    max = ent->origin + ent->maxs;

    num = gi.AreaEntities(min, max, touch, MAX_GENTITIES);

    for (i = 0; i < num; i++) {
        hit = &g_entities[touch[i]];

        if (!hit->inuse || (hit->entity == ent) || !hit->entity || (hit->entity == world)
            || (!hit->entity->edict->solid)) {
            continue;
        }

        hit->entity->Damage(
            ent,
            ent,
            hit->entity->health + 100000,
            ent->origin,
            vec_zero,
            vec_zero,
            0,
            DAMAGE_NO_PROTECTION,
            MOD_TELEFRAG
        );

        //
        // if we didn't kill it, fail
        //
        if (hit->entity->getSolidType() != SOLID_NOT) {
            fail++;
        }
    }

    //
    // all clear
    //
    return !fail;
}

qboolean IsNumeric(const char *str)

{
    int      len;
    int      i;
    qboolean dot;

    if (*str == '-') {
        str++;
    }

    dot = false;
    len = strlen(str);
    for (i = 0; i < len; i++) {
        if (!isdigit(str[i])) {
            if ((str[i] == '.') && !dot) {
                dot = true;
                continue;
            }
            return false;
        }
    }

    return true;
}

/*
=================
findradius

Returns entities that have origins within a spherical area

findradius (org, radius)
=================
*/
Entity *findradius(Entity *startent, Vector org, float rad)

{
    Vector     eorg;
    gentity_t *from;
    float      r2, distance;

    if (!startent) {
        from = active_edicts.next;
    } else {
        from = startent->edict->next;
    }

    assert(from);
    if (!from) {
        return NULL;
    }

    assert((from == &active_edicts) || (from->inuse));

    // square the radius so that we don't have to do a square root
    r2 = rad * rad;

    for (; from != &active_edicts; from = from->next) {
        assert(from->inuse);
        assert(from->entity);

        eorg = org - from->entity->centroid;

        // dot product returns length squared
        distance = eorg * eorg;

        if (distance <= r2) {
            return from->entity;
        } else {
            // subtract the object's own radius from this distance
            distance -= from->radius2;
            if (distance <= r2) {
                return from->entity;
            }
        }
    }

    return NULL;
}

/*
=================
findclientinradius

Returns clients that have origins within a spherical area

findclientinradius (org, radius)
=================
*/
Entity *findclientsinradius(Entity *startent, Vector org, float rad)

{
    Vector     eorg;
    gentity_t *ed;
    float      r2;
    int        i;

    // square the radius so that we don't have to do a square root
    r2 = rad * rad;

    if (!startent) {
        i = 0;
    } else {
        i = startent->entnum + 1;
    }

    for (; i < game.maxclients; i++) {
        ed = &g_entities[i];

        if (!ed->inuse || !ed->entity) {
            continue;
        }

        eorg = org - ed->entity->centroid;

        // dot product returns length squared
        if ((eorg * eorg) <= r2) {
            return ed->entity;
        }
    }

    return NULL;
}

Vector G_CalculateImpulse(Vector start, Vector end, float speed, float gravity)

{
    float  traveltime, vertical_speed;
    Vector dir, xydir, velocity;

    dir            = end - start;
    xydir          = dir;
    xydir.z        = 0;
    traveltime     = xydir.length() / speed;
    vertical_speed = (dir.z / traveltime) + (0.5f * gravity * sv_gravity->value * traveltime);
    xydir.normalize();

    velocity   = speed * xydir;
    velocity.z = vertical_speed;
    return velocity;
}

Vector G_PredictPosition(Vector start, Vector target, Vector targetvelocity, float speed)

{
    Vector projected;
    float  traveltime;
    Vector dir, xydir;

    dir        = target - start;
    xydir      = dir;
    xydir.z    = 0;
    traveltime = xydir.length() / speed;
    projected  = target + (targetvelocity * traveltime);

    return projected;
}

/*
==============
G_ArchiveEdict
==============
*/
void G_ArchiveEdict(Archiver& arc, gentity_t *edict)

{
    int i;
    str tempStr;

    assert(edict);

    //
    // this is written funny because it is used for both saving and loading
    //

    if (edict->client) {
        arc.ArchiveRaw(edict->client, sizeof(*edict->client));
    }

    arc.ArchiveInteger(&edict->s.beam_entnum);

    for (i = 0; i < MAX_FRAMEINFOS; i++) {
        arc.ArchiveInteger(&edict->s.frameInfo[i].index);
        arc.ArchiveFloat(&edict->s.frameInfo[i].time);
        arc.ArchiveFloat(&edict->s.frameInfo[i].weight);
    }

    arc.ArchiveFloat(&edict->s.actionWeight);

    arc.ArchiveFloat(&edict->s.shader_data[0]);
    arc.ArchiveFloat(&edict->s.shader_data[1]);
    arc.ArchiveFloat(&edict->s.shader_time);

    arc.ArchiveVec3(edict->s.eyeVector);

    arc.ArchiveInteger(&edict->s.eType);
    arc.ArchiveInteger(&edict->s.eFlags);

    arc.ArchiveVec3(edict->s.netorigin);
    arc.ArchiveVec3(edict->s.origin);
    arc.ArchiveVec3(edict->s.origin2);
    arc.ArchiveVec3(edict->s.netangles);
    arc.ArchiveVec3(edict->s.angles);

    arc.ArchiveInteger(&edict->s.constantLight);

    if (arc.Saving()) {
        if (edict->s.loopSound) {
            tempStr = gi.getConfigstring(CS_SOUNDS + edict->s.loopSound);
        } else {
            tempStr = "";
        }

        arc.ArchiveString(&tempStr);
    } else {
        arc.ArchiveString(&tempStr);

        if (tempStr.length()) {
            edict->s.loopSound = gi.soundindex(tempStr.c_str(), true);
        } else {
            edict->s.loopSound = 0;
        }
    }

    arc.ArchiveFloat(&edict->s.loopSoundVolume);
    arc.ArchiveFloat(&edict->s.loopSoundMinDist);
    arc.ArchiveFloat(&edict->s.loopSoundMaxDist);
    arc.ArchiveFloat(&edict->s.loopSoundPitch);
    arc.ArchiveInteger(&edict->s.loopSoundFlags);

    arc.ArchiveInteger(&edict->s.parent);
    arc.ArchiveInteger(&edict->s.tag_num);
    arc.ArchiveBoolean(&edict->s.attach_use_angles);
    arc.ArchiveVec3(edict->s.attach_offset);

    arc.ArchiveInteger(&edict->s.skinNum);
    arc.ArchiveInteger(&edict->s.wasframe);

    for (i = 0; i < NUM_BONE_CONTROLLERS; i++) {
        arc.ArchiveInteger(&edict->s.bone_tag[i]);
        arc.ArchiveVec3(edict->s.bone_angles[i]);
        arc.ArchiveVec4(edict->s.bone_quat[i]);
    }

    arc.ArchiveRaw(&edict->s.surfaces, sizeof(edict->s.surfaces));

    arc.ArchiveInteger(&edict->s.clientNum);
    arc.ArchiveInteger(&edict->s.groundEntityNum);
    arc.ArchiveInteger(&edict->s.solid);

    arc.ArchiveFloat(&edict->s.scale);
    arc.ArchiveFloat(&edict->s.alpha);
    arc.ArchiveInteger(&edict->s.renderfx);
    arc.ArchiveVec4(edict->s.quat);
    arc.ArchiveRaw(&edict->mat, sizeof(edict->mat));

    arc.ArchiveInteger(&edict->r.svFlags);

    arc.ArchiveVec3(edict->r.mins);
    arc.ArchiveVec3(edict->r.maxs);
    arc.ArchiveInteger(&edict->r.contents);
    arc.ArchiveVec3(edict->r.absmin);
    arc.ArchiveVec3(edict->r.absmax);
    arc.ArchiveFloat(&edict->r.radius);
    if (!arc.Saving()) {
        edict->radius2 = edict->r.radius * edict->r.radius;
    }

    arc.ArchiveVec3(edict->r.currentOrigin);
    arc.ArchiveVec3(edict->r.currentAngles);

    arc.ArchiveInteger(&edict->r.ownerNum);
    ArchiveEnum(edict->solid, solid_t);
    arc.ArchiveFloat(&edict->freetime);
    arc.ArchiveFloat(&edict->spawntime);

    tempStr = str(edict->entname);
    arc.ArchiveString(&tempStr);
    strncpy(edict->entname, tempStr.c_str(), sizeof(edict->entname) - 1);

    arc.ArchiveInteger(&edict->clipmask);
    arc.ArchiveBoolean(&edict->r.bmodel);

    if (arc.Loading()) {
        gi.linkentity(edict);
    }

    arc.ArchiveInteger(&edict->r.lastNetTime);
}

/*
=========================================================================

model / sound configstring indexes

=========================================================================
*/

/*
=======================
G_FindConfigstringIndex
=======================
*/
int G_FindConfigstringIndex(char *name, int start, int max, qboolean create)
{
    int   i;
    char *s;

    if (!name || !name[0]) {
        return 0;
    }

    for (i = 1; i < max; i++) {
        s = gi.getConfigstring(start + i);
        if (!s || !s[0]) {
            break;
        }
        if (!strcmp(s, name)) {
            return i;
        }
    }

    if (!create) {
        return 0;
    }

    if (i == max) {
        gi.Error(ERR_DROP, "G_FindConfigstringIndex: overflow");
    }

    gi.setConfigstring(start + i, name);
    return i;
}

int G_ModelIndex(char *name)
{
    return G_FindConfigstringIndex(name, CS_MODELS, MAX_MODELS, true);
}

int G_SoundIndex(char *name)
{
    return G_FindConfigstringIndex(name, CS_SOUNDS, MAX_SOUNDS, true);
}

/*
===============
G_SetTrajectory

Sets the pos trajectory for a fixed position
===============
*/
void G_SetTrajectory(gentity_t *ent, vec3_t org)

{
    ent->s.pos.trTime = 0;
    VectorClear(ent->s.pos.trDelta);

    VectorCopy(org, ent->s.origin);
}

/*
===============
G_SetConstantLight

Sets the encoded constant light parameter for entities
===============
*/
void G_SetConstantLight(int *constantlight, float *red, float *green, float *blue, float *radius, int *lightStyle)

{
    int ir, ig, ib, iradius;

    if (!constantlight) {
        return;
    }

    ir      = (*constantlight) & 255;
    ig      = ((*constantlight) >> 8) & 255;
    ib      = ((*constantlight) >> 16) & 255;
    iradius = ((*constantlight) >> 24) & 255;

    if (red) {
        ir = *red * 255;
        if (ir > 255) {
            ir = 255;
        }
    }

    if (green) {
        ig = *green * 255;
        if (ig > 255) {
            ig = 255;
        }
    }

    if (blue) {
        ib = *blue * 255;
        if (ib > 255) {
            ib = 255;
        }
    }

    if (radius) {
        iradius = *radius / CONSTANTLIGHT_RADIUS_SCALE;
        if (iradius > 255) {
            iradius = 255;
        }
    }

    if (lightStyle) {
        ir = *lightStyle;
        if (ir > 255) {
            ir = 255;
        }
    }
    *constantlight = (ir) + (ig << 8) + (ib << 16) + (iradius << 24);
}

char *CanonicalTikiName(const char *szInName)
{
    static char filename[1024];

    if (*szInName && Q_stricmpn("models/", szInName, 7)) {
        sprintf(filename, "models/%s", szInName);
    } else {
        strcpy(filename, szInName);
    }

    gi.FS_CanonicalFilename(filename);
    return filename;
}

void G_ProcessCacheInitCommands(dtiki_t *tiki)
{
    dtikicmd_t *pcmd;

    if (tiki->a->num_server_initcmds) {
        int    i, j;
        Event *event;

        for (i = 0; i < tiki->a->num_server_initcmds; i++) {
            pcmd  = &tiki->a->server_initcmds[i];
            event = new Event(pcmd->args[0]);

            if (Director.GetFlags(event) & EV_CACHE) {
                for (j = 1; j < pcmd->num_args; j++) {
                    event->AddToken(pcmd->args[j]);
                }

                if (!Director.ProcessEvent(event)) {
                    Com_Printf(
                        "^~^~^ Entity::G_ProcessCacheInitCommands: Bad init server command '%s' in '%s'\n",
                        pcmd->args[0],
                        tiki->name
                    );
                }
            } else {
                delete event;
            }
        }
    }
}

void CacheResource(const char *stuff)

{
    AliasListNode_t *ret;
    qboolean         streamed = qfalse;
    char             filename[MAX_STRING_TOKENS];

    if (!stuff) {
        return;
    }

    if (!strchr(stuff, '.')) {
        // must be a global alias
        stuff = gi.GlobalAlias_FindRandom(stuff, &ret);
        if (!stuff) {
            if (gi.fsDebug->integer) {
                Com_Printf("alias not found\n");
            }

            return;
        }
        streamed = ret->streamed;
    }

    strcpy(filename, stuff);
    gi.FS_CanonicalFilename(filename);

    if (strstr(filename, ".wav")) {
        gi.soundindex(filename, streamed);
    } else if (strstr(filename, ".mp3")) {
        gi.soundindex(filename, streamed);
    } else if (strstr(filename, ".tik")) {
        dtiki_t *tiki;

        if (*stuff && strncmp("models/", stuff, 7)) {
            sprintf(filename, "models/%s", stuff);
        } else {
            strcpy(filename, stuff);
        }

        gi.FS_CanonicalFilename(filename);

        tiki = gi.TIKI_RegisterModel(filename);

        if (tiki) {
            G_ProcessCacheInitCommands(tiki);
        }
    } else if (strstr(filename, ".scr")) {
        Director.GetScript(filename);
    }
}

void ChangeMusic(const char *current, const char *fallback, qboolean force)
{
    int        j;
    gentity_t *other;

    if (current || fallback) {
        for (j = 0; j < game.maxclients; j++) {
            other = &g_entities[j];
            if (other->inuse && other->client) {
                Player *client;

                client = (Player *)other->entity;
                client->ChangeMusic(current, fallback, force);
            }
        }
        if (current && fallback) {
            gi.DPrintf("music set to %s with fallback %s\n", current, fallback);
        }
    }
}

void ChangeMusicVolume(float volume, float fade_time)
{
    int        j;
    gentity_t *other;

    for (j = 0; j < game.maxclients; j++) {
        other = &g_entities[j];
        if (other->inuse && other->client) {
            Player *client;

            client = (Player *)other->entity;
            client->ChangeMusicVolume(volume, fade_time);
        }
    }
    gi.DPrintf("music volume set to %.2f, fade time %.2f\n", volume, fade_time);
}

void RestoreMusicVolume(float fade_time)
{
    int        j;
    gentity_t *other;

    for (j = 0; j < game.maxclients; j++) {
        other = &g_entities[j];
        if (other->inuse && other->client) {
            Player *client;

            client = (Player *)other->entity;
            client->RestoreMusicVolume(fade_time);
        }
    }
}

void ChangeSoundtrack(const char *soundtrack)

{
    level.saved_soundtrack   = level.current_soundtrack;
    level.current_soundtrack = soundtrack;

    gi.setConfigstring(CS_SOUNDTRACK, soundtrack);
    gi.DPrintf("soundtrack switched to %s.\n", soundtrack);
}

void RestoreSoundtrack(void)

{
    if (level.saved_soundtrack.length()) {
        level.current_soundtrack = level.saved_soundtrack;
        level.saved_soundtrack   = "";
        gi.setConfigstring(CS_SOUNDTRACK, level.current_soundtrack.c_str());
        gi.DPrintf("soundtrack restored %s.\n", level.current_soundtrack.c_str());
    }
}

const char *G_AIEventStringFromType(int iType)
{
    switch (iType) {
    case AI_EVENT_WEAPON_FIRE:
        return "weapon_fire";
        break;
    case AI_EVENT_WEAPON_IMPACT:
        return "weapon_impact";
        break;
    case AI_EVENT_EXPLOSION:
        return "explosion";
        break;
    case AI_EVENT_AMERICAN_VOICE:
        return "american_voice";
        break;
    case AI_EVENT_GERMAN_VOICE:
        return "german_voice";
        break;
    case AI_EVENT_AMERICAN_URGENT:
        return "american_urgent";
        break;
    case AI_EVENT_GERMAN_URGENT:
        return "german_urgent";
        break;
    case AI_EVENT_MISC:
        return "misc";
        break;
    case AI_EVENT_MISC_LOUD:
        return "misc_loud";
        break;
    case AI_EVENT_FOOTSTEP:
        return "footstep";
        break;
    case AI_EVENT_GRENADE:
        return "grenade";
        break;
    default:
        return "????";
        break;
    }
}

int G_AIEventTypeFromString(const char *pszType)
{
    if (!Q_stricmp(pszType, "weapon_fire")) {
        return AI_EVENT_WEAPON_FIRE;
    } else if (!Q_stricmp(pszType, "weapon_impact")) {
        return AI_EVENT_WEAPON_IMPACT;
    } else if (!Q_stricmp(pszType, "explosion")) {
        return AI_EVENT_EXPLOSION;
    } else if (!Q_stricmp(pszType, "american_voice")) {
        return AI_EVENT_AMERICAN_VOICE;
    } else if (!Q_stricmp(pszType, "german_voice")) {
        return AI_EVENT_GERMAN_VOICE;
    } else if (!Q_stricmp(pszType, "american_urgent")) {
        return AI_EVENT_AMERICAN_URGENT;
    } else if (!Q_stricmp(pszType, "german_urgent")) {
        return AI_EVENT_GERMAN_URGENT;
    } else if (!Q_stricmp(pszType, "misc")) {
        return AI_EVENT_MISC;
    } else if (!Q_stricmp(pszType, "misc_loud")) {
        return AI_EVENT_MISC_LOUD;
    } else if (!Q_stricmp(pszType, "footstep")) {
        return AI_EVENT_FOOTSTEP;
    } else if (!Q_stricmp(pszType, "grenade")) {
        return AI_EVENT_GRENADE;
    } else {
        return AI_EVENT_NONE;
    }
}

float G_AIEventRadius(int iType)
{
    static float fRadius[] = {
        2048.0f, 384.0f, 4096.0f, 1024.0f, 1024.0f, 1536.0f, 1536.0f, 1500.0f, 2250.0f, 512.0f, 384.0f, 0, 0, 0, 0};

    if (iType <= AI_EVENT_GRENADE) {
        return fRadius[iType];
    } else {
        Com_Printf("G_AIEventRadius: invalid event type\n");
        return 1500.0f;
    }
}

void G_BroadcastAIEvent(Entity *originator, Vector origin, char *pszType)
{
    G_BroadcastAIEvent(originator, origin, G_AIEventTypeFromString(pszType), -1.0f);
}

void G_BroadcastAIEvent(Entity *originator, Vector origin, int iType, float radius)
{
    Sentient *ent;
    Vector    delta;
    str       name;
    float     r2;
    float     dist2;
    int       i;
    int       iNumSentients;
    int       iAreaNum;

    if (iType < AI_EVENT_FOOTSTEP) {
        ent = (Sentient *)G_GetEntity(0);

        if (ent && ent->m_bIsDisguised) {
            return;
        }
    }

    if (radius <= 0.0f) {
        radius = G_AIEventRadius(iType);
    }

    assert(originator);
    if (originator && !(originator->flags & FL_NOTARGET)) {
        r2            = Square(radius);
        iNumSentients = SentientList.NumObjects();
        for (i = 1; i <= iNumSentients; i++) {
            ent = SentientList.ObjectAt(i);
            if ((ent == originator) || ent->deadflag) {
                continue;
            }

            delta = origin - ent->centroid;

            // dot product returns length squared
            dist2 = Square(delta);

            if (originator) {
                iAreaNum = originator->edict->r.areanum;
            } else {
                iAreaNum = gi.AreaForPoint(origin);
            }

            if ((dist2 <= r2)
                && ((iAreaNum == ent->edict->r.areanum) || (gi.AreasConnected(iAreaNum, ent->edict->r.areanum))))

            {
                if (ent->IsSubclassOfActor()) {
                    Actor *act = (Actor *)ent;

                    if (!act->IgnoreSound(iType)) {
                        act->ReceiveAIEvent(origin, iType, originator, dist2, r2);
                    }
                } else if (ent->IsSubclassOfBot()) {
                    PlayerBot *bot = (PlayerBot *)ent;

                    bot->NoticeEvent(origin, iType, originator, dist2, r2);
                }
            }
        }

#if 0
		gi.DPrintf( "Broadcast event %s to %d entities\n", ev->getName(), count );
#endif
    }
}

void CloneEntity(Entity *dest, Entity *src)
{
    int i, num;

#if 0
	dest->setModel(src->model);
	// don't process our init commands
	//dest->CancelEventsOfType( EV_ProcessInitCommands );
	dest->setOrigin(src->origin);
	dest->setAngles(src->angles);
#endif
    dest->setScale(src->edict->s.scale);
    dest->setAlpha(src->edict->s.alpha);
    dest->health = src->health;
    // copy the surfaces
    memcpy(dest->edict->s.surfaces, src->edict->s.surfaces, sizeof(src->edict->s.surfaces));
    dest->edict->s.constantLight = src->edict->s.constantLight;
    //dest->edict->s.eFlags = src->edict->s.eFlags;
    dest->edict->s.renderfx = src->edict->s.renderfx;

    num = src->numchildren;
    for (i = 0; (i < MAX_MODEL_CHILDREN) && num; i++) {
        Entity *clone;
        Entity *child;

        // duplicate the children
        if (!src->children[i]) {
            continue;
        }
        child = G_GetEntity(src->children[i]);
        if (child) {
            clone = new Animate;
            CloneEntity(clone, child);
            clone->attach(dest->entnum, child->edict->s.tag_num);
        }
        num--;
    }
}

weaponhand_t WeaponHandNameToNum(str side)

{
    if (!side.length()) {
        gi.DPrintf("WeaponHandNameToNum : Weapon hand not specified\n");
        return WEAPON_ERROR;
    }

    if (!side.icmp("mainhand") || !side.icmp("main")) {
        return WEAPON_MAIN;
    } else if (!side.icmp("offhand") || !side.icmp("off")) {
        return WEAPON_OFFHAND;
    } else {
        return (weaponhand_t)atoi(side);
    }
}

const char *WeaponHandNumToName(weaponhand_t hand)

{
    switch (hand) {
    case WEAPON_MAIN:
        return "mainhand";
    case WEAPON_OFFHAND:
        return "offhand";
    default:
        return "Invalid Hand";
    }
}

firemode_t WeaponModeNameToNum(str mode)

{
    if (!mode.length()) {
        gi.DPrintf("WeaponModeNameToNum : Weapon mode not specified\n");
        return FIRE_ERROR;
    }

    if (!mode.icmp("primary")) {
        return FIRE_PRIMARY;
    }

    if (!mode.icmp("secondary")) {
        return FIRE_SECONDARY;
    } else {
        return (firemode_t)atoi(mode);
    }
}

int G_WeaponClassNameToNum(str name)
{
    int weaponindex = 0;

    if (!name.length()) {
        gi.DPrintf("WeaponClassNameToNum: Weapon class not specified\n");
        return 0;
    }

    if (!str::icmp(name, "pistol")) {
        weaponindex = WEAPON_CLASS_PISTOL;
    } else if (!str::icmp(name, "rifle")) {
        weaponindex = WEAPON_CLASS_RIFLE;
    } else if (!str::icmp(name, "smg")) {
        weaponindex = WEAPON_CLASS_SMG;
    } else if (!str::icmp(name, "mg")) {
        weaponindex = WEAPON_CLASS_MG;
    } else if (!str::icmp(name, "grenade")) {
        weaponindex = WEAPON_CLASS_GRENADE;
    } else if (!str::icmp(name, "heavy")) {
        weaponindex = WEAPON_CLASS_HEAVY;
    } else if (!str::icmp(name, "cannon")) {
        weaponindex = WEAPON_CLASS_CANNON;
    } else if (!str::icmp(name, "item")) {
        weaponindex = WEAPON_CLASS_ITEM;
    } else if (!str::icmp(name, "item1")) {
        weaponindex = WEAPON_CLASS_ITEM1;
    } else if (!str::icmp(name, "item2")) {
        weaponindex = WEAPON_CLASS_ITEM2;
    } else if (!str::icmp(name, "item3")) {
        weaponindex = WEAPON_CLASS_ITEM3;
    } else if (!str::icmp(name, "item4")) {
        weaponindex = WEAPON_CLASS_ITEM4;
    } else {
        gi.DPrintf("WeaponClassNameToNum: Unknown Weapon class %s\n", name.c_str());
        return 0;
    }

    return weaponindex;
}

str G_WeaponClassNumToName(int num)
{
    if (num & WEAPON_CLASS_PISTOL) {
        return "pistol";
    } else if (num & WEAPON_CLASS_RIFLE) {
        return "rifle";
    } else if (num & WEAPON_CLASS_SMG) {
        return "smg";
    } else if (num & WEAPON_CLASS_MG) {
        return "mg";
    } else if (num & WEAPON_CLASS_GRENADE) {
        return "grenade";
    } else if (num & WEAPON_CLASS_HEAVY) {
        return "heavy";
    } else if (num & WEAPON_CLASS_CANNON) {
        return "cannon";
    } else if (num & WEAPON_CLASS_ITEM) {
        return "item";
    } else if (num & WEAPON_CLASS_ITEM1) {
        return "item1";
    } else if (num & WEAPON_CLASS_ITEM2) {
        return "item2";
    } else if (num & WEAPON_CLASS_ITEM3) {
        return "item3";
    } else if (num & WEAPON_CLASS_ITEM4) {
        return "item4";
    } else {
        assert(0);
        return "";
    }
}

void G_DebugTargets(Entity *e, str from)

{
    gi.DPrintf("DEBUGTARGETS:%s ", from.c_str());

    if (e->TargetName() && strlen(e->TargetName())) {
        gi.DPrintf("Targetname=\"%s\"\n", e->TargetName().c_str());
    } else {
        gi.DPrintf("Targetname=\"None\"\n");
    }

    if (e->Target() && strlen(e->Target())) {
        gi.DPrintf("Target=\"%s\"\n", e->Target().c_str());
    } else {
        gi.DPrintf("Target=\"None\"\n");
    }
}

void G_DebugDamage(float damage, Entity *victim, Entity *attacker, Entity *inflictor)

{
    gi.DPrintf(
        "Victim:%s Attacker:%s Inflictor:%s Damage:%f\n",
        victim->getClassname(),
        attacker->getClassname(),
        inflictor->getClassname(),
        damage
    );
}

void G_FadeOut(float delaytime)
{
    // Fade the screen out
    level.m_fade_color      = Vector(0, 0, 0);
    level.m_fade_alpha      = 1.0f;
    level.m_fade_time       = delaytime;
    level.m_fade_time_start = delaytime;
    level.m_fade_type       = fadeout;
    level.m_fade_style      = alphablend;
}

void G_AutoFadeIn(void)

{
    level.m_fade_time_start = 1;
    level.m_fade_time       = 1;
    level.m_fade_color[0]   = 0;
    level.m_fade_color[1]   = 0;
    level.m_fade_color[2]   = 0;
    level.m_fade_alpha      = 1;
    level.m_fade_type       = fadein;
    level.m_fade_style      = alphablend;
}

void G_ClearFade(void)

{
    level.m_fade_time = -1;
    level.m_fade_type = fadein;
}

void G_FadeSound(float delaytime)
{
    float time;

    // Fade the screen out
    time = delaytime * 1000;
    gi.SendServerCommand(0, va("fadesound %0.2f", time));
}

void G_RestartLevelWithDelay(float delaytime)
{
    int i;

    if (level.died_already) {
        return;
    }

    level.died_already = true;

    // Restart the level soon
    for (i = 0; i < game.maxclients; i++) {
        if (g_entities[i].inuse) {
            if (g_entities[i].entity) {
                g_entities[i].entity->PostEvent(EV_Player_Respawn, delaytime);
            }
        }
    }
}

//
// restarts the game after delaytime
//
void G_PlayerDied(float delaytime)
{
    int i;

    if (level.died_already) {
        return;
    }

    level.died_already = true;

    // Restart the level soon

    for (i = 0; i < game.maxclients; i++) {
        if (g_entities[i].inuse) {
            if (g_entities[i].entity) {
                g_entities[i].entity->PostEvent(EV_Player_Respawn, delaytime);
            }
        }
    }

    G_FadeOut(delaytime);
    G_FadeSound(delaytime);
}

void G_MissionFailed(void)

{
    // Make the music system play the failure music for this level
    ChangeMusic("failure", "normal", true);

    G_PlayerDied(3);

    // tell the player they f'd up
    gi.centerprintf(&g_entities[0], "@textures/menu/mission.tga");

    level.mission_failed = true;
}

void G_StartCinematic(void)

{
    level.cinematic = true;
    gi.cvar_set("sv_cinematic", "1");
}

void G_StopCinematic(void)

{
    // clear out the skip thread
    level.cinematic = false;
    gi.cvar_set("sv_cinematic", "0");
}

void G_PrintToAllClients(const char *pszString, int iType)
{
    if (g_protocol >= protocol_e::PROTOCOL_MOHTA_MIN) {
        if (iType == 0) {
            gi.SendServerCommand(-1, "print \"" HUD_MESSAGE_YELLOW "%s\"", pszString);
        } else if (iType == 1) {
            gi.SendServerCommand(-1, "print \"" HUD_MESSAGE_WHITE "%s\"", pszString);
        } else if (iType == 2) {
            gi.SendServerCommand(-1, "print \"" HUD_MESSAGE_CHAT_WHITE "%s\"", pszString);
        }
    } else {
        if (iType == 0) {
            gi.SendServerCommand(-1, "print \"" HUD_MESSAGE_YELLOW "%s\n\"", pszString);
        } else {
            gi.SendServerCommand(-1, "print \"" HUD_MESSAGE_WHITE "%s\n\"", pszString);
        }
    }
}

void G_CenterPrintToAllClients(const char *pszString)
{
    gentity_t *ent;
    int        i;

    for (i = 0, ent = g_entities; i < game.maxclients; ent++, i++) {
        if (!ent->inuse || !ent->entity) {
            continue;
        }

        gi.centerprintf(ent, va("%s\n", pszString));
    }
}

void SanitizeName(const char *oldName, char *newName, size_t maxLen)
{
    size_t      i;
    size_t      j;
    size_t      len;
    const char *p;

    len = strlen(oldName);
    j   = 0;
    p   = oldName;

    for (i = 0; i < len && i < maxLen - 1; i++, p++) {
        if (i >= len - 1 && *p <= ' ') {
            newName[j++] = '?';
        } else {
            newName[j++] = *p;
        }
    }

    newName[j] = 0;
}

/*
=================
G_PrintDeathMessageEmulated

Emulate the client-side behavior of printdeathmessage.
So old clients can still have the string displayed.
=================
*/
const char *
G_PrintDeathMessageEmulated(const char *s1, const char *s2, char *attackerName, const char *victimName, char type)
{
    const char *result1, *result2;
    int         hudColor;

    result1 = NULL;
    result2 = NULL;

    if (type == tolower(type)) {
        hudColor = 4;
    } else {
        hudColor = 5;
    }

    if (*s1 != 'x') {
        result1 = gi.LV_ConvertString(s1);
    }
    if (*s2 != 'x') {
        result2 = gi.LV_ConvertString(s2);
    }
    if (tolower(type) == 's') {
        return va("%c%s %s\n", hudColor, victimName, result1);
    } else if (tolower(type) == 'p') {
        if (*s2 == 'x') {
            return va("%c%s %s %s\n", hudColor, victimName, result1, attackerName);
        } else {
            return va("%c%s %s %s%s\n", hudColor, victimName, result1, attackerName, result2);
        }
    } else if (tolower(type) == 'w') {
        return va("%c%s %s\n", hudColor, victimName, result1);
    } else {
        return va("%s", s1);
    }
}

void G_PrintDeathMessage(
    const char *s1, const char *s2, const char *attackerName, const char *victimName, Player *victim, const char *type
)
{
    gentity_t *ent;
    Player    *pPlayer;
    int        i;
    char       attackerNameSanitized[MAX_NAME_LENGTH];
    char       victimNameSanitized[MAX_NAME_LENGTH];

    SanitizeName(attackerName, attackerNameSanitized, MAX_NAME_LENGTH);
    SanitizeName(victimName, victimNameSanitized, MAX_NAME_LENGTH);

    if (g_protocol >= protocol_e::PROTOCOL_MOHTA_MIN) {
        for (i = 0, ent = g_entities; i < game.maxclients; i++, ent++) {
            if (!ent->inuse || !ent->entity) {
                continue;
            }

            pPlayer = static_cast<Player *>(ent->entity);

            if (pPlayer->GetTeam() == TEAM_ALLIES
                || pPlayer->GetTeam() == TEAM_AXIS && pPlayer->GetTeam() == victim->GetTeam()
                || pPlayer->GetTeam() != TEAM_ALLIES && pPlayer->GetTeam() != TEAM_AXIS
                       && victim->GetTeam() == TEAM_ALLIES) {
                gi.SendServerCommand(
                    ent - g_entities,
                    "printdeathmsg \"%s\"\"%s\"\"%s\"\"%s\" %c",
                    s1,
                    s2,
                    attackerNameSanitized,
                    victimNameSanitized,
                    *type
                );
            } else {
                gi.SendServerCommand(
                    ent - g_entities,
                    "printdeathmsg \"%s\"\"%s\"\"%s\"\"%s\" %c",
                    s1,
                    s2,
                    attackerNameSanitized,
                    victimNameSanitized,
                    toupper(*type)
                );
            }
        }
    } else {
        const char *string =
            G_PrintDeathMessageEmulated(s1, s2, attackerNameSanitized, victimNameSanitized, toupper(*type));

        // Fallback to the old version
        G_PrintDeathMessage_Old(string);
    }
}

void G_PrintDeathMessage_Old(const char *pszString)
{
    gentity_t *ent;
    int        i;

    for (i = 0, ent = g_entities; i < game.maxclients; i++, ent++) {
        if (!ent->inuse || !ent->entity) {
            continue;
        }

        gi.SendServerCommand(ent - g_entities, "print \"" HUD_MESSAGE_CHAT_RED "%s\"", pszString);
    }
}

char *G_TimeString(float fTime)
{
    float       fTmp;
    static char szTime[32];

    fTmp = fTime / 3600.0f;

    if (fTmp >= 1.0f) {
        Com_sprintf(
            szTime,
            sizeof(szTime),
            "%i:%02i:%02i",
            (int)(fTmp),
            (int)(fmod(fTime / 60.0f, 60.0f)),
            (int)(fmod(fTime, 60.0f))
        );
    } else {
        Com_sprintf(szTime, sizeof(szTime), "%i:%02i", (int)(fTime / 60.0f), (int)(fmod(fTime, 60.0f)));
    }

    return szTime;
}

void G_WarnPlayer(Player *player, const char *format, ...)
{
    char    buffer[4100];
    va_list va;

    va_start(va, format);
    vsprintf(buffer, format, va);
    va_end(va);

    gi.SendServerCommand(player->client->ps.clientNum, "print \"%s\"\n", buffer);
    gi.SendServerCommand(player->client->ps.clientNum, "print \"%c%s\"", HUD_MESSAGE_YELLOW, buffer);
}

int G_GetClientNumber(gentity_t *ent)
{
    return ent->client->ps.clientNum;
}

int G_GetClientNumber(Entity *entity)
{
    return 0;
}

SimpleArchivedEntity *G_FindArchivedClass(SimpleArchivedEntity *ent, const char *classname)
{
    SimpleArchivedEntity *arcent;
    int                   i;

    if (ent) {
        i = level.m_SimpleArchivedEntities.IndexOfObject((SimpleArchivedEntity *)ent) - 1;
    } else {
        i = level.m_SimpleArchivedEntities.NumObjects();
    }

    for (; i > 0; i--) {
        arcent = level.m_SimpleArchivedEntities.ObjectAt(i);

        if (!Q_stricmp(arcent->getClassID(), classname)) {
            return arcent;
        }
    }

    return NULL;
}

Entity *G_GetEntityByClient(int clientNum)
{
    gentity_t *ent = globals.gentities;
    if (clientNum < 0 || clientNum > globals.max_entities) {
        gi.DPrintf("G_GetEntity: %d out of valid range.", clientNum);
        return NULL;
    }

    for (int i = 0; i < globals.num_entities; i++, ent++) {
        if (ent->s.clientNum == clientNum) {
            break;
        }
    }

    return ent->entity;
}

int G_GetEntityIndex(int number)
{
    gentity_t *ent = globals.gentities;

    if (number < 0 || number > globals.max_entities) {
        return -1;
    }

    for (int i = 0; i < globals.num_entities; i++, ent++) {
        if (ent->s.number == number) {
            return i;
        }
    }

    return -1;
}

int G_GetEntityIndex(gentity_t *ent)
{
    return G_GetEntityIndex(ent->s.number);
}

gentity_t *G_GetGEntity(int ent_num)
{
    gentity_t *ent = &globals.gentities[ent_num];

    if (ent_num < 0 || ent_num > globals.max_entities) {
        gi.DPrintf("G_GetEntity: %d out of valid range.\n", ent_num);
        return NULL;
    }

    return ent;
}
