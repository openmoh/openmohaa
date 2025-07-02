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
    const Vector mins(MINS_X, MINS_Y, 0);
    const Vector maxs(MAXS_X, MAXS_Y, NavigationMapConfiguration::agentHeight);
    gentity_t   *edict;
    Vector       start, end;
    Vector       tstart, tend;
    Vector       top;
    trace_t      trace;

    for (edict = active_edicts.next; edict != &active_edicts; edict = edict->next) {
        if (!edict->entity) {
            continue;
        }

        if (edict->entity->isSubclassOf(FuncLadder)) {
            const FuncLadder *ladder    = static_cast<FuncLadder *>(edict->entity);
            const Vector&     facingDir = ladder->getFacingDir();

            offMeshNavigationPoint point;

            start = edict->entity->origin + Vector(0, 0, edict->entity->mins.z) - facingDir * 32;
            end   = edict->entity->origin + Vector(0, 0, edict->entity->maxs.z);

            // Trace to this:
            //
            // start
            // ↓  ┌┐
            // ●──│──● ← end
            //    │
            //    │
            tstart = end - facingDir * 32 - Vector(0, 0, STEPSIZE);
            tend   = end + facingDir * 64 - Vector(0, 0, STEPSIZE);
            trace  = G_Trace(tstart, mins, maxs, tend, NULL, MASK_PLAYERSOLID, qfalse, "ConnectLadders");

            // Trace to this:
            //
            //         end
            //         ↓
            //         ●
            //         │ ┌┐
            // start → ● │
            //           │
            //           │
            tstart = trace.endpos;
            tend   = trace.endpos + Vector(0, 0, STEPSIZE * 4);
            trace  = G_Trace(tstart, mins, maxs, tend, NULL, MASK_PLAYERSOLID, qfalse, "ConnectLadders");

            // Trace to this:
            //
            //  start
            //  ↓
            //  ●───● ← end
            //   ┌┐ 
            //   │
            //   │
            //   │
            // Connect to the end position
            top    = trace.endpos;
            tstart = top;
            tend   = top + facingDir * 48;
            trace  = G_Trace(tstart, mins, maxs, tend, NULL, MASK_PLAYERSOLID, qfalse, "ConnectLadders");

            // Trace to this:
            //
            //      start
            //      ↓
            //      ●
            //   ┌┐ │
            //   │  ● ← end
            //   │
            //   │
            // Connect to the end position
            tstart = trace.endpos;
            tend   = trace.endpos - Vector(0, 0, STEPSIZE * 6);
            trace  = G_Trace(tstart, mins, maxs, tend, NULL, MASK_PLAYERSOLID, qfalse, "ConnectLadders");

            // Trace to this:
            //
            //    ┌┐
            //    │ ●──● ← start
            //    │ ↑
            //    │ └ end
            //
            tstart = trace.endpos;
            tend   = top - facingDir * 32;
            tend.z = tstart.z - STEPSIZE;
            trace  = G_Trace(tstart, mins, maxs, tend, NULL, MASK_PLAYERSOLID, qfalse, "ConnectLadders");

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

bool NavigationMapExtension_JumpFall::AddPoint(
    Container<offMeshNavigationPoint>& points, const offMeshNavigationPoint& point
)
{
    if (!point.area) {
        return false;
    }

    points.AddObject(point);
    return true;
}

bool NavigationMapExtension_JumpFall::AreVertsValid(const vec3_t pos1, const vec3_t pos2) const
{
    vec3_t delta;

    VectorSub2D(pos2, pos1, delta);
    if (VectorLength2DSquared(delta) > Square(256)) {
        return false;
    }

    const float deltaHeight = pos2[2] - pos1[2];
    const float minHeight   = STEPSIZE / 2;
    if (deltaHeight >= -minHeight && deltaHeight <= minHeight) {
        // ignore steps
        return false;
    }

    return true;
}

void NavigationMapExtension_JumpFall::Handle(Container<offMeshNavigationPoint>& points, const rcPolyMesh *polyMesh)
{
    int             i, j;
    vec3_t         *vertpos;
    Vector          pos1, pos2;
    bool           *walkableVert;
    unsigned short *vertreg;
    unsigned char  *verttype;

    walkableVert = new bool[polyMesh->nverts];
    vertpos      = new vec3_t[polyMesh->nverts];
    vertreg      = new unsigned short[polyMesh->nverts];
    verttype     = new unsigned char[polyMesh->nverts];

    for (i = 0; i < polyMesh->nverts; i++) {
        walkableVert[i] = false;
        vertreg[i]      = 0;
        verttype[i]     = 0;

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

        for (j = i + 1; j < polyMesh->nverts; j++) {
            if (!walkableVert[j]) {
                continue;
            }

            if (vertreg[j] == vertreg[i]) {
                continue;
            }

            if (!AreVertsValid(vertpos[i], vertpos[j])) {
                continue;
            }
            pos1 = vertpos[i];
            pos2 = vertpos[j];

            offMeshNavigationPoint point;

            if (AddPoint(points, CanConnectStraightPoint(polyMesh, pos1, pos2))) {
                verttype[i] = verttype[j] = 1;
                continue;
            }
            if (AddPoint(points, CanConnectJumpPoint(polyMesh, pos1, pos2))) {
                verttype[i] = verttype[j] = 2;
                continue;
            }
            if (AddPoint(points, CanConnectFallPoint(polyMesh, pos1, pos2))) {
                verttype[i] = verttype[j] = 3;
                continue;
            }
        }
    }

    //
    // Add "jumping over obstacles" points.
    //  Those points are created if there are no jump points nor fall points
    //  that would allow the bot to get around the obstacle or exit an area
    //

    for (i = 0; i < polyMesh->nverts; i++) {
        if (!walkableVert[i]) {
            continue;
        }

        for (j = i + 1; j < polyMesh->nverts; j++) {
            if (!walkableVert[j]) {
                continue;
            }

            if (vertreg[j] == vertreg[i]) {
                continue;
            }

            if (verttype[i] && verttype[j]) {
                continue;
            }

            if (!AreVertsValid(vertpos[i], vertpos[j])) {
                continue;
            }
            pos1 = vertpos[i];
            pos2 = vertpos[j];

            offMeshNavigationPoint point;

            if (AddPoint(points, CanConnectJumpOverLedgePoint(polyMesh, pos1, pos2))) {
                verttype[i] = verttype[j] = 4;
                continue;
            }
        }
    }

    delete[] verttype;
    delete[] vertreg;
    delete[] vertpos;
    delete[] walkableVert;
}

void NavigationMapExtension_JumpFall::FixupPoint(vec3_t pos)
{
    trace_t trace;
    int     i;
    Vector  start, end;
    float   bestFrac = 9999;
    Vector  bestPos  = pos;

    const Vector mins(MINS_X, MINS_Y, 0);
    const Vector maxs(MAXS_X, MAXS_Y, STEPSIZE);

    trace = G_Trace(pos, mins, maxs, pos, NULL, MASK_PLAYERSOLID, qfalse, "FixupPoint");
    if (trace.startsolid) {
        //
        // Find the best fraction
        //

        for (i = 0; i < 4; i++) {
            float angle = ((2 * M_PI) * (float)i / (float)4);
            float dx    = cos(angle) * maxs.x;
            float dy    = sin(angle) * maxs.y;
            float dz    = STEPSIZE;

            start = Vector(pos[0] + dx, pos[1] + dy, pos[2] + dz);
            end   = pos;

            trace = G_Trace(start, mins, maxs, end, NULL, MASK_PLAYERSOLID, qfalse, "FixupPoint");
            if (trace.fraction < bestFrac && !trace.startsolid) {
                bestFrac = trace.fraction;
                bestPos  = trace.endpos;
            }
        }
    }

    //
    // Drop to floor
    //
    trace = G_Trace(
        bestPos + Vector(0, 0, STEPSIZE),
        mins,
        maxs,
        bestPos - Vector(0, 0, STEPSIZE * 2),
        NULL,
        MASK_PLAYERSOLID,
        qfalse,
        "CanConnectJumpPoint"
    );

    VectorCopy(trace.endpos, pos);
}

/*
============
NavigationMap::CanConnectFallPoint
============
*/
offMeshNavigationPoint
NavigationMapExtension_JumpFall::CanConnectFallPoint(const rcPolyMesh *polyMesh, const Vector& pos1, const Vector& pos2)
{
    const Vector           mins(MINS_X, MINS_Y, 0);
    const Vector           maxs(MAXS_X, MAXS_Y, NavigationMapConfiguration::agentHeight);
    const float            maxDistEdge = maxs.x * 3;
    Vector                 start, end;
    Vector                 tstart, tend;
    Vector                 delta;
    Vector                 dir;
    float                  fallheight;
    float                  dist;
    trace_t                trace;
    offMeshNavigationPoint point;

    if (pos1.z > pos2.z) {
        start = pos1;
        end   = pos2;
    } else {
        start = pos2;
        end   = pos1;
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

    if (!G_SightTrace(
            tstart, mins, maxs, tend, (Entity *)NULL, NULL, MASK_PLAYERSOLID, qfalse, "CanConnectFallPoint"
        )) {
        return {};
    }

    point.start         = start;
    point.end           = tend;
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
    const Vector           mins(MINS_X, MINS_Y, 0);
    const Vector           maxs(MAXS_X, MAXS_Y, NavigationMapConfiguration::agentHeight);
    Vector                 start, end;
    Vector                 tend;
    Vector                 delta;
    Vector                 fwdDir;
    float                  jumpheight;
    float                  length;
    Vector                 dir, right;
    Vector                 destGroundDir, destGroundUp;
    Vector                 planeNormal, wallForward;
    trace_t                trace;
    int                    i;
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

    //planeNormal = trace.plane.normal;
    //planeNormal.toAngles().AngleVectors(&destGroundUp, NULL, &destGroundDir);

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

    // Check for this situation:
    //
    // straight path
    //       |
    //       ↓
    // x───────────e
    trace = G_Trace(start, mins, maxs, end, NULL, MASK_PLAYERSOLID, qfalse, "CanConnectJumpPoint");
    if (!trace.allsolid && trace.fraction >= 0.999) {
        // Straight path
        return {};
    }

    // Calculate the right so multiple tries can be done
    planeNormal = trace.plane.normal;
    planeNormal.toAngles().AngleVectors(&wallForward, &right);
    right   = -right;
    right.z = 0;

    // Drop to floor
    //
    //        ┌ Go to e and drop to d
    //        ↓
    // s─────┐e┌─x
    //       └d┘
    trace = G_Trace(start, mins, maxs, start + dir * length, NULL, MASK_PLAYERSOLID, qfalse, "CanConnectJumpPoint");

    //
    //        ┌ Drop here
    //        ↓
    // ──────┐s┌─x
    //       └e┘
    trace = G_Trace(
        trace.endpos,
        mins,
        maxs,
        trace.endpos - Vector(0, 0, NavigationMapExtensionConfiguration::agentJumpHeight),
        NULL,
        MASK_PLAYERSOLID,
        qfalse,
        "CanConnectJumpPoint"
    );

    if (!trace.allsolid && trace.fraction > 0 && trace.fraction < 1) {
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

    for (i = 0; i < 4; i++) {
        Vector target;
        Vector highestPoint;

        //          ┌ get the highest point
        //          ↓
        //          h
        //          │
        // ──────┐ ┌x
        //       └─┘
        //
        highestPoint   = end;
        highestPoint.z = start.z + NavigationMapExtensionConfiguration::agentJumpHeight;

        trace = G_Trace(end, mins, maxs, highestPoint, NULL, MASK_PLAYERSOLID, qfalse, "CanConnectJumpPoint");

        target = start;
        target += wallForward * (i * 4);
        target += right * (i * 4);
        target.z = trace.endpos[2];

        //  Make sure the player can jump at this point
        //
        //        ┌ must be able to jump
        //        ↓
        //        e
        // ──────┐│┌x
        //       └s┘
        //
        trace = G_Trace(start, mins, maxs, target, NULL, MASK_PLAYERSOLID, qfalse, "CanConnectJumpPoint");
        if (trace.allsolid || trace.fraction == 0) {
            continue;
        }

        tend = trace.endpos;

        //   Check if the point is reachable while jumping
        //
        //           ┌ must be able to reach this point
        //        s  ↓
        // ──────┐│┌─x
        //       └┴┘
        //

        if (G_SightTrace(tend, mins, maxs, end, (Entity *)NULL, NULL, MASK_PLAYERSOLID, qfalse, "CanConnectJumpPoint")
            && G_SightTrace(
                end, mins, maxs, tend, (Entity *)NULL, NULL, MASK_PLAYERSOLID, qfalse, "CanConnectJumpPoint"
            )) {
            delta      = end - start;
            jumpheight = delta.z;
            if (jumpheight > NavigationMapExtensionConfiguration::agentJumpHeight) {
                return {};
            }

            if (delta.lengthSquared() > Square(128)) {
                return {};
            }
            break;
        }
    }

    if (i >= 4) {
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
NavigationMap::CanConnectJumpOverLedgePoint

The following action can be performed:

    Jump over that ledge
    ↓
   ┌─┐
 s─┘ └─e
============
*/
offMeshNavigationPoint NavigationMapExtension_JumpFall::CanConnectJumpOverLedgePoint(
    const rcPolyMesh *polyMesh, const Vector& pos1, const Vector& pos2
)
{
    const Vector           mins(MINS_X, MINS_Y, 0);
    const Vector           maxs(MAXS_X, MAXS_Y, NavigationMapConfiguration::agentHeight);
    Vector                 start, end;
    Vector                 startStep;
    Vector                 startInAir;
    Vector                 startToEnd;
    Vector                 delta;
    Vector                 dir;
    float                  length, dist;
    float                  fallheight;
    float                  heightDiff;
    float                  jumpHeight;
    int                    i;
    trace_t                trace;
    offMeshNavigationPoint point;

    if (pos1.z > pos2.z) {
        start = pos1;
        end   = pos2;
    } else {
        start = pos2;
        end   = pos1;
    }

    heightDiff = fabs(start.z - end.z);
    if (heightDiff > 600) {
        return {};
    }

    delta = end - start;

    dir    = delta;
    dir.z  = 0;
    length = dir.normalize();

    // Check for this situation:
    //
    // straight path
    //       |
    //       ↓
    // x───────────e
    if (G_SightTrace(
            start, mins, maxs, end, (Entity *)NULL, NULL, MASK_PLAYERSOLID, qfalse, "CanConnectJumpOverLedgePoint"
        )) {
        // Straight path
        return {};
    }

    // Go here
    // ↓
    // e
    // │  ┌─┐
    // s──┘ └─t
    trace = G_Trace(
        start,
        mins,
        maxs,
        start + Vector(0, 0, NavigationMapExtensionConfiguration::agentJumpHeight),
        NULL,
        MASK_PLAYERSOLID,
        qfalse,
        "CanConnectJumpOverLedgePoint"
    );

    if (trace.allsolid) {
        return {};
    }

    jumpHeight = trace.endpos[2] - start[2];
    startInAir = trace.endpos;

    for (i = 0; i < STEPSIZE; i += STEPSIZE / 2.0) {
        startStep = start + Vector(0, 0, i);

        //
        //   Go here
        //   ↓
        //    ┌─┐
        // s─e┘ └─t
        trace = G_Trace(
            startStep,
            mins,
            maxs,
            startStep + dir * length,
            NULL,
            MASK_PLAYERSOLID,
            qfalse,
            "CanConnectJumpOverLedgePoint"
        );
        if (trace.fraction >= 0.999) {
            return {};
        }
    }

    delta = trace.endpos - startStep;
    if (delta.lengthXYSquared() > Square(32)) {
        // Too much distance
        return {};
    }

    for (i = 0; i < NavigationMapExtensionConfiguration::agentJumpHeight && i < jumpHeight; i += 10) {
        //        Go here
        //        ↓
        // s──────e
        //    ┌─┐
        // ───┘ └─t
        trace = G_Trace(
            startInAir - Vector(0, 0, i),
            mins,
            maxs,
            startInAir + dir * length - Vector(0, 0, i),
            NULL,
            MASK_PLAYERSOLID,
            qfalse,
            "CanConnectJumpOverLedgePoint"
        );

        if (trace.fraction >= 0.999) {
            break;
        }
    }

    if (i >= NavigationMapExtensionConfiguration::agentJumpHeight) {
        return {};
    }

    startToEnd = trace.endpos;
    fallheight = startInAir.z - end.z;

    if (fallheight > 600) {
        // This would cause too much damage
        return {};
    }

    delta = startInAir - end;
    dist  = delta.lengthXY();
    if (dist > fallheight) {
        // Too far from the start
        return {};
    }

    //        Go here
    //        ↓
    //        s
    //    ┌─┐ │
    // ───┘ └─e
    if (!G_SightTrace(
            startToEnd, mins, maxs, end, (Entity *)NULL, NULL, MASK_PLAYERSOLID, qfalse, "CanConnectJumpOverLedgePoint"
        )) {
        return {};
    }

    point.start = start;
    point.end   = end;
    if (fallheight > NavigationMapExtensionConfiguration::agentJumpHeight) {
        point.bidirectional = false;
    } else {
        point.bidirectional = true;
    }

    point.radius = NavigationMapConfiguration::agentRadius;
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
NavigationMap::CanConnectStraightPoint
============
*/
offMeshNavigationPoint NavigationMapExtension_JumpFall::CanConnectStraightPoint(
    const rcPolyMesh *polyMesh, const Vector& pos1, const Vector& pos2
)
{
    const Vector           mins(MINS_X, MINS_Y, 0);
    const Vector           maxs(MAXS_X, MAXS_Y, NavigationMapConfiguration::agentHeight);
    Vector                 start, end;
    Vector                 delta;
    Vector                 fwdDir;
    float                  jumpheight;
    Vector                 dir;
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

    if (!G_SightTrace(
            start, mins, maxs, end, NULL, (Entity *)NULL, MASK_PLAYERSOLID, qfalse, "CanConnectStraightPoint"
        )) {
        // Not straight path
        return {};
    }

    if (!G_SightTrace(
            end, mins, maxs, start, NULL, (Entity *)NULL, MASK_PLAYERSOLID, qfalse, "CanConnectStraightPoint"
        )) {
        return {};
    }

    dir = delta;
    dir.normalize();

    for (i = 0; i < ARRAY_LEN(offsets); i++) {
        if (!G_SightTrace(
                start + dir * offsets[i],
                vec_zero,
                vec_zero,
                end,
                NULL,
                (Entity *)NULL,
                MASK_PLAYERSOLID,
                qfalse,
                "CanConnectStraightPoint"
            )) {
            return {};
        }

        if (!G_SightTrace(
                end + dir * offsets[i],
                vec_zero,
                vec_zero,
                start,
                NULL,
                (Entity *)NULL,
                MASK_PLAYERSOLID,
                qfalse,
                "CanConnectStraightPoint"
            )) {
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
