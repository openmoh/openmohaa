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
class dtQueryFilter;
class dtCrowd;
class dtTileCache;
struct dtTileCacheAlloc;
struct dtTileCacheCompressor;
struct dtTileCacheMeshProcess;
class RecastBuildContext;
struct rcPolyMesh;
struct rcPolyMeshDetail;
struct offMeshNavigationPoint;
class INavigationMapExtension;

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
     * @brief Get the query used when querying the navmesh.
     *
     * @return The query filter object.
     */
    const dtQueryFilter *GetQueryFilter() const;

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

private:
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

    void InitializeExtensions();
    void ClearExtensions();

    void InitializeNavMesh(RecastBuildContext& buildContext, const navMap_t& navMap);
    void InitializeFilter();

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
    dtNavMesh      *navMeshDt;
    dtNavMeshQuery *navMeshQuery;
    dtQueryFilter  *queryFilter;
    NavigationBSP   navigationData;

public:
    float          *offMeshConVerts;
    float          *offMeshConRad;
    unsigned short *offMeshConFlags;
    unsigned char  *offMeshConAreas;
    unsigned char  *offMeshConDir;
    unsigned int   *offMeshConUserID;
    int             offMeshConCount;

    Container<INavigationMapExtension *> extensions;

    str  currentMap;
    bool validNavigation;
};

extern NavigationMap navigationMap;

void G_Navigation_Frame();
