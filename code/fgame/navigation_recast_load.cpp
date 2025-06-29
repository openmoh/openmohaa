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
 * @file navigation_recast_load.cpp
 * @brief Modern navigation system using Recast and Detour.
 *
 * Parse BSP file, get surface triangles and rasterize them to create a navigation mesh.
 */

#include "g_local.h"
#include "navigation_recast_load.h"
#include "navigation_recast_load_ext.h"
#include "navigation_recast_obstacle.h"
#include "navigation_recast_path.h"
#include "navigation_recast_config.h"
#include "navigation_recast_helpers.h"
#include "navigation_recast_debug.h"
#include "../script/scriptexception.h"
#include "navigate.h"
#include "debuglines.h"
#include "level.h"

#include "Recast.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshBuilder.h"
#include "DetourNavMeshQuery.h"
#include "DetourNode.h"

NavigationMap navigationMap;

/// Recast build context.
class RecastBuildContext : public rcContext
{
protected:
    virtual void doResetLog() override {}

    virtual void doLog(const rcLogCategory category, const char *msg, const int len) override
    {
        gi.DPrintf("Recast (category %d): %s\n", (int)category, msg);
    }
};

/*
============
NavigationMap::GatherOffMeshPoints
============
*/
void NavigationMap::GatherOffMeshPoints(Container<offMeshNavigationPoint>& points, const rcPolyMesh *polyMesh)
{
    size_t i;

    //
    // Look for extensions and extend the navigation map
    //

    for (i = 1; i <= extensions.NumObjects(); i++) {
        INavigationMapExtension *navExtension = extensions.ObjectAt(i);
        navExtension->Handle(points, polyMesh);
    }
}

/*
============
NavigationMap::InitializeNavMesh
============
*/
void NavigationMap::InitializeNavMesh(RecastBuildContext& buildContext, const navMap_t& navMap)
{
    dtNavMeshParams params;
    dtStatus        status;

    params.orig[0]    = MIN_MAP_BOUNDS;
    params.orig[1]    = MIN_MAP_BOUNDS;
    params.orig[2]    = MIN_MAP_BOUNDS;
    params.tileWidth  = MAP_SIZE * NavigationMapConfiguration::recastCellSize;
    params.tileHeight = MAP_SIZE * NavigationMapConfiguration::recastCellSize;
    params.maxTiles   = 32;
    params.maxPolys   = 32768;

    navMeshDt = dtAllocNavMesh();
    status    = navMeshDt->init(&params);

    if (dtStatusFailed(status)) {
        buildContext.log(RC_LOG_ERROR, "Failed to initialize the navigation mesh");
        return;
    }

    //
    // Create the nav mesh query
    //
    navMeshQuery = dtAllocNavMeshQuery();
    status       = navMeshQuery->init(navMeshDt, MAX_PATHNODES);

    if (dtStatusFailed(status)) {
        buildContext.log(RC_LOG_ERROR, "Could not init Detour navmesh query");
        return;
    }
}

/*
============
NavigationMap::InitializeFilter
============
*/
void NavigationMap::InitializeFilter()
{
    size_t i, j;

    queryFilter = new dtQueryFilter();
    queryFilter->setExcludeFlags(RECAST_POLYFLAG_BUSY);

    for (i = 1; i <= extensions.NumObjects(); i++) {
        INavigationMapExtension *navExtension = extensions.ObjectAt(i);

        Container<ExtensionArea> areas = navExtension->GetSupportedAreas();
        for (j = 1; j <= areas.NumObjects(); j++) {
            const ExtensionArea& area = areas.ObjectAt(j);

            queryFilter->setAreaCost(area.number, area.cost);
        }
    }
}

/*
============
NavigationMap::InitializeExtensions
============
*/
void NavigationMap::InitializeExtensions()
{
    for (const ClassDef *c = ClassDef::classlist; c; c = c->next) {
        if (c != INavigationMapExtension::classinfostatic()
            && checkInheritance(INavigationMapExtension::classinfostatic(), c)) {
            extensions.AddObject(static_cast<INavigationMapExtension *>(c->newInstance()));
        }
    }
}

/*
============
NavigationMap::ClearExtensions
============
*/
void NavigationMap::ClearExtensions()
{
    size_t i;

    for (i = 1; i <= extensions.NumObjects(); i++) {
        delete extensions.ObjectAt(i);
    }

    extensions.FreeObjectList();
}

/*
============
NavigationMap::BuildDetourData
============
*/
void NavigationMap::BuildDetourData(
    RecastBuildContext&                      buildContext,
    rcPolyMesh                              *polyMesh,
    rcPolyMeshDetail                        *polyMeshDetail,
    int                                      index,
    const Container<offMeshNavigationPoint>& points
)
{
    unsigned char *navData     = NULL;
    int            navDataSize = 0;
    dtStatus       status;

    if (polyMesh->npolys < 2) {
        // Useless mesh
        return;
    }

    dtNavMeshCreateParams dtParams {0};

    dtParams.verts            = polyMesh->verts;
    dtParams.vertCount        = polyMesh->nverts;
    dtParams.polys            = polyMesh->polys;
    dtParams.polyAreas        = polyMesh->areas;
    dtParams.polyFlags        = polyMesh->flags;
    dtParams.polyCount        = polyMesh->npolys;
    dtParams.nvp              = polyMesh->nvp;
    dtParams.detailMeshes     = polyMeshDetail->meshes;
    dtParams.detailVerts      = polyMeshDetail->verts;
    dtParams.detailVertsCount = polyMeshDetail->nverts;
    dtParams.detailTris       = polyMeshDetail->tris;
    dtParams.detailTriCount   = polyMeshDetail->ntris;
    dtParams.walkableHeight   = NavigationMapConfiguration::agentHeight;
    dtParams.walkableRadius   = NavigationMapConfiguration::agentRadius;
    dtParams.walkableClimb    = NavigationMapConfiguration::agentMaxClimb;
    rcVcopy(dtParams.bmin, polyMesh->bmin);
    rcVcopy(dtParams.bmax, polyMesh->bmax);
    dtParams.cs          = NavigationMapConfiguration::recastCellSize;
    dtParams.ch          = NavigationMapConfiguration::recastCellHeight;
    dtParams.tileX       = 0;
    dtParams.tileY       = 0;
    dtParams.tileLayer   = index;
    dtParams.buildBvTree = true;

    if (points.NumObjects()) {
        dtParams.offMeshConCount = offMeshConCount = points.NumObjects();

        offMeshConVerts  = new float[6 * dtParams.offMeshConCount];
        offMeshConRad    = new float[dtParams.offMeshConCount];
        offMeshConFlags  = new unsigned short[dtParams.offMeshConCount];
        offMeshConAreas  = new unsigned char[dtParams.offMeshConCount];
        offMeshConDir    = new unsigned char[dtParams.offMeshConCount];
        offMeshConUserID = new unsigned int[dtParams.offMeshConCount];

        for (int i = 0; i < dtParams.offMeshConCount; i++) {
            const offMeshNavigationPoint& point = points.ObjectAt(i + 1);

            ConvertGameToRecastCoord(point.start, &offMeshConVerts[i * 6]);
            ConvertGameToRecastCoord(point.end, &offMeshConVerts[i * 6 + 3]);

            offMeshConRad[i]    = point.radius;
            offMeshConFlags[i]  = point.flags;
            offMeshConAreas[i]  = point.area;
            offMeshConDir[i]    = point.bidirectional ? DT_OFFMESH_CON_BIDIR : 0;
            offMeshConUserID[i] = i;
        }

        dtParams.offMeshConVerts  = offMeshConVerts;
        dtParams.offMeshConRad    = offMeshConRad;
        dtParams.offMeshConFlags  = offMeshConFlags;
        dtParams.offMeshConAreas  = offMeshConAreas;
        dtParams.offMeshConDir    = offMeshConDir;
        dtParams.offMeshConUserID = offMeshConUserID;
    }

    dtCreateNavMeshData(&dtParams, &navData, &navDataSize);

    if (points.NumObjects()) {
        delete[] offMeshConVerts;
        delete[] offMeshConRad;
        delete[] offMeshConFlags;
        delete[] offMeshConAreas;
        delete[] offMeshConDir;
        delete[] offMeshConUserID;
    }

    status = navMeshDt->addTile(navData, navDataSize, DT_TILE_FREE_DATA, 0, NULL);
    if (!dtStatusSucceed(status)) {
        buildContext.log(RC_LOG_ERROR, "Failed to create tile for navigation mesh");
    }
}

/*
============
NavigationMap::GeneratePolyMesh
============
*/
void NavigationMap::GeneratePolyMesh(
    RecastBuildContext& buildContext,
    float              *vertsBuffer,
    int                 numVertices,
    int                *indexesBuffer,
    int                 numIndexes,
    rcPolyMesh       *&       outPolyMesh,
    rcPolyMeshDetail *& outPolyMeshDetail
)
{
    Vector             minBounds, maxBounds;
    int                gridSizeX, gridSizeZ;
    const unsigned int walkableHeight =
        (int)ceilf(NavigationMapConfiguration::agentHeight / NavigationMapConfiguration::recastCellHeight);
    const unsigned int walkableClimb =
        (int)floorf(NavigationMapConfiguration::agentMaxClimb / NavigationMapConfiguration::recastCellHeight);
    const unsigned int walkableRadius =
        (int)ceilf(NavigationMapConfiguration::agentRadius / NavigationMapConfiguration::recastCellSize);
    const unsigned int maxEdgeLen =
        (int)(NavigationMapConfiguration::edgeMaxLen / NavigationMapConfiguration::recastCellSize);
    const unsigned int numTris = numIndexes / 3;

    //
    // Calculate the grid size
    //
    rcCalcBounds(vertsBuffer, numVertices, minBounds, maxBounds);
    rcCalcGridSize(minBounds, maxBounds, NavigationMapConfiguration::recastCellSize, &gridSizeX, &gridSizeZ);

    rcHeightfield *heightfield = rcAllocHeightfield();

    //
    // Rasterize polygons
    //

    rcCreateHeightfield(
        &buildContext,
        *heightfield,
        gridSizeX,
        gridSizeZ,
        minBounds,
        maxBounds,
        NavigationMapConfiguration::recastCellSize,
        NavigationMapConfiguration::recastCellHeight
    );

    unsigned char *triAreas = new unsigned char[numTris];
    memset(triAreas, 0, sizeof(unsigned char) * numTris);

    rcMarkWalkableTriangles(
        &buildContext,
        NavigationMapConfiguration::agentMaxSlope,
        vertsBuffer,
        numVertices,
        indexesBuffer,
        numTris,
        triAreas
    );
    rcRasterizeTriangles(
        &buildContext, vertsBuffer, numVertices, indexesBuffer, triAreas, numTris, *heightfield, walkableClimb
    );

    delete[] triAreas;

    //
    // Filter walkable surfaces
    //

    rcFilterLowHangingWalkableObstacles(&buildContext, walkableClimb, *heightfield);

    //rcFilterLedgeSpans(&buildContext, walkableHeight, walkableClimb, *heightfield);
    rcFilterWalkableLowHeightSpans(&buildContext, walkableHeight, *heightfield);

    // Partition walkable surfaces

    rcCompactHeightfield *compactedHeightfield = rcAllocCompactHeightfield();

    rcBuildCompactHeightfield(&buildContext, walkableHeight, walkableClimb, *heightfield, *compactedHeightfield);

    // The heightfield is not needed anymore
    rcFreeHeightField(heightfield);

    rcErodeWalkableArea(&buildContext, walkableRadius, *compactedHeightfield);

    rcBuildDistanceField(&buildContext, *compactedHeightfield);
    rcBuildRegions(
        &buildContext,
        *compactedHeightfield,
        0,
        Square(NavigationMapConfiguration::regionMinSize),
        Square(NavigationMapConfiguration::regionMergeSize)
    );

    //rcBuildRegionsMonotone(&buildContext, *compactedHeightfield, 0, Square(NavigationMapConfiguration::regionMinSize), Square(NavigationMapConfiguration::regionMergeSize));

    //
    // Simplify region contours
    //

    rcContourSet *contourSet = rcAllocContourSet();

    rcBuildContours(
        &buildContext, *compactedHeightfield, NavigationMapConfiguration::edgeMaxError, maxEdgeLen, *contourSet
    );

    //
    // Build polygon mesh from contours
    //

    rcPolyMesh *polyMesh = rcAllocPolyMesh();

    rcBuildPolyMesh(&buildContext, *contourSet, NavigationMapConfiguration::vertsPerPoly, *polyMesh);

    //
    // Create detail mesh to access approximate height for each polygon
    //

    rcPolyMeshDetail *polyMeshDetail = rcAllocPolyMeshDetail();

    rcBuildPolyMeshDetail(
        &buildContext,
        *polyMesh,
        *compactedHeightfield,
        NavigationMapConfiguration::recastCellSize * NavigationMapConfiguration::detailSampleDist,
        NavigationMapConfiguration::detailSampleMaxError,
        *polyMeshDetail
    );

    rcFreeCompactHeightfield(compactedHeightfield);
    rcFreeContourSet(contourSet);

    outPolyMesh       = polyMesh;
    outPolyMeshDetail = polyMeshDetail;
}

/*
============
NavigationMap::BuildRecastMesh
============
*/
void NavigationMap::BuildRecastMesh(
    RecastBuildContext& buildContext,
    const navModel_t&   model,
    const Vector&       origin,
    const Vector&       angles,
    rcPolyMesh       *&       outPolyMesh,
    rcPolyMeshDetail *& outPolyMeshDetail
)
{
    int numIndexes  = 0;
    int numVertices = 0;
    int baseVertice, baseIndice;
    int i, j;

    for (i = 1; i <= model.surfaces.NumObjects(); i++) {
        const navSurface_t& surface = model.surfaces.ObjectAt(i);

        numIndexes += surface.indices.NumObjects();
        numVertices += surface.vertices.NumObjects();
    }

    //
    // Recreate the vertice buffer so it's compatible
    // with Recast's right-handed Y-up coordinate system
    float *vertsBuffer = new float[numVertices * 3];

    baseIndice  = 0;
    baseVertice = 0;

    if (angles != vec_zero) {
        float axis[3][3];

        AnglesToAxis(angles, axis);
        for (i = 1; i <= model.surfaces.NumObjects(); i++) {
            const navSurface_t& surface = model.surfaces.ObjectAt(i);

            for (j = 0; j < surface.vertices.NumObjects(); j++) {
                const navVertice_t& inVertice = surface.vertices.ObjectAt(j + 1);
                Vector              offset;

                MatrixTransformVector(inVertice.xyz, axis, offset);
                offset += origin;

                ConvertGameToRecastCoord(offset, &vertsBuffer[baseVertice * 3 + j * 3]);
            }

            baseVertice += surface.vertices.NumObjects();
        }
    } else {
        for (i = 1; i <= model.surfaces.NumObjects(); i++) {
            const navSurface_t& surface = model.surfaces.ObjectAt(i);

            for (j = 0; j < surface.vertices.NumObjects(); j++) {
                const navVertice_t& inVertice = surface.vertices.ObjectAt(j + 1);
                const Vector        offset    = inVertice.xyz + origin;

                ConvertGameToRecastCoord(offset, &vertsBuffer[baseVertice * 3 + j * 3]);
            }

            baseVertice += surface.vertices.NumObjects();
        }
    }

    baseIndice  = 0;
    baseVertice = 0;

    int *indexesBuffer = new int[numIndexes];
    for (i = 1; i <= model.surfaces.NumObjects(); i++) {
        const navSurface_t& surface = model.surfaces.ObjectAt(i);

        const int numTris = surface.indices.NumObjects() / 3;

        for (j = 0; j < numTris; j++) {
            indexesBuffer[baseIndice + j * 3 + 0] = baseVertice + surface.indices[j * 3 + 2].indice;
            indexesBuffer[baseIndice + j * 3 + 1] = baseVertice + surface.indices[j * 3 + 1].indice;
            indexesBuffer[baseIndice + j * 3 + 2] = baseVertice + surface.indices[j * 3 + 0].indice;
        }

        baseIndice += surface.indices.NumObjects();
        baseVertice += surface.vertices.NumObjects();
    }

    //
    // Generate the mesh
    //

    rcPolyMesh       *polyMesh;
    rcPolyMeshDetail *polyMeshDetail;

    GeneratePolyMesh(buildContext, vertsBuffer, numVertices, indexesBuffer, numIndexes, polyMesh, polyMeshDetail);

    // Update poly flags from areas.
    for (int i = 0; i < polyMesh->npolys; ++i) {
        if (polyMesh->areas[i] == RC_WALKABLE_AREA) {
            polyMesh->flags[i] = RECAST_POLYFLAG_WALKABLE;
        }
    }

    delete[] indexesBuffer;
    delete[] vertsBuffer;

    outPolyMesh       = polyMesh;
    outPolyMeshDetail = polyMeshDetail;
}

/*
============
G_Navigation_Frame
============
*/
void G_Navigation_Frame()
{
    pathMaster.Update();
    navigationMap.Update();
    navigationObstacleMap.Update();
    G_Navigation_DebugDraw();
}

/*
============
NavigationMap::NavigationMap
============
*/
NavigationMap::NavigationMap()
    : navMeshDt(NULL)
    , navMeshQuery(NULL)
    , queryFilter(NULL)
{
    validNavigation = false;
}

/*
============
NavigationMap::~NavigationMap
============
*/
NavigationMap::~NavigationMap()
{
    ClearNavigation();
}

/*
============
NavigationMap::GetNavMesh
============
*/
dtNavMesh *NavigationMap::GetNavMesh() const
{
    return navMeshDt;
}

/*
============
NavigationMap::GetNavMeshQuery
============
*/
dtNavMeshQuery *NavigationMap::GetNavMeshQuery() const
{
    return navMeshQuery;
}

/*
============
NavigationMap::GetQueryFilter
============
*/
const dtQueryFilter *NavigationMap::GetQueryFilter() const
{
    return queryFilter;
}

/*
============
NavigationMap::GetNavigationData
============
*/
const navMap_t& NavigationMap::GetNavigationData() const
{
    return navigationData.navMap;
}

/*
============
NavigationMap::Update
============
*/
void NavigationMap::Update() {}

/*
============
NavigationMap::ClearNavigation
============
*/
void NavigationMap::ClearNavigation()
{
    size_t i;

    validNavigation = false;

    pathMaster.ClearNavigation();
    navigationObstacleMap.Clear();

    if (navMeshQuery) {
        dtFreeNavMeshQuery(navMeshQuery);
        navMeshQuery = NULL;
    }

    if (navMeshDt) {
        dtFreeNavMesh(navMeshDt);
        navMeshDt = NULL;
    }

    if (queryFilter) {
        delete queryFilter;
        queryFilter = NULL;
    }

    ClearExtensions();
}

bool NavigationMap::IsValid() const
{
    return navMeshDt != NULL;
}

/*
============
NavigationMap::BuildWorldMesh
============
*/
void NavigationMap::BuildWorldMesh(RecastBuildContext& buildContext, const navMap_t& navigationMap)
{
    rcPolyMesh       *polyMesh;
    rcPolyMeshDetail *polyMeshDetail;

    BuildRecastMesh(buildContext, navigationMap.GetWorldMap(), vec_origin, vec_zero, polyMesh, polyMeshDetail);

    //
    // Create detour data
    //

    Container<offMeshNavigationPoint> points;
    GatherOffMeshPoints(points, polyMesh);

    BuildDetourData(buildContext, polyMesh, polyMeshDetail, 0, points);

    rcFreePolyMeshDetail(polyMeshDetail);
    rcFreePolyMesh(polyMesh);
}

/*
============
NavigationMap::BuildMeshesForEntities
============
*/
void NavigationMap::BuildMeshesForEntities(RecastBuildContext& buildContext, const navMap_t& navigationMap)
{
    gentity_t        *edict;
    rcPolyMesh       *polyMesh;
    rcPolyMeshDetail *polyMeshDetail;

    for (edict = active_edicts.next; edict != &active_edicts; edict = edict->next) {
        if (!edict->entity || edict->entity == world) {
            continue;
        }

        switch (edict->solid) {
        case SOLID_TRIGGER:
        default:
            continue;
        case SOLID_BSP:
            break;
        }

        if (edict->s.modelindex < 1 || edict->s.modelindex > navigationMap.GetNumSubmodels()) {
            continue;
        }

        const navModel_t& submodel = navigationMap.GetSubmodel(edict->s.modelindex - 1);
        if (!submodel.surfaces.NumObjects()) {
            // Could be a trigger
            continue;
        }

        BuildRecastMesh(buildContext, submodel, edict->entity->origin, edict->entity->angles, polyMesh, polyMeshDetail);

        BuildDetourData(buildContext, polyMesh, polyMeshDetail, edict->s.modelindex, {});

        rcFreePolyMeshDetail(polyMeshDetail);
        rcFreePolyMesh(polyMesh);
    }
}

/*
============
NavigationMap::LoadWorldMap
============
*/
void NavigationMap::LoadWorldMap(const char *mapname)
{
    RecastBuildContext buildContext;
    int                start, end;

    gi.Printf("---- Recast Navigation ----\n");

    //
    // Free up existing navigation if there is one
    //

    if (currentMap != mapname) {
        currentMap = mapname;
        ClearNavigation();
    }

    if (!sv_maxbots->integer) {
        gi.Printf("No bots, skipping navigation\n");
        return;
    }

    if (validNavigation) {
        return;
    }

    //
    // Parse the BSP file into triangles
    //

    try {
        start = gi.Milliseconds();

        navigationData.ProcessBSPForNavigation(mapname);
    } catch (const ScriptException& e) {
        gi.Printf("Failed to load BSP for navigation: %s\n", e.string.c_str());
        ClearNavigation();
        return;
    }

    end = gi.Milliseconds();

    gi.Printf("BSP file loaded and parsed in %.03f seconds\n", (float)((end - start) / 1000.0));

    //
    // Build and create the navigation mesh
    //

    InitializeExtensions();

    InitializeNavMesh(buildContext, navigationData.navMap);
    InitializeFilter();

    gi.Printf("Building the navigation mesh...\n");

    try {
        start = gi.Milliseconds();

        gi.Printf("  Building the world mesh...\n");
        BuildWorldMesh(buildContext, navigationData.navMap);

        gi.Printf("  Building meshes for entities...\n");
        // FIXME: TODO
        //  Split everything into chunks and use a tile-based approach
        //BuildMeshesForEntities(buildContext, navigationData.navMap);

    } catch (const ScriptException& e) {
        gi.Printf("Couldn't build recast navigation mesh: %s\n", e.string.c_str());
        ClearNavigation();
        return;
    }

    pathMaster.PostLoadNavigation(*this);
    navigationObstacleMap.Init();

    // Finished processing the map with extensions
    ClearExtensions();

    end = gi.Milliseconds();

    gi.Printf("Recast navigation mesh(es) generated in %.03f seconds\n", (float)((end - start) / 1000.0));

    validNavigation = true;
}

void NavigationMap::CleanUp(qboolean samemap)
{
    if (!samemap) {
        ClearNavigation();
    }
}
