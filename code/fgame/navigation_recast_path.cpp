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

// navigation -- Modern navigation system using Recast and Detour

#include "navigation_recast_path.h"
#include "navigation_recast_load.h"
#include "level.h"

#include "DetourCrowd.h"
#include "entity.h"
#include "bg_local.h"

RecastPathMaster pathMaster;

RecastPather::RecastPather()
    : nextCheckTime(0)
    , hasAgent(false)
{}

RecastPather::~RecastPather() {}

void RecastPather::FindPath(const Vector& start, const Vector& end, const PathSearchParameter& parameters)
{
    Vector        recastStart, recastEnd;
    Vector        half(16, 16, 16);
    dtPolyRef     endRef;
    vec3_t        endPt;
    dtQueryFilter filter;

    lastorg = start;
    ResetAgent(start);

    ConvertGameToRecastCoord(start, recastStart);
    ConvertGameToRecastCoord(end, recastEnd);

    if (navigationMap.GetNavMeshQuery()->findNearestPoly(recastEnd, half, &filter, &endRef, endPt) != DT_SUCCESS
        || !endRef) {
        pathMaster.agentManager.GetCrowd()->resetMoveTarget(navAgentId);
        return;
    }

    pathMaster.agentManager.GetCrowd()->requestMoveTarget(navAgentId, endRef, endPt);
}

void RecastPather::FindPathNear(
    const Vector& start, const Vector& end, float radius, const PathSearchParameter& parameters
)
{
    Vector        recastStart, recastEnd;
    Vector        half(16, 16, 16);
    dtPolyRef     endRef;
    vec3_t        endPt;
    dtQueryFilter filter;

    lastorg = start;
    ResetAgent(start);

    ConvertGameToRecastCoord(start, recastStart);
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

    pathMaster.agentManager.GetCrowd()->requestMoveTarget(navAgentId, endRef, endPt);
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
    dtPolyRef     endRef;
    vec3_t        endPt;
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

    for (i = 0; i < 36; i++) {
        float angle = startAngle + ((2 * M_PI) * (float)i / (float)36);
        float dx    = cos(angle) * radius;
        float dz    = sin(angle) * radius;

        for(j = 0; j < 4; j++) {
            float pitch = startPitch + (M_PI * (float)j / 4);
            float dy = sin(pitch) * radius;
            Vector point(recastAvoid[0] + dx, recastAvoid[1] + dy, recastAvoid[2] + dz);

            if (navigationMap.GetNavMeshQuery()->findNearestPoly(point, half, &filter, &endRef, endPt) == DT_SUCCESS
                && endRef) {
                pathMaster.agentManager.GetCrowd()->requestMoveTarget(navAgentId, endRef, endPt);
                return;
            }
        }
    }

    pathMaster.agentManager.GetCrowd()->resetMoveTarget(navAgentId);
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

void RecastPather::UpdatePos(const Vector& origin, float speed)
{
    dtCrowd      *crowdManager = pathMaster.agentManager.GetCrowd();
    dtCrowdAgent *agent;

    lastorg = origin;

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

        ConvertRecastToGameCoord(agent->npos, agentPos);
        delta = agentPos - lastorg;
        if (delta.lengthSquared() < Square(16)) {
            // traversed
            traversingOffMeshLink = false;
        }
    } else if (level.inttime >= nextCheckTime) {
        ConvertGameToRecastCoord(origin, agent->npos);
        if (agent->targetState != DT_CROWDAGENT_TARGET_REQUESTING
            && agent->targetState != DT_CROWDAGENT_TARGET_WAITING_FOR_QUEUE
            && agent->targetState != DT_CROWDAGENT_TARGET_WAITING_FOR_PATH) {
            agent->corridor.movePosition(
                agent->npos, navigationMap.GetNavMeshQuery(), crowdManager->getFilter(0)
            );

            Vector delta = Vector(agent->npos) - Vector(agent->corridor.getPos());
            if (delta.lengthSquared() > Square(64)) {
                agent->targetState = DT_CROWDAGENT_TARGET_REQUESTING;
                agent->corridor.reset(0, agent->npos);
            }
        }

        nextCheckTime = level.inttime + 2000;
        return;
    }

    //Vector agentPos;
    //ConvertRecastToGameCoord(agent->npos, agentPos);
    //g_entities[2].entity->setOrigin(agentPos);
}

void RecastPather::Clear()
{
    dtCrowd *crowdManager;

    if (!hasAgent) {
        return;
    }

    crowdManager = pathMaster.agentManager.GetCrowd();
    crowdManager->resetMoveTarget(navAgentId);
}

PathNav RecastPather::GetNode(unsigned int index) const
{
    const dtCrowdAgent *agent;
    PathNav             nav;
    Vector              target;
    Vector              delta;
    Vector              agentPos;
    const dtPolyRef    *path;
    int                 npath;

    if (!hasAgent) {
        return {};
    }

    agent = pathMaster.agentManager.GetCrowd()->getAgent(navAgentId);

    path  = agent->corridor.getPath();
    npath = agent->corridor.getPathCount();
    if (npath <= 0) {
        return {};
    }

    if (index + 1 == npath) {
        // Just return the target pos
        ConvertRecastToGameCoord(agent->corridor.getTarget(), nav.origin);
        ConvertRecastToGameCoord(agent->npos, agentPos);

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
}

Vector RecastPather::GetCurrentDelta() const
{
    dtCrowd *crowdManager = pathMaster.agentManager.GetCrowd();
    Vector   delta;

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

    return delta;
}

Vector RecastPather::GetCurrentDirection() const
{
    dtCrowd *crowdManager = pathMaster.agentManager.GetCrowd();
    Vector   delta;

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

    delta.normalize();

    return delta;
}

Vector RecastPather::GetDestination() const
{
    Vector dest;

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

    return dest;
}

bool RecastPather::HasReachedGoal(const Vector& origin) const
{
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

    return false;
}

bool RecastPather::IsQuerying() const
{
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
}

void RecastPather::ResetAgent(const Vector& origin)
{
    dtCrowd *crowdManager = pathMaster.agentManager.GetCrowd();
    vec3_t   rcOrigin;

    traversingOffMeshLink = false;

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
            ConvertRecastToGameCoord(agent->npos, lastValidOrg);
        } else {
            agent->corridor.reset(0, agent->npos);
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
}

void RecastPather::RemoveAgent()
{
    if (!hasAgent) {
        return;
    }

    pathMaster.agentManager.GetCrowd()->removeAgent(navAgentId);
    hasAgent = false;
}

RecastAgentManager::RecastAgentManager()
{
    crowd = NULL;
}

RecastAgentManager::~RecastAgentManager()
{
    DestroyCrowd();
}

void RecastAgentManager::CreateCrowd(float agentRadius, dtNavMesh *mesh)
{
    this->agentRadius = agentRadius;

    crowd = dtAllocCrowd();
    crowd->init(MAX_CLIENTS, this->agentRadius, mesh);
}

void RecastAgentManager::DestroyCrowd()
{
    if (crowd) {
        dtFreeCrowd(crowd);
        crowd = NULL;
    }
}

dtCrowd *RecastAgentManager::GetCrowd() const
{
    return crowd;
}

void RecastAgentManager::Update()
{
    if (!crowd) {
        return;
    }

    crowd->update(level.frametime, NULL);
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
