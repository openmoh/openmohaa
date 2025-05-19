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
class dtTileCache;
struct dtTileCacheAlloc;
struct dtTileCacheCompressor;
struct dtTileCacheMeshProcess;
class RecastBuildContext;
struct rcPolyMesh;
struct rcPolyMeshDetail;

/**
 * @brief An offmesh point that the navigation system will use to find path.
 *
 */
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

/**
 * @brief Full navigation map with meshes and tiles.
 *
 */
class NavigationMap
{
public:
    NavigationMap();
    ~NavigationMap();

    /**
     * @brief Generate the navigation system from the specified BSP map file.
     * 
     * @param mapname The .bsp map file.
     */
    void LoadWorldMap(const char *mapname);

    /**
     * @brief Do some cleanups
     * 
     * @param samemap Whether or not it's the same map
     */
    void CleanUp(qboolean samemap);

    /**
     * @brief Clear and free memory allocated for the navigation system.
     */
    void ClearNavigation();

    /**
     * @brief Whether or not the navigation system is valid.
     * 
     * @return true if there is a valid navigation generated.
     */
    bool IsValid() const;

    /**
     * @brief Get the navigation mesh.
     * 
     * @return dtNavMesh* The navigation mesh.
     */
    dtNavMesh *GetNavMesh() const;

    /**
     * @brief Get navigation query object.
     * 
     * @return dtNavMeshQuery* The navigation query object.
     */
    dtNavMeshQuery *GetNavMeshQuery() const;

    /**
     * @brief Return the navigation data loaded from the BSP.
     * 
     * @return Const navigation data
     */
    const navMap_t& GetNavigationData() const;

    /**
     * @brief Update the navigation map
     * 
     */
    void Update();

public:
    static const float recastCellSize;
    static const float recastCellHeight;
    static const float agentHeight;
    static const float agentMaxClimb;
    static const float agentMaxSlope;
    static const float agentRadius;
    static const int   regionMinSize;
    static const int   regionMergeSize;
    static const float edgeMaxLen;
    static const float edgeMaxError;
    static const int   vertsPerPoly;
    static const float detailSampleDist;
    static const float detailSampleMaxError;

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

    void InitializeNavMesh(RecastBuildContext& buildContext, const navMap_t& navMap);
    void BuildDetourData(
        RecastBuildContext&                      buildContext,
        rcPolyMesh                              *polyMesh,
        rcPolyMeshDetail                        *polyMeshDetail,
        int                                      index,
        const Container<offMeshNavigationPoint>& points
    );

    void BuildRecastMesh(
        RecastBuildContext& buildContext,
        const navModel_t&   model,
        const Vector&       origin,
        const Vector&       angles,
        rcPolyMesh       *&       outPolyMesh,
        rcPolyMeshDetail *& outPolyMeshDetail
    );
    void ProcessBSPForNavigation(const char *mapname, navMap_t& outNavigationMap);

    void BuildWorldMesh(RecastBuildContext& buildContext, const navMap_t& navigationMap);
    void BuildMeshesForEntities(RecastBuildContext& buildContext, const navMap_t& navigationMap);

private:
    dtNavMesh              *navMeshDt;
    dtNavMeshQuery         *navMeshQuery;
    dtTileCache            *tileCache;
    dtTileCacheAlloc       *talloc;
    dtTileCacheCompressor  *tcomp;
    dtTileCacheMeshProcess *tmproc;
    NavigationBSP           navigationData;

public:
    float          *offMeshConVerts;
    float          *offMeshConRad;
    unsigned short *offMeshConFlags;
    unsigned char  *offMeshConAreas;
    unsigned char  *offMeshConDir;
    unsigned int   *offMeshConUserID;
    int             offMeshConCount;
    str             currentMap;
    bool            validNavigation;
};

extern NavigationMap navigationMap;

/**
 * @brief Convert Recast coordinates to game coordinates.
 * 
 * @param in XYZ Recast coordinates.
 * @param out XYZ Game coordinates.
 */
void ConvertRecastToGameCoord(const float *in, float *out);

/**
 * @brief Convert game coordinates to Recast coordinates.
 * 
 * @param in XYZ Game coordinates 
 * @param out XYZ Recast coordinates.
 */
void ConvertGameToRecastCoord(const float *in, float *out);

void G_Navigation_Frame();
