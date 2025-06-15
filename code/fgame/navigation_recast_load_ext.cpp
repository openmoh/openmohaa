/*
===========================================================================
Copyright (C) 2025 the OpenMoHAA team

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

/**
 * @file navigation_recast_load_ext.cpp
 * @brief Extension for recast-based navigation
 * 
 */

#include "g_local.h"
#include "navigation_recast_load_ext.h"
#include "navigation_recast_config.h"
#include "navigation_recast_config_ext.h"
#include "navigation_recast_helpers.h"
#include "Recast.h"
#include "entity.h"
#include "misc.h"

CLASS_DECLARATION(Class, INavigationMapExtension, NULL) {
    {NULL, NULL}
};

CLASS_DECLARATION(INavigationMapExtension, NavigationMapExtension_Ladders, NULL) {
    {NULL, NULL}
};

void NavigationMapExtension_Ladders::Handle(Container<offMeshNavigationPoint>& points, const rcPolyMesh *polyMesh)
{
    gentity_t *edict;
    Vector     start, end;
    Vector     tend;
    trace_t    trace;

    for (edict = active_edicts.next; edict != &active_edicts; edict = edict->next) {
        if (!edict->entity) {
            continue;
        }

        if (edict->entity->isSubclassOf(FuncLadder)) {
            const FuncLadder *ladder    = static_cast<FuncLadder *>(edict->entity);
            const Vector&     facingDir = ladder->getFacingDir();

            offMeshNavigationPoint point;

            start = edict->entity->origin + Vector(0, 0, edict->entity->mins.z) - facingDir * 32;
            end   = edict->entity->origin + Vector(0, 0, edict->entity->maxs.z + 16) + facingDir * 16;

            tend = end;
            tend.z -= STEPSIZE;
            trace = G_Trace(end, vec_zero, vec_zero, tend, NULL, MASK_PLAYERSOLID, qfalse, "ConnectLadders");

            point.start         = start;
            point.end           = trace.endpos;
            point.bidirectional = true;
            point.radius        = NavigationMapConfiguration::agentRadius;
            point.area          = RECAST_AREA_LADDER;
            point.flags         = RECAST_POLYFLAG_WALKABLE;

            points.AddObject(point);
        }
    }
}

Container<ExtensionArea> NavigationMapExtension_Ladders::GetSupportedAreas() const
{
    Container<ExtensionArea> list;

    // Ladders are good alternative but they shouldn't be used all the time
    list.AddObject(ExtensionArea(RECAST_AREA_LADDER, 10.0));

    return list;
}

CLASS_DECLARATION(INavigationMapExtension, NavigationMapExtension_JumpFall, NULL) {
    {NULL, NULL}
};

void NavigationMapExtension_JumpFall::Handle(Container<offMeshNavigationPoint>& points, const rcPolyMesh *polyMesh)
{
    int                   i, j;
    vec3_t                delta;
    vec3_t               *vertpos;
    Vector                pos1, pos2;
    const unsigned short *v1, *v2;
    bool                 *walkableVert;
    unsigned short       *vertreg;

    walkableVert = new bool[polyMesh->nverts];
    vertpos      = new vec3_t[polyMesh->nverts];
    vertreg      = new unsigned short[polyMesh->nverts];

    for (i = 0; i < polyMesh->nverts; i++) {
        walkableVert[i] = false;
        vertreg[i]      = 0;

        GetPolyMeshVertPosition(polyMesh, i, vertpos[i]);
        FixupPoint(vertpos[i]);
    }

    for (i = 0; i < polyMesh->npolys; i++) {
        const unsigned short *poly = &polyMesh->polys[i * polyMesh->nvp * 2];

        if (polyMesh->areas[i] != RC_WALKABLE_AREA) {
            continue;
        }

        for (j = 0; j < polyMesh->nvp; j++) {
            if (poly[j] == RC_MESH_NULL_IDX) {
                break;
            }

            assert(poly[j] < polyMesh->nverts);
            walkableVert[poly[j]] = true;
            vertreg[poly[j]]      = polyMesh->regs[i];
        }
    }

    for (i = 0; i < polyMesh->nverts; i++) {
        if (!walkableVert[i]) {
            continue;
        }

        v1   = &polyMesh->verts[i * 3];
        pos1 = vertpos[i];

        for (j = i + 1; j < polyMesh->nverts; j++) {
            if (!walkableVert[j]) {
                continue;
            }

            if (vertreg[j] == vertreg[i]) {
                continue;
            }

            v2   = &polyMesh->verts[j * 3];
            pos2 = vertpos[j];

            VectorSub2D(pos2, pos1, delta);
            if (VectorLength2DSquared(delta) > Square(256)) {
                continue;
            }

            const float deltaHeight = pos2.z - pos1.z;
            const float minHeight   = STEPSIZE / 2;
            if (deltaHeight >= -minHeight && deltaHeight <= minHeight) {
                // ignore steps
                continue;
            }

            offMeshNavigationPoint point;

            point = CanConnectFallPoint(polyMesh, pos1, pos2);
            if (point.area) {
                points.AddObject(point);
            }

            point = CanConnectJumpPoint(polyMesh, pos1, pos2);
            if (point.area) {
                points.AddObject(point);
            }

            point = CanConnectStraightPoint(polyMesh, pos1, pos2);
            if (point.area) {
                points.AddObject(point);
            }
        }
    }

    delete[] vertreg;
    delete[] vertpos;
    delete[] walkableVert;
}

void NavigationMapExtension_JumpFall::FixupPoint(vec3_t pos)
{
    const Vector mins(-15, -15, 0);
    const Vector maxs(15, 15, NavigationMapConfiguration::agentHeight);
    trace_t      trace;

    trace = G_Trace(pos, mins, maxs, pos, NULL, MASK_PLAYERSOLID, qfalse, "CanConnectFallPoint");
    if (trace.startsolid) {
        int i;

        for (i = 0; i < 4; i++) {
            float  angle = ((2 * M_PI) * (float)i / (float)4);
            float  dx    = cos(angle) * maxs.x;
            float  dy    = sin(angle) * maxs.y;
            Vector point(pos[0] + dx, pos[1] + dy, pos[2] + STEPSIZE);

            trace = G_Trace(point, mins, maxs, pos, NULL, MASK_PLAYERSOLID, qfalse, "CanConnectFallPoint");
            if (!trace.startsolid) {
                VectorCopy(trace.endpos, pos);
                break;
            }
        }
    }
}

/*
============
NavigationMap::CanConnectFallPoint
============
*/
offMeshNavigationPoint
NavigationMapExtension_JumpFall::CanConnectFallPoint(const rcPolyMesh *polyMesh, const Vector& pos1, const Vector& pos2)
{
    const Vector           mins(-15, -15, 0);
    const Vector           maxs(15, 15, NavigationMapConfiguration::agentHeight);
    const float            maxDistEdge = maxs.x * 3;
    Vector                 start, end;
    Vector                 tstart, tend;
    Vector                 delta;
    Vector                 dir;
    float                  fallheight;
    float                  dist;
    trace_t                trace;
    offMeshNavigationPoint point;

    if (pos1.z <= pos2.z) {
        start = pos2;
        end   = pos1;
    } else {
        start = pos1;
        end   = pos2;
    }

    delta      = end - start;
    fallheight = -delta.z;

    if (fallheight > 600) {
        // This would cause too much damage
        return {};
    }

    dist = delta.lengthXY();
    if (dist > fallheight) {
        // Too far from the start
        return {};
    }

    dir   = delta;
    dir.z = 0;
    dir.normalize();

    // s---x <-- Check if the path from s to x is reachable
    //     |
    //     |
    tstart = start;
    tend   = tstart + dir * Q_min(dist, maxDistEdge);

    trace = G_Trace(tstart, mins, maxs, tend, NULL, MASK_PLAYERSOLID, qfalse, "CanConnectFallPoint");
    if (trace.allsolid || trace.startsolid) {
        return {};
    }
    /*
    else if (trace.startsolid) {
        tend = tstart;
        tstart.z += STEPSIZE;

        trace = G_Trace(tstart, mins, maxs, tend, NULL, MASK_PLAYERSOLID, qtrue, "CanConnectFallPoint");
        if (trace.allsolid || trace.startsolid) {
            // Still in a solid
            return {};
        }

        // Apply the new position
        tstart = start = trace.endpos;
        tend           = tstart + dir * Q_min(dist, maxDistEdge);

        trace = G_Trace(tstart, mins, maxs, tend, NULL, MASK_PLAYERSOLID, qtrue, "CanConnectFallPoint");
        if (trace.allsolid || trace.startsolid) {
            return {};
        }
    }
    */

    // ----s
    //     |
    //     |
    //     x <-- Check if the path from s to x is reachable
    tstart = trace.endpos;
    tend   = end;

    trace = G_Trace(tstart, mins, maxs, tend, NULL, MASK_PLAYERSOLID, qfalse, "CanConnectFallPoint");
    if (trace.fraction < 0.999) {
        return {};
    }

    point.start         = start;
    point.end           = trace.endpos;
    point.bidirectional = false;
    point.radius        = NavigationMapConfiguration::agentRadius;
    if (fallheight <= NavigationMapExtensionConfiguration::smallFallHeight) {
        point.area = RECAST_AREA_FALL;
    } else if (fallheight <= NavigationMapExtensionConfiguration::mediumFallHeight) {
        point.area = RECAST_AREA_MEDIUM_FALL;
    } else {
        point.area = RECAST_AREA_HIGH_FALL;
    }

    point.flags = RECAST_POLYFLAG_WALKABLE;

    return point;
}

/*
============
NavigationMap::CanConnectJumpPoint
============
*/
offMeshNavigationPoint
NavigationMapExtension_JumpFall::CanConnectJumpPoint(const rcPolyMesh *polyMesh, const Vector& pos1, const Vector& pos2)
{
    const Vector           mins(-15, -15, 0);
    const Vector           maxs(15, 15, NavigationMapConfiguration::agentHeight);
    Vector                 start, end;
    Vector                 tend;
    Vector                 delta;
    Vector                 fwdDir;
    float                  jumpheight;
    float                  length;
    Vector                 dir;
    trace_t                trace;
    offMeshNavigationPoint point;

    if (pos1.z > pos2.z) {
        start = pos2;
        end   = pos1;
    } else {
        start = pos1;
        end   = pos2;
    }

    jumpheight = end.z - start.z;
    if (jumpheight > NavigationMapExtensionConfiguration::agentJumpHeight) {
        return {};
    }

    // Drop to floor
    trace = G_Trace(
        start + Vector(0, 0, STEPSIZE),
        mins,
        maxs,
        start - Vector(0, 0, STEPSIZE * 2),
        NULL,
        MASK_PLAYERSOLID,
        qtrue,
        "CanConnectJumpPoint"
    );
    start = trace.endpos;
    trace = G_Trace(
        end + Vector(0, 0, STEPSIZE),
        mins,
        maxs,
        end - Vector(0, 0, STEPSIZE * 2),
        NULL,
        MASK_PLAYERSOLID,
        qtrue,
        "CanConnectJumpPoint"
    );
    end = trace.endpos;

    delta      = end - start;
    jumpheight = delta.z;
    if (jumpheight > NavigationMapExtensionConfiguration::agentJumpHeight) {
        return {};
    }

    if (delta.lengthSquared() > Square(128)) {
        return {};
    }

    dir    = delta;
    dir.z  = 0;
    length = dir.normalize();
    fwdDir = dir * 32;

    trace = G_Trace(start, mins, maxs, end, NULL, MASK_PLAYERSOLID, qfalse, "CanConnectJumpPoint");
    if (!trace.allsolid && trace.fraction >= 0.999) {
        // Straight path
        return {};
    }

    // Drop to floor
    trace = G_Trace(start, mins, maxs, start + dir * length, NULL, MASK_PLAYERSOLID, qfalse, "CanConnectJumpPoint");

    trace = G_Trace(
        trace.endpos,
        mins,
        maxs,
        trace.endpos - Vector(0, 0, STEPSIZE * 2),
        NULL,
        MASK_PLAYERSOLID,
        qtrue,
        "CanConnectJumpPoint"
    );
    if (!trace.startsolid && trace.fraction > 0 && trace.fraction < 1) {
        start      = trace.endpos;
        delta      = end - start;
        jumpheight = delta.z;
        if (jumpheight > NavigationMapExtensionConfiguration::agentJumpHeight) {
            return {};
        }

        if (delta.lengthSquared() > Square(128)) {
            return {};
        }
    }

    trace = G_Trace(
        start,
        mins,
        maxs,
        start + Vector(0, 0, NavigationMapExtensionConfiguration::agentJumpHeight),
        NULL,
        MASK_PLAYERSOLID,
        qtrue,
        "CanConnectJumpPoint"
    );
    if (trace.allsolid) {
        return {};
    }

    tend = trace.endpos;

    trace = G_Trace(tend, mins, maxs, end, NULL, MASK_PLAYERSOLID, qfalse, "CanConnectJumpPoint");
    if (trace.fraction < 0.999) {
        return {};
    }
    point.start         = start;
    point.end           = end;
    point.bidirectional = true;
    point.radius        = NavigationMapConfiguration::agentRadius;
    point.area          = RECAST_AREA_JUMP;
    point.flags         = RECAST_POLYFLAG_WALKABLE;

    return point;
}

/*
============
NavigationMap::CanConnectStraightPoint
============
*/
offMeshNavigationPoint NavigationMapExtension_JumpFall::CanConnectStraightPoint(
    const rcPolyMesh *polyMesh, const Vector& pos1, const Vector& pos2
)
{
    const Vector           mins(-15, -15, 0);
    const Vector           maxs(15, 15, NavigationMapConfiguration::agentHeight);
    Vector                 start, end;
    Vector                 delta;
    Vector                 fwdDir;
    float                  jumpheight;
    Vector                 dir;
    trace_t                trace;
    int                    i;
    static const float     offsets[2] = {-16, 16};
    offMeshNavigationPoint point;

    if (pos1.z > pos2.z) {
        start = pos2;
        end   = pos1;
    } else {
        start = pos1;
        end   = pos2;
    }

    delta      = end - start;
    jumpheight = delta.z;
    if (jumpheight > STEPSIZE) {
        return {};
    }

    trace = G_Trace(start, mins, maxs, end, NULL, MASK_PLAYERSOLID, qfalse, "CanConnectStraightPoint");
    if (trace.allsolid || trace.startsolid || trace.fraction < 0.999 || trace.plane.normal[2] >= MIN_WALK_NORMAL) {
        // Straight path
        return {};
    }

    dir = delta;
    dir.normalize();

    for (i = 0; i < ARRAY_LEN(offsets); i++) {
        trace = G_Trace(
            start + dir * offsets[i], vec_zero, vec_zero, end, NULL, MASK_PLAYERSOLID, qfalse, "CanConnectStraightPoint"
        );
        if (trace.fraction < 0.999 || trace.startsolid) {
            return {};
        }

        trace = G_Trace(
            end + dir * offsets[i], vec_zero, vec_zero, start, NULL, MASK_PLAYERSOLID, qfalse, "CanConnectStraightPoint"
        );
        if (trace.fraction < 0.999 || trace.startsolid) {
            return {};
        }
    }

    point.start         = start;
    point.end           = end;
    point.bidirectional = true;
    point.radius        = NavigationMapConfiguration::agentRadius;
    point.area          = RECAST_AREA_STRAIGHT;
    point.flags         = RECAST_POLYFLAG_WALKABLE;

    return point;
}

Container<ExtensionArea> NavigationMapExtension_JumpFall::GetSupportedAreas() const
{
    Container<ExtensionArea> list;

    // Only jump when necessary
    list.AddObject(ExtensionArea(RECAST_AREA_JUMP, 10.0));

    // Small falls can be used as a shortcut
    list.AddObject(ExtensionArea(RECAST_AREA_FALL, 5.0));
    list.AddObject(ExtensionArea(RECAST_AREA_MEDIUM_FALL, 10.0));
    // Take high fall as a last resort, when no alternative is available
    list.AddObject(ExtensionArea(RECAST_AREA_HIGH_FALL, 20.0));

    list.AddObject(ExtensionArea(RECAST_AREA_STRAIGHT, 100.0));

    return list;
}
