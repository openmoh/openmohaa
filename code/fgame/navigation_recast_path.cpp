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
 * @file navigation_recast_path.cpp
 * @brief Modern navigation system using Recast and Detour
 *
 * Detour is used for path-finding using a mesh that was automatically generated when parsing the BSP file.
 */

#include "navigation_recast_path.h"
#include "navigation_recast_load.h"
#include "navigation_recast_helpers.h"
#include "level.h"

#include "DetourPathCorridor.h"
#include "DetourCommon.h"
#include "entity.h"
#include "bg_local.h"

#define MAX_NPOLYS 256

RecastPathMaster pathMaster;

static const vec3_t DETOUR_EXTENT = {(MAXS_X - MINS_X) / 2, (MAXS_Z - MINS_Z) / 2, (MAXS_Y - MINS_Y) / 2};

struct DetourData {
public:
    dtPathCorridor corridor;
    vec3_t         corners[4];
    unsigned char  cornerFlags[4];
    dtPolyRef      cornerPolys[4];
    int            ncorners;
};

RecastPather::RecastPather()
    : lastCheckTime(0)
    , moving(false)
{
    detourData = new DetourData();
    detourData->corridor.init(256);
}

RecastPather::~RecastPather()
{
    if (detourData) {
        delete detourData;
        detourData = NULL;
    }
}

void RecastPather::FindPath(const Vector& start, const Vector& end, const PathSearchParameter& parameters)
{
    Vector               recastStart, recastEnd;
    dtPolyRef            startRef, endRef;
    vec3_t               startPt, endPt;
    const dtQueryFilter *filter = navigationMap.GetQueryFilter();

    lastorg = start;
    ResetPosition(start);

    ConvertGameToRecastCoord(start, recastStart);
    ConvertGameToRecastCoord(end, recastEnd);

    endRef   = 0;
    startRef = detourData->corridor.getFirstPoly();
    dtVcopy(startPt, detourData->corridor.getPos());
    navigationMap.GetNavMeshQuery()->findNearestPoly(recastEnd, DETOUR_EXTENT, filter, &endRef, endPt);

    if (!startRef || !endRef) {
        return;
    }

    dtPolyRef polys[MAX_NPOLYS];
    int       nPolys = 0;
    navigationMap.GetNavMeshQuery()->findPath(startRef, endRef, startPt, endPt, filter, polys, &nPolys, ARRAY_LEN(polys) - 1);

    if (nPolys) {
        vec3_t   closestPos;
        dtStatus status;

        if (polys[nPolys - 1] != endRef) {
            status = navigationMap.GetNavMeshQuery()->closestPointOnPoly(polys[nPolys - 1], endPt, closestPos, 0);
            if (dtStatusFailed(status)) {
                VectorCopy(endPt, closestPos);
            }
        } else {
            VectorCopy(endPt, closestPos);
        }

        moving = true;
        detourData->corridor.setCorridor(closestPos, polys, nPolys);
    }
}

void RecastPather::FindPathNear(
    const Vector& start, const Vector& end, float radius, const PathSearchParameter& parameters
)
{
    Vector               recastStart, recastEnd;
    dtPolyRef            startRef, endRef;
    vec3_t               startPt, endPt;
    const dtQueryFilter *filter = navigationMap.GetQueryFilter();

    lastorg = start;
    ResetPosition(start);

    ConvertGameToRecastCoord(start, recastStart);
    ConvertGameToRecastCoord(end, recastEnd);

    endRef   = 0;
    startRef = detourData->corridor.getFirstPoly();
    dtVcopy(startPt, detourData->corridor.getPos());
    navigationMap.GetNavMeshQuery()->findNearestPoly(recastEnd, DETOUR_EXTENT, filter, &endRef, endPt);

    if (!startRef || !endRef) {
        return;
    }

    // Now find a point within this radius
    if (navigationMap.GetNavMeshQuery()->findRandomPointAroundCircle(
            endRef, endPt, radius, filter, &G_Random, &endRef, endPt
        ) != DT_SUCCESS
        || !endRef) {
        return;
    }

    dtPolyRef polys[MAX_NPOLYS];
    int       nPolys = 0;
    navigationMap.GetNavMeshQuery()->findPath(startRef, endRef, startPt, endPt, filter, polys, &nPolys, ARRAY_LEN(polys) - 1);

    if (nPolys) {
        vec3_t   closestPos;
        dtStatus status;

        if (polys[nPolys - 1] != endRef) {
            status = navigationMap.GetNavMeshQuery()->closestPointOnPoly(polys[nPolys - 1], endPt, closestPos, 0);
            if (dtStatusFailed(status)) {
                VectorCopy(endPt, closestPos);
            }
        } else {
            VectorCopy(endPt, closestPos);
        }

        moving = true;
        detourData->corridor.setCorridor(closestPos, polys, nPolys);
    }
}

void RecastPather::FindPathAway(
    const Vector&              start,
    const Vector&              avoid,
    const Vector&              preferredDir,
    float                      radius,
    const PathSearchParameter& parameters
)
{
    Vector               recastStart, recastAvoid, recastEnd;
    Vector               dirNormalized;
    dtPolyRef            startRef, endRef;
    vec3_t               startPt, endPt;
    const dtQueryFilter *filter = navigationMap.GetQueryFilter();
    float                startAngle;
    float                startPitch;
    int                  i, j;

    lastorg = start;
    ResetPosition(start);

    startAngle = DEG2RAD(preferredDir.toYaw());
    startPitch = DEG2RAD(preferredDir.toPitch());

    dirNormalized = preferredDir;
    dirNormalized.normalize();

    ConvertGameToRecastCoord(start, recastStart);
    ConvertGameToRecastCoord(avoid, recastAvoid);
    ConvertGameToRecastCoord(avoid + dirNormalized * radius, recastEnd);

    startRef = detourData->corridor.getFirstPoly();
    dtVcopy(startPt, detourData->corridor.getPos());

    if (!startRef) {
        return;
    }

    for (i = 0; i < 36; i++) {
        float angle = startAngle + ((2 * M_PI) * (float)i / (float)36);
        float dx    = cos(angle) * radius;
        float dz    = sin(angle) * radius;

        for (j = 0; j < 4; j++) {
            float  pitch = startPitch + (M_PI * (float)j / 4);
            float  dy    = sin(pitch) * radius;
            Vector point(recastAvoid[0] + dx, recastAvoid[1] + dy, recastAvoid[2] + dz);

            if (navigationMap.GetNavMeshQuery()->findNearestPoly(point, DETOUR_EXTENT, filter, &endRef, endPt)
                    == DT_SUCCESS
                && endRef) {
                dtPolyRef polys[MAX_NPOLYS];
                int       nPolys = 0;
                navigationMap.GetNavMeshQuery()->findPath(
                    startRef, endRef, startPt, endPt, filter, polys, &nPolys, ARRAY_LEN(polys) - 1
                );

                if (nPolys) {
                    vec3_t   closestPos;
                    dtStatus status;

                    if (polys[nPolys - 1] != endRef) {
                        status = navigationMap.GetNavMeshQuery()->closestPointOnPoly(
                            polys[nPolys - 1], endPt, closestPos, 0
                        );
                        if (dtStatusFailed(status)) {
                            VectorCopy(endPt, closestPos);
                        }
                    } else {
                        VectorCopy(endPt, closestPos);
                    }

                    moving = true;
                    detourData->corridor.setCorridor(closestPos, polys, nPolys);
                }

                return;
            }
        }
    }
}

bool RecastPather::TestPath(const Vector& start, const Vector& end, const PathSearchParameter& parameters)
{
    Vector               recastStart, recastEnd;
    const dtQueryFilter *filter = navigationMap.GetQueryFilter();
    dtStatus             status;

    ConvertGameToRecastCoord(start, recastStart);
    ConvertGameToRecastCoord(end, recastEnd);

    dtPolyRef nearestStartRef, nearestEndRef;
    vec3_t    nearestStartPt, nearestEndPt;
    navigationMap.GetNavMeshQuery()->findNearestPoly(
        recastStart, DETOUR_EXTENT, filter, &nearestStartRef, nearestStartPt
    );
    navigationMap.GetNavMeshQuery()->findNearestPoly(recastEnd, DETOUR_EXTENT, filter, &nearestEndRef, nearestEndPt);

    dtPolyRef polys[MAX_NPOLYS];
    int       nPolys;
    status = navigationMap.GetNavMeshQuery()->findPath(
        nearestStartRef, nearestEndRef, nearestStartPt, nearestEndPt, filter, polys, &nPolys, 256
    );

    if (!(status & DT_SUCCESS)) {
        // Invalid path
        return false;
    }

    return true;
}

static bool overOffmeshConnection(
    const vec3_t pos, const unsigned char *cornerFlags, const vec3_t cornerVerts, const float radius, const int ncorners
)
{
    if (!ncorners) {
        return false;
    }

    const bool offMeshConnection = (cornerFlags[ncorners - 1] & DT_STRAIGHTPATH_OFFMESH_CONNECTION) ? true : false;
    if (offMeshConnection) {
        const float distSq = dtVdist2DSqr(&cornerVerts[(ncorners - 1) * 3], pos);
        if (distSq < radius * radius) {
            return true;
        }
    }

    return false;
}

void RecastPather::UpdatePos(const Vector& origin)
{
    const dtQueryFilter *filter = navigationMap.GetQueryFilter();
    const Vector velocity = (origin - lastorg) * (1.0 / level.frametime);
    const float distSqr = Q_min(Square(64), velocity.lengthSquared());

    lastorg = origin;
    Vector recastOrigin;

    ConvertGameToRecastCoord(origin, recastOrigin);

    if (traversingOffMeshLink) {
        Vector agentPos;
        Vector delta;

        ConvertRecastToGameCoord(detourData->corridor.getPos(), agentPos);
        delta = agentPos - origin;
        if (delta.lengthSquared() < Square(24) + distSqr) {
            // traversed
            traversingOffMeshLink = false;
        }
    } else if (level.inttime >= lastCheckTime + 2000) {
        vec3_t delta;
        VectorSubtract(recastOrigin, detourData->corridor.getPos(), delta);

        if (VectorLengthSquared(delta) > Square(64)) {
            dtPolyRef startRef, endRef;
            vec3_t    startPt, endPt;

            //
            // Get the target position
            //
            endRef = detourData->corridor.getLastPoly();
            VectorCopy(detourData->corridor.getTarget(), endPt);

            ResetPosition(origin);

            startRef = 0;
            navigationMap.GetNavMeshQuery()->findNearestPoly(recastOrigin, DETOUR_EXTENT, filter, &startRef, startPt);

            if (startRef) {
                dtPolyRef polys[MAX_NPOLYS];
                int       nPolys = 0;
                navigationMap.GetNavMeshQuery()->findPath(
                    startRef, endRef, startPt, endPt, filter, polys, &nPolys, ARRAY_LEN(polys) - 1
                );

                if (nPolys) {
                    moving = true;
                    detourData->corridor.setCorridor(endPt, polys, nPolys);
                }
            }
        }

        lastCheckTime = level.inttime;
    } else {
        detourData->corridor.movePosition(recastOrigin, navigationMap.GetNavMeshQuery(), filter);
        ConvertRecastToGameCoord(detourData->corridor.getPos(), lastValidOrg);

        detourData->ncorners = detourData->corridor.findCorners(
            (float *)detourData->corners,
            detourData->cornerFlags,
            detourData->cornerPolys,
            4,
            navigationMap.GetNavMeshQuery(),
            filter
        );
        if (detourData->ncorners) {
            dtPolyRef refs[2];
            vec3_t    startOffPos, endOffPos;

            if (overOffmeshConnection(
                    recastOrigin, detourData->cornerFlags, (const vec_t *)detourData->corners, 16, detourData->ncorners
                )
                && detourData->corridor.moveOverOffmeshConnection(
                    detourData->cornerPolys[detourData->ncorners - 1],
                    refs,
                    startOffPos,
                    endOffPos,
                    navigationMap.GetNavMeshQuery()
                )) {
                ConvertRecastToGameCoord(detourData->corridor.getPos(), currentNodePos);

                traversingOffMeshLink = true;
            } else {
                ConvertRecastToGameCoord(detourData->corners[0], currentNodePos);
            }
        } else {
            ConvertRecastToGameCoord(detourData->corridor.getPos(), currentNodePos);
        }
    }
}

void RecastPather::Clear()
{
    ResetPosition(lastorg);
}

PathNav RecastPather::GetNode(unsigned int index) const
{
    PathNav               nav;
    Vector                target;
    Vector                delta;
    Vector                agentPos;
    const dtPathCorridor *inCorridor;
    const dtPolyRef      *path;
    int                   npath;

    inCorridor = &detourData->corridor;

    path  = inCorridor->getPath();
    npath = inCorridor->getPathCount();
    if (npath <= 0) {
        return {};
    }

    if (index + 1 == npath) {
        // Just return the target pos
        ConvertRecastToGameCoord(inCorridor->getTarget(), nav.origin);
        //ConvertRecastToGameCoord(agent->npos, agentPos);
        agentPos = lastorg;

        nav.dir[0] = nav.origin[0] - agentPos[0];
        nav.dir[1] = nav.origin[1] - agentPos[1];
        nav.dist   = VectorLength2D(nav.dir);
        VectorNormalize2D(nav.dir);

        return nav;
    }

    const dtMeshTile *tile;
    const dtPoly     *poly;
    if (navigationMap.GetNavMesh()->getTileAndPolyByRef(path[index], &tile, &poly) != DT_SUCCESS) {
        return {};
    }

    const unsigned int tileId = (unsigned int)(poly - tile->polys);

    if (poly->getType() == DT_POLYTYPE_OFFMESH_CONNECTION) {
        dtOffMeshConnection *con = &tile->offMeshCons[tileId - tile->header->offMeshBase];
        Vector               start, end;

        ConvertRecastToGameCoord(&con->pos[0], start);
        ConvertRecastToGameCoord(&con->pos[3], end);

        nav.origin = start;
        nav.dir[0] = end[0] - start[0];
        nav.dir[1] = end[1] - start[1];
        VectorNormalize2D(nav.dir);
        nav.dist = delta.length();
    } else {
        const dtPolyDetail *dm = &tile->detailMeshes[tileId];
        Vector              middle[1];

        for (int i = 0; i < 1; i++) {
            for (int j = 0; j < dm->triCount; ++j) {
                const unsigned char *t = &tile->detailTris[(dm->triBase + j) * 4];
                for (int k = 0; k < 3; ++k) {
                    if (t[k] < poly->vertCount) {
                        middle[i] += &tile->verts[poly->verts[t[k]] * 3];
                    } else {
                        middle[i] += &tile->detailVerts[(dm->vertBase + t[k] - poly->vertCount) * 3];
                    }
                }
            }

            middle[i] /= dm->triCount * 3;
        }

        ConvertRecastToGameCoord(middle[0], nav.origin);
        ConvertRecastToGameCoord(middle[1], target);

        delta      = target - nav.origin;
        nav.dir[0] = delta[0];
        nav.dir[1] = delta[1];
        VectorNormalize2D(nav.dir);
        nav.dist = delta.length();
    }

    return nav;
}

int RecastPather::GetNodeCount() const
{
    if (!moving) {
        return 0;
    }

    return detourData->corridor.getPathCount();
}

Vector RecastPather::GetCurrentDelta() const
{
    Vector delta;

    delta = currentNodePos - lastorg;

    return delta;
}

Vector RecastPather::GetCurrentDirection() const
{
    Vector delta;

    delta = currentNodePos - lastorg;
    delta.normalize();

    return delta;
}

Vector RecastPather::GetDestination() const
{
    Vector dest;

    ConvertRecastToGameCoord(detourData->corridor.getTarget(), dest);

    return dest;
}

bool RecastPather::HasReachedGoal(const Vector& origin) const
{
    Vector target;

    const dtPolyRef lastPoly = detourData->corridor.getLastPoly();
    ConvertRecastToGameCoord(detourData->corridor.getTarget(), target);

    if (fabs(origin[0] - target[0]) < 16.0f && fabs(origin[1] - target[1]) < 16.0f) {
        return true;
    }

    return false;
}

bool RecastPather::IsQuerying() const
{
    return false;
}

void RecastPather::ResetPosition(const Vector& origin)
{
    const dtQueryFilter *filter = navigationMap.GetQueryFilter();
    vec3_t               agentPos;

    traversingOffMeshLink = false;
    lastCheckTime         = level.inttime;

    moving = false;

    ConvertGameToRecastCoord(origin, agentPos);

    // Find nearest position on navmesh and place the agent there.
    dtPolyRef ref = 0;
    float     nearest[3];
    dtStatus  status;

    status = navigationMap.GetNavMeshQuery()->findNearestPoly(agentPos, DETOUR_EXTENT, filter, &ref, nearest);

    if (dtStatusFailed(status) || !ref) {
        // Use the last valid position instead
        ConvertGameToRecastCoord(lastValidOrg, agentPos);
        status = navigationMap.GetNavMeshQuery()->findNearestPoly(agentPos, DETOUR_EXTENT, filter, &ref, nearest);
    }

    if (dtStatusSucceed(status) && ref) {
        detourData->corridor.reset(ref, nearest);
        ConvertRecastToGameCoord(agentPos, lastValidOrg);
    } else {
        detourData->corridor.reset(0, agentPos);
    }
}

void RecastPathMaster::PostLoadNavigation(const NavigationMap& map) {}

void RecastPathMaster::ClearNavigation() {}

void RecastPathMaster::Update() {}
