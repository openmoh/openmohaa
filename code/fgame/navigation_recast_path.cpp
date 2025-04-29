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

RecastPathMaster pathMaster;

RecastPather::RecastPather()
    : hasAgent(false)
{}

RecastPather::~RecastPather() {}

void RecastPather::FindPath(const Vector& start, const Vector& end, const PathSearchParameter& parameters)
{
    lastorg = start;
    ResetAgent(start);
}

void RecastPather::FindPathNear(
    const Vector& start, const Vector& end, float radius, const PathSearchParameter& parameters
)
{
    lastorg = start;
    ResetAgent(start);
}

void RecastPather::FindPathAway(
    const Vector&              start,
    const Vector&              avoid,
    const Vector&              preferredDir,
    float                      radius,
    const PathSearchParameter& parameters
)
{
    lastorg = start;
    ResetAgent(start);
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

void RecastPather::UpdatePos(const Vector& origin)
{
    ResetAgent(origin);
}

void RecastPather::Clear()
{
    RemoveAgent();
}

PathNav RecastPather::GetNode(unsigned int index) const
{
    return {};
}

int RecastPather::GetNodeCount() const
{
    return 0;
}

Vector RecastPather::GetCurrentDelta() const
{
    return Vector();
}

bool RecastPather::HasReachedGoal(const Vector& origin) const
{
    return false;
}

void RecastPather::ResetAgent(const Vector& origin)
{
    dtCrowd *crowdManager = pathMaster.agentManager.GetCrowd();
    vec3_t   rcOrigin;

    if (hasAgent) {
        crowdManager->resetMoveTarget(navAgentId);
        return;
    }

    ConvertGameToRecastCoord(origin, rcOrigin);

    dtCrowdAgentParams ap {0};
    ap.radius                = NavigationMap::agentRadius;
    ap.height                = NavigationMap::agentHeight;
    ap.maxAcceleration       = 8.0f;
    ap.maxSpeed              = 3.5f;
    ap.collisionQueryRange   = ap.radius * 12.0f;
    ap.pathOptimizationRange = ap.radius * 30.0f;
    ap.updateFlags           = 0;

    navAgentId = crowdManager->addAgent(rcOrigin, &ap);
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
    crowd = dtAllocCrowd();
    crowd->init(MAX_CLIENTS, agentRadius, mesh);
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

void RecastPathMaster::PostLoadNavigation(const NavigationMap& map)
{
    agentManager.CreateCrowd(map.agentRadius, map.GetNavMesh());
}

void RecastPathMaster::ClearNavigation()
{
    agentManager.DestroyCrowd();
}

void RecastPathMaster::Update()
{
    agentManager.Update();
}
