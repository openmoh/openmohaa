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

#pragma once

#include "g_local.h"
#include "navigation_bsp.h"

class dtNavMesh;
class dtNavMeshQuery;
class dtCrowd;
class RecastBuildContext;
struct rcPolyMesh;
struct rcPolyMeshDetail;

struct offMeshNavigationPoint {
    Vector         start;
    Vector         end;
    float          radius;
    unsigned short flags;
    unsigned char  area;
    bool           bidirectional;
    int            id;

    offMeshNavigationPoint()
        : radius(0)
        , flags(0)
        , area(0)
        , bidirectional(true)
        , id(0)
    {}

    bool operator==(const offMeshNavigationPoint& other) const { return start == other.start && end == other.end; }

    bool operator!=(const offMeshNavigationPoint& other) const { return !(*this == other); }
};

class NavigationMap
{
public:
    NavigationMap();
    ~NavigationMap();

    void LoadWorldMap(const char *mapname);
    void ClearNavigation();
    bool IsValid() const;

    dtNavMesh      *GetNavMesh() const;
    dtNavMeshQuery *GetNavMeshQuery() const;

private:
    void                   ConnectLadders(Container<offMeshNavigationPoint>& points);
    offMeshNavigationPoint CanConnectFallPoint(const rcPolyMesh *polyMesh, const Vector& pos1, const Vector& pos2);
    offMeshNavigationPoint CanConnectJumpPoint(const rcPolyMesh *polyMesh, const Vector& pos1, const Vector& pos2);
    void TryConnectJumpFallPoints(Container<offMeshNavigationPoint>& points, const rcPolyMesh *polyMesh);
    void GatherOffMeshPoints(Container<offMeshNavigationPoint>& points, const rcPolyMesh *polyMesh);
    void GeneratePolyMesh(
        RecastBuildContext& buildContext,
        float              *vertsBuffer,
        int                 numVertices,
        int                *indexesBuffer,
        int                 numIndexes,
        rcPolyMesh       *&       outPolyMesh,
        rcPolyMeshDetail *& outPolyMeshDetail
    );
    void BuildDetourData(RecastBuildContext& buildContext, rcPolyMesh *polyMesh, rcPolyMeshDetail *polyMeshDetail);

    void BuildRecastMesh(navMap_t& navigationMap);
    void ProcessBSPForNavigation(const char *mapname, navMap_t& outNavigationMap);

private:
    dtNavMesh      *navMeshDt;
    dtNavMeshQuery *navMeshQuery;
};

extern NavigationMap navigationMap;

void G_Navigation_DebugDraw();
