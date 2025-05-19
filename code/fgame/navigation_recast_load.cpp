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
#include "navigation_recast_path.h"
#include "../script/scriptexception.h"
#include "navigate.h"
#include "debuglines.h"
#include "level.h"

#include "Recast.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshBuilder.h"
#include "DetourNavMeshQuery.h"
#include "DetourCrowd.h"
#include "DetourNode.h"
#include "DetourTileCache.h"
#include "DetourTileCacheBuilder.h"

NavigationMap navigationMap;

const float NavigationMap::recastCellSize   = 12.25;
const float NavigationMap::recastCellHeight = 1.0;
const float NavigationMap::agentHeight      = DEFAULT_VIEWHEIGHT;
const float NavigationMap::agentMaxClimb    = STEPSIZE;

// normal of { 0.714142799, 0, 0.700000048 }, or an angle of -44.4270058
const float NavigationMap::agentMaxSlope = 45.5729942f;

const float NavigationMap::agentRadius          = 0.5;
const int   NavigationMap::regionMinSize        = 5;
const int   NavigationMap::regionMergeSize      = 20;
const float NavigationMap::edgeMaxLen           = 100.0;
const float NavigationMap::edgeMaxError         = 1.3f;
const int   NavigationMap::vertsPerPoly         = 6;
const float NavigationMap::detailSampleDist     = 12.0;
const float NavigationMap::detailSampleMaxError = 1.0;
const float NavigationMap::agentJumpHeight      = 54;

dtCrowd *navCrowd;
int      navAgentId = -1;

static Vector ai_startpath;
static Vector ai_endpath;
static Vector ai_pathlist[256];
static int    ai_numPaths = 0;
static int    ai_lastpath = 0;

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

//
// Just so it stops being an annoyance
//
struct NoCompressor : public dtTileCacheCompressor {
    virtual ~NoCompressor() {}

    virtual int maxCompressedSize(const int bufferSize) { return (int)bufferSize; }

    virtual dtStatus compress(
        const unsigned char *buffer,
        const int            bufferSize,
        unsigned char       *compressed,
        const int /*maxCompressedSize*/,
        int *compressedSize
    )
    {
        *compressedSize = bufferSize;
        memcpy(compressed, buffer, bufferSize);
        return DT_SUCCESS;
    }

    virtual dtStatus decompress(
        const unsigned char *compressed,
        const int            compressedSize,
        unsigned char       *buffer,
        const int            maxBufferSize,
        int                 *bufferSize
    )
    {
        memcpy(buffer, compressed, compressedSize);
        *bufferSize = compressedSize;
        return DT_SUCCESS;
    }
};

struct MeshProcess : public dtTileCacheMeshProcess {
    inline MeshProcess() {}

    virtual ~MeshProcess();

    inline void init() {}

    virtual void process(struct dtNavMeshCreateParams *params, unsigned char *polyAreas, unsigned short *polyFlags)
    {
        // Update poly flags from areas.
        for (int i = 0; i < params->polyCount; ++i) {
            if (polyAreas[i] == RC_WALKABLE_AREA) {
                polyFlags[i] = 1;
            }
        }

        // Pass in off-mesh connections.
        params->offMeshConVerts  = navigationMap.offMeshConVerts;
        params->offMeshConRad    = navigationMap.offMeshConVerts;
        params->offMeshConDir    = navigationMap.offMeshConDir;
        params->offMeshConAreas  = navigationMap.offMeshConAreas;
        params->offMeshConFlags  = navigationMap.offMeshConFlags;
        params->offMeshConUserID = navigationMap.offMeshConUserID;
        params->offMeshConCount  = navigationMap.offMeshConCount;
    }
};

MeshProcess::~MeshProcess()
{
    // Defined out of line to fix the weak v-tables warning
}

/*
============
ConvertGameToRecastCoord
============
*/
void ConvertGameToRecastCoord(const float *in, float *out)
{
#if 0
    vec3_t mat[3];
    vec3_t angles = { 0, 0, -90 };
    AnglesToAxis(angles, mat);

    MatrixTransformVector(in, mat, out);
#endif

    // Same as above
    out[0] = in[0];
    out[1] = in[2];
    out[2] = -in[1];
}

/*
============
ConvertRecastToGameCoord
============
*/
void ConvertRecastToGameCoord(const float *in, float *out)
{
#if 0
    vec3_t mat[3];
    vec3_t angles = { 0, 0, 90 };
    AnglesToAxis(angles, mat);

    MatrixTransformVector(in, mat, out);
#endif

    out[0] = in[0];
    out[1] = -in[2];
    out[2] = in[1];
}

void TestAgent(const Vector& start, const Vector& end, Vector *paths, int *numPaths, int maxPaths)
{
    vec3_t        half = {64, 64, 64};
    vec3_t        startNav;
    vec3_t        endNav;
    dtQueryFilter filter;
    dtPolyRef     nearestStartRef, nearestEndRef;
    vec3_t        nearestStartPt, nearestEndPt;

    ConvertGameToRecastCoord(start, startNav);
    ConvertGameToRecastCoord(end, endNav);

    if (navAgentId != -1) {
        navCrowd->removeAgent(navAgentId);
    }

    dtCrowdAgentParams ap {0};

    ap.radius                = NavigationMap::agentRadius;
    ap.height                = NavigationMap::agentHeight;
    ap.maxAcceleration       = 8.0f;
    ap.maxSpeed              = 3.5f;
    ap.collisionQueryRange   = ap.radius * 12.0f;
    ap.pathOptimizationRange = ap.radius * 30.0f;
    ap.updateFlags           = 0;
    navAgentId               = navCrowd->addAgent(startNav, &ap);

    navigationMap.GetNavMeshQuery()->findNearestPoly(startNav, half, &filter, &nearestStartRef, nearestStartPt);
    navigationMap.GetNavMeshQuery()->findNearestPoly(endNav, half, &filter, &nearestEndRef, nearestEndPt);

    navCrowd->requestMoveTarget(navAgentId, nearestEndRef, nearestEndPt);

    dtPolyRef polys[256];
    int       nPolys;
    navigationMap.GetNavMeshQuery()->findPath(
        nearestStartRef, nearestEndRef, nearestStartPt, nearestEndPt, &filter, polys, &nPolys, 256
    );
    navigationMap.GetNavMeshQuery()->findStraightPath(
        nearestStartPt, nearestEndPt, polys, nPolys, (float *)paths, NULL, NULL, numPaths, maxPaths
    );

    for (int i = 0; i < *numPaths; i++) {
        Vector converted;

        ConvertRecastToGameCoord(paths[i], converted);
        paths[i] = converted;
    }
}

/*
============
NavigationMap::ConnectLadders
============
*/
void NavigationMap::ConnectLadders(Container<offMeshNavigationPoint>& points)
{
    gentity_t *edict;

    for (edict = active_edicts.next; edict != &active_edicts; edict = edict->next) {
        if (!edict->entity) {
            continue;
        }

        if (edict->entity->isSubclassOf(FuncLadder)) {
            const FuncLadder *ladder    = static_cast<FuncLadder *>(edict->entity);
            const Vector&     facingDir = ladder->getFacingDir();

            offMeshNavigationPoint point;

            point.start         = edict->entity->origin + Vector(0, 0, edict->entity->mins.z) - facingDir * 32;
            point.end           = edict->entity->origin + Vector(0, 0, edict->entity->maxs.z + 16) + facingDir * 16;
            point.bidirectional = true;
            point.radius        = agentRadius;
            point.area          = 0;
            point.flags         = 1;

            points.AddObject(point);
        }
    }
}

/*
============
NavigationMap::CanConnectFallPoint
============
*/
offMeshNavigationPoint
NavigationMap::CanConnectFallPoint(const rcPolyMesh *polyMesh, const Vector& pos1, const Vector& pos2)
{
    const Vector           mins(-15, -15, 0);
    const Vector           maxs(15, 15, agentHeight);
    Vector                 start, end;
    float                  fallheight;
    float                  dist;
    Vector                 dir;
    trace_t                trace;
    offMeshNavigationPoint point;

    if (pos1.z <= pos2.z) {
        start = pos2;
        end   = pos1;
    } else {
        start = pos1;
        end   = pos2;
    }

    if (start.z - end.z < STEPSIZE) {
        // Can step easily
        return {};
    }

    if (start.z - end.z > 600) {
        // This would cause too much damage
        return {};
    }

    fallheight = start.z - end.z;

    dist = (end - start).lengthXYSquared();
    if (dist > Square(fallheight)) {
        return {};
    }

    dir   = end - start;
    dir.z = 0;
    dir.normalize();

    trace =
        G_Trace(start, mins, maxs, start + dir * Q_min(dist, 32), NULL, MASK_PLAYERSOLID, qtrue, "CanConnectFallPoint");
    if (trace.allsolid || trace.startsolid) {
        return {};
    }

    trace = G_Trace(trace.endpos, mins, maxs, end, NULL, MASK_PLAYERSOLID, qtrue, "CanConnectFallPoint");
    if (trace.fraction < 0.999) {
        return {};
    }

    point.start         = start;
    point.end           = trace.endpos;
    point.bidirectional = false;
    point.radius        = agentRadius;
    point.area          = 1;
    point.flags         = 2;

    return point;
}

/*
============
NavigationMap::CanConnectJumpPoint
============
*/
offMeshNavigationPoint
NavigationMap::CanConnectJumpPoint(const rcPolyMesh *polyMesh, const Vector& pos1, const Vector& pos2)
{
    const Vector           mins(-15, -15, 0);
    const Vector           maxs(15, 15, agentHeight);
    Vector                 start, end;
    float                  jumpheight;
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
    if (jumpheight > agentJumpHeight) {
        return {};
    }

    if ((end - start).lengthSquared() > Square(128)) {
        return {};
    }

    dir   = end - start;
    dir.z = 0;
    dir.normalize();

    trace = G_Trace(start, mins, maxs, end, NULL, MASK_PLAYERSOLID, qtrue, "CanConnectJumpPoint");
    if (trace.fraction >= 0.999) {
        // Straight path
        return {};
    }

    trace = G_Trace(start, mins, maxs, start + Vector(0, 0, 56), NULL, MASK_PLAYERSOLID, qtrue, "CanConnectJumpPoint");
    if (trace.allsolid || trace.startsolid) {
        return {};
    }

    trace = G_Trace(trace.endpos, mins, maxs, end, NULL, MASK_PLAYERSOLID, qtrue, "CanConnectJumpPoint");
    if (trace.fraction < 0.999) {
        return {};
    }

    point.start         = start;
    point.end           = end;
    point.bidirectional = true;
    point.radius        = agentRadius;
    point.area          = 1;
    point.flags         = 3;

    return point;
}

/*
============
NavigationMap::TryConnectJumpFallPoints
============
*/
void NavigationMap::TryConnectJumpFallPoints(Container<offMeshNavigationPoint>& points, const rcPolyMesh *polyMesh)
{
    int    i, j, k, l;
    vec3_t tmp;
    Vector pos1, pos2;

    for (i = 0; i < polyMesh->npolys; i++) {
        const unsigned short *p1 = &polyMesh->polys[i * polyMesh->nvp * 2];

        if (polyMesh->areas[i] != RC_WALKABLE_AREA) {
            continue;
        }

        for (j = i + 1; j < polyMesh->npolys; j++) {
            const unsigned short *p2 = &polyMesh->polys[j * polyMesh->nvp * 2];

            if (i == j) {
                continue;
            }

            if (polyMesh->areas[j] != RC_WALKABLE_AREA) {
                continue;
            }

            if (polyMesh->regs[i] == polyMesh->regs[j]) {
                continue;
            }

            for (k = 0; k < polyMesh->nvp; k++) {
                if (p1[k] == RC_MESH_NULL_IDX) {
                    break;
                }

                const unsigned short *v1 = &polyMesh->verts[p1[k] * 3];

                tmp[0] = polyMesh->bmin[0] + v1[0] * polyMesh->cs;
                tmp[1] = polyMesh->bmin[1] + (v1[1] + 1) * polyMesh->ch;
                tmp[2] = polyMesh->bmin[2] + v1[2] * polyMesh->cs;

                Vector pos1;
                ConvertRecastToGameCoord(tmp, pos1);

                for (l = 0; l < polyMesh->nvp; l++) {
                    if (p2[l] == RC_MESH_NULL_IDX) {
                        break;
                    }

                    if (p1[k] == p2[l]) {
                        continue;
                    }

                    const unsigned short *v2 = &polyMesh->verts[p2[l] * 3];

                    tmp[0] = polyMesh->bmin[0] + v2[0] * polyMesh->cs;
                    tmp[1] = polyMesh->bmin[1] + (v2[1] + 1) * polyMesh->ch;
                    tmp[2] = polyMesh->bmin[2] + v2[2] * polyMesh->cs;

                    Vector pos2;
                    ConvertRecastToGameCoord(tmp, pos2);

                    if ((pos2 - pos1).lengthXYSquared() > Square(256)) {
                        continue;
                    }

                    if (fabs(pos2.z - pos1.z) < STEPSIZE) {
                        // ignore steps
                        continue;
                    }

                    offMeshNavigationPoint point;

                    point = CanConnectFallPoint(polyMesh, pos1, pos2);
                    if (point.area) {
                        points.AddUniqueObject(point);
                    }

                    point = CanConnectJumpPoint(polyMesh, pos1, pos2);
                    if (point.area) {
                        points.AddUniqueObject(point);
                    }
                }
            }
        }
    }
}

/*
============
NavigationMap::GatherOffMeshPoints
============
*/
void NavigationMap::GatherOffMeshPoints(Container<offMeshNavigationPoint>& points, const rcPolyMesh *polyMesh)
{
    ConnectLadders(points);
    TryConnectJumpFallPoints(points, polyMesh);
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
    params.tileWidth  = MAP_SIZE * recastCellSize;
    params.tileHeight = MAP_SIZE * recastCellSize;
    params.maxTiles   = 1;
    params.maxPolys   = 65536;

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

    dtTileCacheParams tcparams;
    memset(&tcparams, 0, sizeof(tcparams));
    tcparams.orig[0]                = MIN_MAP_BOUNDS;
    tcparams.orig[1]                = MIN_MAP_BOUNDS;
    tcparams.orig[2]                = MIN_MAP_BOUNDS;
    tcparams.cs                     = recastCellSize;
    tcparams.ch                     = recastCellHeight;
    tcparams.width                  = MAP_SIZE;
    tcparams.height                 = MAP_SIZE;
    tcparams.walkableHeight         = agentHeight;
    tcparams.walkableRadius         = agentRadius;
    tcparams.walkableClimb          = agentMaxClimb;
    tcparams.maxSimplificationError = edgeMaxError;
    tcparams.maxTiles               = 128;
    tcparams.maxObstacles           = MAX_GENTITIES;

    tileCache = dtAllocTileCache();
    tileCache->init(&tcparams, talloc, tcomp, tmproc);

    navCrowd = dtAllocCrowd();
    navCrowd->init(MAX_CLIENTS, agentRadius, navMeshDt);
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
    dtParams.walkableHeight   = agentHeight;
    dtParams.walkableRadius   = agentRadius;
    dtParams.walkableClimb    = agentMaxClimb;
    rcVcopy(dtParams.bmin, polyMesh->bmin);
    rcVcopy(dtParams.bmax, polyMesh->bmax);
    dtParams.cs          = recastCellSize;
    dtParams.ch          = recastCellHeight;
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
    const unsigned int walkableHeight = (int)ceilf(agentHeight / recastCellHeight);
    const unsigned int walkableClimb  = (int)floorf(agentMaxClimb / recastCellHeight);
    const unsigned int walkableRadius = (int)ceilf(agentRadius / recastCellSize);
    const unsigned int maxEdgeLen     = (int)(edgeMaxLen / recastCellSize);
    const unsigned int numTris        = numIndexes / 3;

    //
    // Calculate the grid size
    //
    rcCalcBounds(vertsBuffer, numVertices, minBounds, maxBounds);
    rcCalcGridSize(minBounds, maxBounds, recastCellSize, &gridSizeX, &gridSizeZ);

    rcHeightfield *heightfield = rcAllocHeightfield();

    //
    // Rasterize polygons
    //

    rcCreateHeightfield(
        &buildContext, *heightfield, gridSizeX, gridSizeZ, minBounds, maxBounds, recastCellSize, recastCellHeight
    );

    unsigned char *triAreas = new unsigned char[numTris];
    memset(triAreas, 0, sizeof(unsigned char) * numTris);

    rcMarkWalkableTriangles(&buildContext, agentMaxSlope, vertsBuffer, numVertices, indexesBuffer, numTris, triAreas);
    rcRasterizeTriangles(
        &buildContext, vertsBuffer, numVertices, indexesBuffer, triAreas, numTris, *heightfield, walkableClimb
    );

    delete[] triAreas;

    //
    // Filter walkable surfaces
    //

    //rcFilterLowHangingWalkableObstacles(&buildContext, walkableClimb, *heightfield);

    //rcFilterLedgeSpans(&buildContext, walkableHeight, walkableClimb, *heightfield);
    rcFilterWalkableLowHeightSpans(&buildContext, walkableHeight, *heightfield);

    // Partition walkable surfaces

    rcCompactHeightfield *compactedHeightfield = rcAllocCompactHeightfield();

    rcBuildCompactHeightfield(&buildContext, walkableHeight, walkableClimb, *heightfield, *compactedHeightfield);

    // The heightfield is not needed anymore
    rcFreeHeightField(heightfield);

    rcErodeWalkableArea(&buildContext, walkableRadius, *compactedHeightfield);

    rcBuildDistanceField(&buildContext, *compactedHeightfield);
    rcBuildRegions(&buildContext, *compactedHeightfield, 0, Square(regionMinSize), Square(regionMergeSize));

    //rcBuildRegionsMonotone(&buildContext, *compactedHeightfield, 0, Square(regionMinSize), Square(regionMergeSize));

    //
    // Simplify region contours
    //

    rcContourSet *contourSet = rcAllocContourSet();

    rcBuildContours(&buildContext, *compactedHeightfield, edgeMaxError, maxEdgeLen, *contourSet);

    //
    // Build polygon mesh from contours
    //

    rcPolyMesh *polyMesh = rcAllocPolyMesh();

    rcBuildPolyMesh(&buildContext, *contourSet, vertsPerPoly, *polyMesh);

    //
    // Create detail mesh to access approximate height for each polygon
    //

    rcPolyMeshDetail *polyMeshDetail = rcAllocPolyMeshDetail();

    rcBuildPolyMeshDetail(
        &buildContext,
        *polyMesh,
        *compactedHeightfield,
        recastCellSize * detailSampleDist,
        detailSampleMaxError,
        *polyMeshDetail
    );

    rcFreeCompactHeightfield(compactedHeightfield);
    rcFreeContourSet(contourSet);

    // Update poly flags from areas.
    for (int i = 0; i < polyMesh->npolys; ++i) {
        if (polyMesh->areas[i] == RC_WALKABLE_AREA) {
            polyMesh->flags[i] = 1;
        }
    }

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

    delete[] indexesBuffer;
    delete[] vertsBuffer;

    outPolyMesh       = polyMesh;
    outPolyMeshDetail = polyMeshDetail;
}

void G_Navigation_DrawModel(
    const Vector& origin, const navModel_t& model, float maxDistSquared, const Vector& offset = vec_zero
)
{
    int i, j;

    for (i = 1; i <= model.surfaces.NumObjects(); i++) {
        const navSurface_t& surface = model.surfaces.ObjectAt(i);

        for (j = 0; j < surface.indices.NumObjects(); j += 3) {
            const navVertice_t& v1 = surface.vertices[surface.indices[j + 0]];
            const navVertice_t& v2 = surface.vertices[surface.indices[j + 1]];
            const navVertice_t& v3 = surface.vertices[surface.indices[j + 2]];

            for (int k = 0; k < 3; ++k) {
                const Vector delta = (offset + surface.vertices[surface.indices[j + k]].xyz) - origin;

                if (delta.lengthSquared() >= maxDistSquared) {
                    continue;
                }

                G_DebugLine(
                    offset + surface.vertices[surface.indices[j + k]].xyz,
                    offset + surface.vertices[surface.indices[j + ((k + 1) % 3)]].xyz,
                    0,
                    1,
                    0,
                    1
                );
            }
        }
    }
}

/*
============
G_Navigation_DebugDraw
============
*/
void G_Navigation_DebugDraw()
{
    Entity           *ent            = g_entities[0].entity;
    dtNavMesh        *navMeshDt      = navigationMap.GetNavMesh();
    const navMap_t&   navigationData = navigationMap.GetNavigationData();
    const navModel_t& worldMap       = navigationData.GetWorldMap();
    int               i, j;

    if (!navMeshDt) {
        return;
    }

    if (!ent) {
        return;
    }

    if (ai_showallnode->integer) {
        for (int i = 0; i < navMeshDt->getMaxTiles(); i++) {
            const dtMeshTile *tile = static_cast<const dtNavMesh *>(navMeshDt)->getTile(i);
            if (!tile || !tile->header) {
                continue;
            }

            for (int j = 0; j < tile->header->vertCount; j++) {
                const float *pvert = &tile->verts[j * 3];

                Vector org;
                ConvertRecastToGameCoord(pvert, org);

                org.z += 16;

                if (org.z < ent->origin.z - 94 || org.z > ent->origin.z + 94) {
                    continue;
                }

                G_DebugBBox(org, Vector(-8, -8, -8), Vector(8, 8, 8), 1.0, 0.0, 0.5, 1.0);
            }
        }

#if 0
        for (int i = 0; i < polyMesh->nverts; ++i) {
            const unsigned short *v = &polyMesh->verts[i * 3];
            const float           x = polyMesh->bmin[0] + v[0] * polyMesh->cs;
            const float           y = polyMesh->bmin[1] + (v[1] + 1) * polyMesh->ch + 0.1f;
            const float           z = polyMesh->bmin[2] + v[2] * polyMesh->cs;

            Vector org;
            ConvertRecastToGameCoord(Vector(x, y, z), org);

            org.z += 16;

            if (org.z < ent->origin.z - 94 || org.z > ent->origin.z + 94) {
                continue;
            }

            G_DebugBBox(org, Vector(-8, -8, -8), Vector(8, 8, 8), 1.0, 0.0, 0.5, 1.0);
        }
#endif
    }

    switch (ai_showroutes->integer) {
    case 0:
    default:
        break;
    case 1:
        {
            const float maxDistSquared = Square(ai_showroutes_distance->integer);

            for (int i = 0; i < navMeshDt->getMaxTiles(); i++) {
                const dtMeshTile *tile = static_cast<const dtNavMesh *>(navMeshDt)->getTile(i);
                if (!tile || !tile->header) {
                    continue;
                }

                for (int j = 0; j < tile->header->polyCount; j++) {
                    const dtPoly *poly = &tile->polys[j];

                    for (int k = 0; k < poly->vertCount; ++k) {
                        const float *pv1 = &tile->verts[poly->verts[k] * 3];
                        const float *pv2 = &tile->verts[poly->verts[(k + 1) % poly->vertCount] * 3];

                        Vector v1, v2;
                        ConvertRecastToGameCoord(pv1, v1);
                        ConvertRecastToGameCoord(pv2, v2);

                        const Vector delta = v1 - ent->origin;

                        if (delta.lengthSquared() >= maxDistSquared) {
                            continue;
                        }

                        G_DebugLine(v1, v2, 0, 1, 0, 1);
                    }
                }

                /*
                for (int j = 0; j < tile->header->offMeshConCount; j++) {
                    const dtOffMeshConnection* offMeshCon = &tile->offMeshCons[j];
                    const float* pv1 = &offMeshCon->pos[0];
                    const float* pv2 = &offMeshCon->pos[3];

                    Vector v1, v2;
                    ConvertRecastToGameCoord(pv1, v1);
                    ConvertRecastToGameCoord(pv2, v2);

                    const Vector delta = v1 - ent->origin;

                    if (delta.lengthSquared() >= maxDistSquared) {
                        continue;
                    }

                    G_DebugLine(v1, v2, 0, 1, 1, 1);
                }
                */
            }
#if 0
        for (int i = 0; i < polyMesh->npolys; ++i) {
            const unsigned short *p    = &polyMesh->polys[i * polyMesh->nvp * 2];
            const unsigned char   area = polyMesh->areas[i];

            if (area != RC_WALKABLE_AREA) {
                continue;
            }

            unsigned short vi[3];
            for (int j = 2; j < polyMesh->nvp; ++j) {
                if (p[j] == RC_MESH_NULL_IDX) {
                    break;
                }
                vi[0] = p[0];
                vi[1] = p[j - 1];
                vi[2] = p[j];

                Vector vertices[3];

                for (int k = 0; k < 3; ++k) {
                    const unsigned short *v = &polyMesh->verts[vi[k] * 3];

                    vec3_t pos;
                    pos[0] = polyMesh->bmin[0] + v[0] * polyMesh->cs;
                    pos[1] = polyMesh->bmin[1] + (v[1] + 1) * polyMesh->ch;
                    pos[2] = polyMesh->bmin[2] + v[2] * polyMesh->cs;

                    ConvertRecastToGameCoord(pos, vertices[k]);
                }

                for (int k = 0; k < 3; ++k) {
                    const Vector delta = vertices[k] - ent->origin;

                    if (delta.lengthSquared() >= maxDistSquared) {
                        continue;
                    }

                    G_DebugLine(vertices[k], vertices[(k + 1) % 3], 0, 1, 0, 1);
                }
            }
        }
#endif
        }
        break;
    case 2:
        {
            const float maxDistSquared = Square(ai_showroutes_distance->integer);
            gentity_t  *edict;

            G_Navigation_DrawModel(ent->origin, worldMap, maxDistSquared);

            for (edict = active_edicts.next; edict != &active_edicts; edict = edict->next) {
                if (!edict->entity || edict->entity == world) {
                    continue;
                }

                if (edict->s.modelindex < 1 || edict->s.modelindex > navigationData.GetNumSubmodels()) {
                    continue;
                }

                const navModel_t& submodel = navigationData.GetSubmodel(edict->s.modelindex - 1);
                if (!submodel.surfaces.NumObjects()) {
                    // Could be a trigger
                    continue;
                }

                G_Navigation_DrawModel(ent->origin, submodel, maxDistSquared, edict->entity->origin);
            }
        }
        break;
    }

    if (ai_showpath->integer && navMeshDt) {
        switch (ai_showpath->integer) {
        default:
        case 0:
            ai_startpath = ai_endpath = vec_zero;
            break;
        case 1:
            ai_startpath = ent->origin;
            break;
        case 2:
            ai_endpath = ent->origin;
            break;
        case 3:
            if (ai_lastpath != ai_showpath->integer) {
                TestAgent(ai_startpath, ai_endpath, ai_pathlist, &ai_numPaths, ARRAY_LEN(ai_pathlist));
            }
            break;
        }

        ai_lastpath = ai_showpath->integer;

        navCrowd->update(level.frametime, NULL);

        for (int i = 0; i < ai_numPaths - 1; i++) {
            const Vector v1 = ai_pathlist[i] + Vector(0, 0, 64);
            const Vector v2 = ai_pathlist[i + 1] + Vector(0, 0, 64);

            G_DebugLine(v1, v2, 1.0, 0.0, 1.0, 1.0);
        }

#if 0
        if (navAgentId != -1) {
            const dtCrowdAgent* agent = navCrowd->getAgent(navAgentId);

            const dtPolyRef* path = agent->corridor.getPath();
            const int npath = agent->corridor.getPathCount();
            for (int i = 0; i < npath; ++i) {
                const dtMeshTile* tile;
                const dtPoly* poly;
                navMeshDt->getTileAndPolyByRef(path[i], &tile, &poly);

                const unsigned int tileId = (unsigned int)(poly - tile->polys);

                const dtPolyDetail* dm = &tile->detailMeshes[tileId];

                for (int j = 0; j < dm->triCount; ++j)
                {
                    const unsigned char* t = &tile->detailTris[(dm->triBase + i) * 4];
                    for (int k = 0; k < 3; ++k)
                    {
                        const float* pv1;
                        const float* pv2;

                        if (t[k] < poly->vertCount) {
                            pv1 = &tile->verts[poly->verts[t[k]] * 3];
                            pv2 = &tile->verts[poly->verts[t[(k + 1) % 3]] * 3];
                        } else {
                            pv1 = &tile->detailVerts[(dm->vertBase + t[k] - poly->vertCount) * 3];
                            pv2 = &tile->detailVerts[(dm->vertBase + t[(k + 1) % 3] - poly->vertCount) * 3];
                        }

                        Vector v1, v2;
                        ConvertRecastToGameCoord(pv1, v1);
                        ConvertRecastToGameCoord(pv2, v2);

                        G_DebugLine(v1, v2, 0.0, 1.0, 0.0, 1.0);
                    }
                }
            }
        }
#endif

#if 0
        const dtNodePool* pool = navMeshQuery->getNodePool();

        if (pool) {
            for (int i = 0; i < pool->getHashSize(); ++i)
            {
                for (dtNodeIndex j = pool->getFirst(i); j != DT_NULL_IDX; j = pool->getNext(j))
                {
                    const dtNode* node = pool->getNodeAtIdx(j + 1);
                    if (!node) continue;

                    Vector pos;
                    ConvertRecastToGameCoord(node->pos, pos);

                    G_DebugBBox(pos, Vector(-8, -8, -8), Vector(8, 8, 8), 0.0, 1.0, 0.0, 1.0);
                }
            }
        }
#endif
    }
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
{
    talloc          = new dtTileCacheAlloc();
    tcomp           = new NoCompressor();
    tmproc          = new MeshProcess();
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

    delete tmproc;
    delete tcomp;
    delete talloc;
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
void NavigationMap::Update()
{
    tileCache->update(level.frametime, navMeshDt);
}

/*
============
NavigationMap::ClearNavigation
============
*/
void NavigationMap::ClearNavigation()
{
    validNavigation = false;

    pathMaster.ClearNavigation();

    if (navMeshQuery) {
        dtFreeNavMeshQuery(navMeshQuery);
        navMeshQuery = NULL;
    }

    if (navMeshDt) {
        dtFreeNavMesh(navMeshDt);
        navMeshDt = NULL;
    }
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
    int               start;

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

        //if (edict->solid != SOLID_BSP) {
        //    continue;
        //}

        if (edict->solid == SOLID_TRIGGER) {
            continue;
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
    NavigationBSP      navigationBsp;
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

        navigationBsp.ProcessBSPForNavigation(mapname);
    } catch (const ScriptException& e) {
        gi.Printf("Failed to load BSP for navigation: %s\n", e.string.c_str());
        return;
    }

    end = gi.Milliseconds();

    gi.Printf("BSP file loaded and parsed in %.03f seconds\n", (float)((end - start) / 1000.0));

    //
    // Build and create the navigation mesh
    //

    InitializeNavMesh(buildContext, navigationBsp.navMap);

    gi.Printf("Building the navigation mesh...\n");

    try {
        start = gi.Milliseconds();

        gi.Printf("  Building the world mesh...\n");
        BuildWorldMesh(buildContext, navigationBsp.navMap);

        gi.Printf("  Building meshes for entities...\n");
        // FIXME: TODO
        //  Split everything into chunks and use a tile-based approach
        //BuildMeshesForEntities(buildContext, navigationBsp.navMap);

    } catch (const ScriptException& e) {
        gi.Printf("Couldn't build recast navigation mesh: %s\n", e.string.c_str());
        return;
    }

    pathMaster.PostLoadNavigation(*this);

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
