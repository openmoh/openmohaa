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

#include "DetourCrowd.h"
#include "DetourCommon.h"
#include "entity.h"
#include "bg_local.h"

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
#if USE_DETOUR_AGENT
    , hasAgent(false)
#endif
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
    Vector        recastStart, recastEnd;
    Vector        half(16, 16, 16);
    dtPolyRef     startRef, endRef;
    vec3_t        startPt, endPt;
    dtQueryFilter filter;

    lastorg = start;
    ResetAgent(start);

#if USE_DETOUR_AGENT
    ConvertGameToRecastCoord(end, recastEnd);

    if (navigationMap.GetNavMeshQuery()->findNearestPoly(recastEnd, half, &filter, &endRef, endPt) != DT_SUCCESS
        || !endRef) {
        pathMaster.agentManager.GetCrowd()->resetMoveTarget(navAgentId);
        return;
    }

    pathMaster.agentManager.GetCrowd()->requestMoveTarget(navAgentId, endRef, endPt);
#else

    ConvertGameToRecastCoord(start, recastStart);
    ConvertGameToRecastCoord(end, recastEnd);

    endRef   = 0;
    startRef = detourData->corridor.getFirstPoly();
    dtVcopy(startPt, detourData->corridor.getPos());
    navigationMap.GetNavMeshQuery()->findNearestPoly(recastEnd, half, &filter, &endRef, endPt);

    if (!startRef || !endRef) {
        return;
    }

    dtPolyRef polys[256];
    int       nPolys = 0;
    navigationMap.GetNavMeshQuery()->findPath(startRef, endRef, startPt, endPt, &filter, polys, &nPolys, 256);

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
#endif
}

void RecastPather::FindPathNear(
    const Vector& start, const Vector& end, float radius, const PathSearchParameter& parameters
)
{
    Vector        recastStart, recastEnd;
    Vector        half(16, 16, 16);
    dtPolyRef     startRef, endRef;
    vec3_t        startPt, endPt;
    dtQueryFilter filter;

    lastorg = start;
    ResetAgent(start);

#if USE_DETOUR_AGENT
    ConvertGameToRecastCoord(end, recastEnd);

    if (navigationMap.GetNavMeshQuery()->findNearestPoly(recastEnd, half, &filter, &endRef, endPt) != DT_SUCCESS
        || !endRef) {
        pathMaster.agentManager.GetCrowd()->resetMoveTarget(navAgentId);
        return;
    }

    // Now find a point within this radius
    if (navigationMap.GetNavMeshQuery()->findRandomPointAroundCircle(
            endRef, endPt, radius, &filter, &G_Random, &endRef, endPt
        ) != DT_SUCCESS
        || !endRef) {
        pathMaster.agentManager.GetCrowd()->resetMoveTarget(navAgentId);
        return;
    }
#else
    ConvertGameToRecastCoord(start, recastStart);
    ConvertGameToRecastCoord(end, recastEnd);

    endRef   = 0;
    startRef = detourData->corridor.getFirstPoly();
    dtVcopy(startPt, detourData->corridor.getPos());
    navigationMap.GetNavMeshQuery()->findNearestPoly(recastEnd, half, &filter, &endRef, endPt);

    if (!startRef || !endRef) {
        return;
    }

    // Now find a point within this radius
    if (navigationMap.GetNavMeshQuery()->findRandomPointAroundCircle(
            endRef, endPt, radius, &filter, &G_Random, &endRef, endPt
        ) != DT_SUCCESS
        || !endRef) {
        return;
    }

    dtPolyRef polys[256];
    int       nPolys = 0;
    navigationMap.GetNavMeshQuery()->findPath(startRef, endRef, startPt, endPt, &filter, polys, &nPolys, 256);

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
#endif
}

void RecastPather::FindPathAway(
    const Vector&              start,
    const Vector&              avoid,
    const Vector&              preferredDir,
    float                      radius,
    const PathSearchParameter& parameters
)
{
    Vector        recastStart, recastAvoid, recastEnd;
    Vector        dirNormalized;
    Vector        half(16, 16, 16);
    dtPolyRef     startRef, endRef;
    vec3_t        startPt, endPt;
    dtQueryFilter filter;
    float         startAngle;
    float         startPitch;
    int           i, j;

    lastorg = start;
    ResetAgent(start);

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

            if (navigationMap.GetNavMeshQuery()->findNearestPoly(point, half, &filter, &endRef, endPt) == DT_SUCCESS
                && endRef) {
#if USE_DETOUR_AGENT
                pathMaster.agentManager.GetCrowd()->requestMoveTarget(navAgentId, endRef, endPt);
#else

                dtPolyRef polys[256];
                int       nPolys = 0;
                navigationMap.GetNavMeshQuery()->findPath(
                    startRef, endRef, startPt, endPt, &filter, polys, &nPolys, 256
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
#endif
                return;
            }
        }
    }

#if USE_DETOUR_AGENT
    pathMaster.agentManager.GetCrowd()->resetMoveTarget(navAgentId);
#endif
}

bool RecastPather::TestPath(const Vector& start, const Vector& end, const PathSearchParameter& parameters)
{
    Vector        recastStart, recastEnd;
    dtQueryFilter filter;
    vec3_t        half = {64, 64, 64};
    dtStatus      status;

    ConvertGameToRecastCoord(start, recastStart);
    ConvertGameToRecastCoord(end, recastEnd);

    dtPolyRef nearestStartRef, nearestEndRef;
    vec3_t    nearestStartPt, nearestEndPt;
    navigationMap.GetNavMeshQuery()->findNearestPoly(recastStart, half, &filter, &nearestStartRef, nearestStartPt);
    navigationMap.GetNavMeshQuery()->findNearestPoly(recastEnd, half, &filter, &nearestEndRef, nearestEndPt);

    dtPolyRef polys[256];
    int       nPolys;
    status = navigationMap.GetNavMeshQuery()->findPath(
        nearestStartRef, nearestEndRef, nearestStartPt, nearestEndPt, &filter, polys, &nPolys, 256
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
        vec2_t delta;
        VectorSub2D(&cornerVerts[(ncorners - 1) * 3], pos, delta);
        const float distSq = VectorLength2DSquared(delta);
        if (distSq < radius * radius) {
            return true;
        }
    }

    return false;
}

void RecastPather::UpdatePos(const Vector& origin)
{
    lastorg = origin;

#if USE_DETOUR_AGENT
    dtCrowd      *crowdManager = pathMaster.agentManager.GetCrowd();
    dtCrowdAgent *agent;

    if (!hasAgent) {
        return;
    }

    agent = crowdManager->getEditableAgent(navAgentId);

    agent->params.maxSpeed        = speed;
    agent->params.maxAcceleration = agent->params.maxSpeed * 3.0;

    switch (agent->state) {
    case DT_CROWDAGENT_STATE_WALKING:
        if (!traversingOffMeshLink) {
            ConvertRecastToGameCoord(agent->npos, lastValidOrg);
        }
        break;
    case DT_CROWDAGENT_STATE_OFFMESH:
        traversingOffMeshLink = true;
        // Don't do anything else while it's traversing
        return;
    default:
        break;
    }

    if (traversingOffMeshLink) {
        Vector agentPos;
        Vector delta;

        // Clear the velocity and acceleration so it doesn't move
        agent->params.maxAcceleration = 0;
        agent->vel[0] = agent->vel[1] = agent->vel[2];
        agent->nvel[0] = agent->nvel[1] = agent->nvel[2];

        ConvertRecastToGameCoord(agent->corridor.getPos(), agentPos);
        delta = agentPos - lastorg;
        if (delta.lengthSquared() < Square(16)) {
            // traversed
            traversingOffMeshLink = false;
        }
    } else if (level.inttime >= lastCheckTime + 2000) {
        ConvertGameToRecastCoord(origin, agent->npos);
        if (agent->targetState != DT_CROWDAGENT_TARGET_REQUESTING
            && agent->targetState != DT_CROWDAGENT_TARGET_WAITING_FOR_QUEUE
            && agent->targetState != DT_CROWDAGENT_TARGET_WAITING_FOR_PATH) {
            agent->corridor.movePosition(agent->npos, navigationMap.GetNavMeshQuery(), crowdManager->getFilter(0));

            Vector delta = Vector(agent->npos) - Vector(agent->corridor.getPos());
            if (delta.lengthSquared() > Square(64)) {
                agent->targetState = DT_CROWDAGENT_TARGET_REQUESTING;
                agent->corridor.reset(0, agent->npos);
            }
        }

        lastCheckTime = level.inttime;
        return;
    }

    //Vector agentPos;
    //ConvertRecastToGameCoord(agent->npos, agentPos);
    //g_entities[2].entity->setOrigin(agentPos);
#else
    Vector recastOrigin;

    ConvertGameToRecastCoord(origin, recastOrigin);

    dtQueryFilter filter;
    if (traversingOffMeshLink) {
        Vector agentPos;
        Vector delta;

        ConvertRecastToGameCoord(detourData->corridor.getPos(), agentPos);
        delta = agentPos - lastorg;
        if (delta.lengthSquared() < Square(16)) {
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

            ResetAgent(origin);

            startRef = 0;
            navigationMap.GetNavMeshQuery()->findNearestPoly(recastOrigin, DETOUR_EXTENT, &filter, &startRef, startPt);

            if (startRef) {
                dtPolyRef polys[256];
                int       nPolys = 0;
                navigationMap.GetNavMeshQuery()->findPath(
                    startRef, endRef, startPt, endPt, &filter, polys, &nPolys, 256
                );

                if (nPolys) {
                    moving = true;
                    detourData->corridor.setCorridor(endPt, polys, nPolys);
                }
            }
        }

        lastCheckTime = level.inttime;
    } else {
        detourData->corridor.movePosition(recastOrigin, navigationMap.GetNavMeshQuery(), &filter);
        ConvertRecastToGameCoord(detourData->corridor.getPos(), lastValidOrg);

        detourData->ncorners = detourData->corridor.findCorners(
            (float *)detourData->corners,
            detourData->cornerFlags,
            detourData->cornerPolys,
            4,
            navigationMap.GetNavMeshQuery(),
            &filter
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
#endif
}

void RecastPather::Clear()
{
    ResetAgent(lastorg);

#if USE_DETOUR_AGENT
    dtCrowd *crowdManager;

    if (!hasAgent) {
        return;
    }

    crowdManager = pathMaster.agentManager.GetCrowd();
    crowdManager->resetMoveTarget(navAgentId);
#endif
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

#if USE_DETOUR_AGENT
    const dtCrowdAgent *agent;

    if (!hasAgent) {
        return {};
    }

    agent      = pathMaster.agentManager.GetCrowd()->getAgent(navAgentId);
    inCorridor = &agent->corridor;
#else
    inCorridor = &detourData->corridor;
#endif

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
#if USE_DETOUR_AGENT
    if (!hasAgent) {
        return 0;
    }

    const dtCrowdAgent *agent = pathMaster.agentManager.GetCrowd()->getAgent(navAgentId);
    if (agent->targetState == DT_CROWDAGENT_TARGET_NONE || agent->targetState == DT_CROWDAGENT_TARGET_VELOCITY) {
        // Not moving
        return 0;
    }

    if (agent->state == DT_CROWDAGENT_STATE_INVALID) {
        // Invalid path (probably not found)
        return 0;
    }

    return agent->corridor.getPathCount();
#else
    if (!moving) {
        return 0;
    }

    return detourData->corridor.getPathCount();
#endif
}

Vector RecastPather::GetCurrentDelta() const
{
    Vector delta;

#if USE_DETOUR_AGENT
    dtCrowd *crowdManager = pathMaster.agentManager.GetCrowd();

    if (!hasAgent) {
        return {};
    }

    const dtCrowdAgent *agent = pathMaster.agentManager.GetCrowd()->getAgent(navAgentId);
    if (agent->targetState == DT_CROWDAGENT_TARGET_NONE || agent->targetState == DT_CROWDAGENT_TARGET_VELOCITY) {
        return {};
    }

    if (agent->state == DT_CROWDAGENT_STATE_INVALID) {
        return {};
    }

    ConvertRecastToGameCoord(agent->npos, delta);
    delta = delta - lastorg;
#else
    delta = currentNodePos - lastorg;
#endif

    return delta;
}

Vector RecastPather::GetCurrentDirection() const
{
    Vector delta;

#if USE_DETOUR_AGENT
    dtCrowd *crowdManager = pathMaster.agentManager.GetCrowd();

    if (!hasAgent) {
        return {};
    }

    const dtCrowdAgent *agent = pathMaster.agentManager.GetCrowd()->getAgent(navAgentId);
    if (agent->targetState == DT_CROWDAGENT_TARGET_NONE || agent->targetState == DT_CROWDAGENT_TARGET_VELOCITY) {
        return {};
    }

    if (agent->state == DT_CROWDAGENT_STATE_INVALID) {
        return {};
    }

    if (!traversingOffMeshLink) {
        ConvertRecastToGameCoord(agent->vel, delta);
    } else {
        ConvertRecastToGameCoord(agent->npos, delta);
        delta = delta - lastorg;
    }
#else
    delta = currentNodePos - lastorg;
#endif
    delta.normalize();

    return delta;
}

Vector RecastPather::GetDestination() const
{
    Vector dest;

#if USE_DETOUR_AGENT
    if (!hasAgent) {
        return {};
    }

    const dtCrowdAgent *agent = pathMaster.agentManager.GetCrowd()->getAgent(navAgentId);
    if (agent->targetState == DT_CROWDAGENT_TARGET_NONE || agent->targetState == DT_CROWDAGENT_TARGET_VELOCITY) {
        return {};
    }

    if (agent->state == DT_CROWDAGENT_STATE_INVALID) {
        return {};
    }

    if (agent->targetState == DT_CROWDAGENT_TARGET_VALID) {
        ConvertRecastToGameCoord(agent->corridor.getTarget(), dest);
        return dest;
    }

    ConvertRecastToGameCoord(agent->targetPos, dest);
#else
    ConvertRecastToGameCoord(detourData->corridor.getTarget(), dest);
#endif

    return dest;
}

bool RecastPather::HasReachedGoal(const Vector& origin) const
{
#if USE_DETOUR_AGENT
    const dtCrowdAgent *agent;

    if (!hasAgent) {
        return true;
    }

    agent = pathMaster.agentManager.GetCrowd()->getAgent(navAgentId);

    /*
    const dtPolyRef lastPoly = agent->corridor.getLastPoly();
    ConvertRecastToGameCoord(agent->corridor.getTarget(), target);
    
    if (fabs(origin[0] - m_path->point[0]) < 16.0f && fabs(origin[1] - m_path->point[1]) < 16.0f) {
        return true;
    }
    */
#endif

    return false;
}

bool RecastPather::IsQuerying() const
{
#if USE_DETOUR_AGENT
    const dtCrowdAgent *agent = pathMaster.agentManager.GetCrowd()->getAgent(navAgentId);

    if (agent->state == DT_CROWDAGENT_STATE_INVALID) {
        return false;
    }

    switch (agent->targetState) {
    case DT_CROWDAGENT_TARGET_NONE:
    case DT_CROWDAGENT_TARGET_VALID:
    case DT_CROWDAGENT_TARGET_VELOCITY:
        return false;
    default:
        return true;
    }
#else
    return false;
#endif
}

void RecastPather::ResetAgent(const Vector& origin)
{
    traversingOffMeshLink = false;
    lastCheckTime         = level.inttime;

#if USE_DETOUR_AGENT
    dtCrowd *crowdManager = pathMaster.agentManager.GetCrowd();
    vec3_t   rcOrigin;

    if (hasAgent) {
        dtCrowdAgent *agent = crowdManager->getEditableAgent(navAgentId);

        pathMaster.agentManager.GetCrowd()->resetMoveTarget(navAgentId);
        ConvertGameToRecastCoord(origin, agent->npos);

        // Find nearest position on navmesh and place the agent there.
        dtPolyRef ref = 0;
        float     nearest[3];
        dtStatus  status;

        status = navigationMap.GetNavMeshQuery()->findNearestPoly(
            agent->npos, crowdManager->getQueryHalfExtents(), crowdManager->getFilter(0), &ref, nearest
        );

        if (dtStatusFailed(status) || !ref) {
            // Use the last valid position instead
            ConvertGameToRecastCoord(lastValidOrg, agent->npos);
            status = navigationMap.GetNavMeshQuery()->findNearestPoly(
                agent->npos, crowdManager->getQueryHalfExtents(), crowdManager->getFilter(0), &ref, nearest
            );
        }

        if (dtStatusSucceed(status) && ref) {
            agent->state = DT_CROWDAGENT_STATE_WALKING;
            agent->corridor.reset(ref, nearest);
            detourData->corridor.reset(ref, nearest);
            ConvertRecastToGameCoord(agent->npos, lastValidOrg);
        } else {
            agent->corridor.reset(0, agent->npos);
            detourData->corridor.reset(0, agent->npos);
        }

        return;
    }

    ConvertGameToRecastCoord(origin, rcOrigin);

    dtCrowdAgentParams ap {0};
    ap.radius                = pathMaster.agentManager.GetAgentRadius();
    ap.height                = NavigationMap::agentHeight;
    ap.maxSpeed              = sv_runspeed->integer * sv_dmspeedmult->integer;
    ap.maxAcceleration       = ap.maxSpeed * 3.0;
    ap.collisionQueryRange   = ap.radius * 12.0f;
    ap.pathOptimizationRange = ap.radius * 64.0f;
    ap.separationWeight      = 0.0;
    ap.updateFlags           = DT_CROWD_OPTIMIZE_TOPO;

    navAgentId = crowdManager->addAgent(rcOrigin, &ap);
    hasAgent   = true;
#else
    vec3_t agentPos;

    moving = false;

    ConvertGameToRecastCoord(origin, agentPos);

    // Find nearest position on navmesh and place the agent there.
    dtPolyRef     ref = 0;
    float         nearest[3];
    dtStatus      status;
    dtQueryFilter filter;

    status = navigationMap.GetNavMeshQuery()->findNearestPoly(agentPos, DETOUR_EXTENT, &filter, &ref, nearest);

    if (dtStatusFailed(status) || !ref) {
        // Use the last valid position instead
        ConvertGameToRecastCoord(lastValidOrg, agentPos);
        status = navigationMap.GetNavMeshQuery()->findNearestPoly(agentPos, DETOUR_EXTENT, &filter, &ref, nearest);
    }

    if (dtStatusSucceed(status) && ref) {
        detourData->corridor.reset(ref, nearest);
        ConvertRecastToGameCoord(agentPos, lastValidOrg);
    } else {
        detourData->corridor.reset(0, agentPos);
    }
#endif
}

void RecastPather::RemoveAgent()
{
#if USE_DETOUR_AGENT
    if (!hasAgent) {
        return;
    }

    pathMaster.agentManager.GetCrowd()->removeAgent(navAgentId);
    hasAgent = false;
#endif
}

RecastAgentManager::RecastAgentManager()
{
#if USE_DETOUR_AGENT
    crowd = NULL;
#endif
}

RecastAgentManager::~RecastAgentManager()
{
#if USE_DETOUR_AGENT
    DestroyCrowd();
#endif
}

void RecastAgentManager::CreateCrowd(float agentRadius, dtNavMesh *mesh)
{
    this->agentRadius = agentRadius;

#if USE_DETOUR_AGENT
    crowd = dtAllocCrowd();
    crowd->init(MAX_CLIENTS, this->agentRadius, mesh);
#endif
}

void RecastAgentManager::DestroyCrowd()
{
#if USE_DETOUR_AGENT
    if (crowd) {
        dtFreeCrowd(crowd);
        crowd = NULL;
    }
#endif
}

dtCrowd *RecastAgentManager::GetCrowd() const
{
#if USE_DETOUR_AGENT
    return crowd;
#else
    return NULL;
#endif
}

void RecastAgentManager::Update()
{
#if USE_DETOUR_AGENT
    if (!crowd) {
        return;
    }

    crowd->update(level.frametime, NULL);
#endif
}

float RecastAgentManager::GetAgentRadius() const
{
    return agentRadius;
}

void RecastPathMaster::PostLoadNavigation(const NavigationMap& map)
{
    agentManager.CreateCrowd(MAXS_X, map.GetNavMesh());
}

void RecastPathMaster::ClearNavigation()
{
    agentManager.DestroyCrowd();
}

void RecastPathMaster::Update()
{
    agentManager.Update();
}
