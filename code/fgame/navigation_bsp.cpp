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

#include "g_local.h"
#include "navigation_bsp.h"
#include "navigation_bsp_lump.h"
#include "../qcommon/qfiles.h"
#include "../qcommon/container.h"
#include "../qcommon/vector.h"
#include "../script/scriptexception.h"

/*
=================
navVertice_t::navVertice_t
=================
*/
navVertice_t::navVertice_t() {}

/*
=================
navVertice_t::navVertice_t
=================
*/
navVertice_t::navVertice_t(const vec3_t& inXyz)
    : xyz(inXyz)
{}

/*
=================
navVertice_t::navVertice_t
=================
*/
navVertice_t::navVertice_t(const Vector& inXyz)
    : xyz(inXyz)
{}

/*
=================
navMap_t::AddVertice
=================
*/
void navMap_t::AddVertice(const navVertice_t& vert)
{
    vertices.AddObject(vert);
    AddPointToBounds(vert.xyz, bounds[0], bounds[1]);
}

/*
=================
navMap_t::AddIndex
=================
*/
void navMap_t::AddIndex(navIndice_t index)
{
    if (index >= vertices.NumObjects()) {
        throw ScriptException("Attempt to add an index to a non-existent vertice (%d)", (int)index);
    }

    indices.AddObject(index);
}

/*
=================
G_LoadShaders
=================
*/
void G_LoadShaders(const gameLump_c& lump, Container<cshader_t>& shaders)
{
    dshader_t *in;
    int        i, count;

    in = (dshader_t *)lump.buffer;
    if (lump.length % sizeof(dshader_t)) {
        throw ScriptException("Invalid shader lump");
    }
    count = lump.length / sizeof(dshader_t);

    if (count < 1) {
        throw ScriptException("Map with no shaders");
    }

    shaders.Resize(count);

    for (i = 0; i < count; i++, in++) {
        cshader_t out;

        Q_strncpyz(out.shader, in->shader, sizeof(out.shader));
        out.contentFlags = LittleLong(in->contentFlags);
        out.surfaceFlags = LittleLong(in->surfaceFlags);

        shaders.AddObject(out);
    }
}

/*
=================
G_LoadPlanes
=================
*/
void G_LoadPlanes(const gameLump_c& lump, Container<cplane_t>& planes)
{
    int       i, j;
    dplane_t *in;
    int       count;
    int       bits;

    in = (dplane_t *)lump.buffer;
    if (lump.length % sizeof(*in)) {
        throw ScriptException("Invalid plane lump");
    }

    count = lump.length / sizeof(*in);

    if (count < 1) {
        throw ScriptException("Map with no planes");
    }

    planes.Resize(count);

    for (i = 0; i < count; i++, in++) {
        cplane_t out;

        bits = 0;
        for (j = 0; j < 3; j++) {
            out.normal[j] = LittleFloat(in->normal[j]);
            if (out.normal[j] < 0) {
                bits |= 1 << j;
            }
        }

        out.dist     = LittleFloat(in->dist);
        out.type     = PlaneTypeForNormal(out.normal);
        out.signbits = bits;

        planes.AddObject(out);
    }
}

/*
=================
CMod_LoadBrushSides
=================
*/
void G_LoadBrushSides(
    const gameLump_c&           lump,
    const Container<cshader_t>& shaders,
    const Container<cplane_t>&  planes,
    Container<cbrushside_t>&    sides
)
{
    int           i;
    dbrushside_t *in;
    int           count;
    int           num;

    in = (dbrushside_t *)lump.buffer;
    if (lump.length % sizeof(*in)) {
        throw ScriptException("Invalid plane lump");
    }

    count = lump.length / sizeof(*in);

    sides.Resize(count);

    for (i = 0; i < count; i++, in++) {
        cbrushside_t out;

        num              = LittleLong(in->planeNum);
        out.shaderNum    = LittleLong(in->shaderNum);
        out.plane        = &planes[num];
        out.planenum     = num;
        out.surfaceFlags = shaders.ObjectAt(out.shaderNum + 1).surfaceFlags;
        out.winding      = NULL;

        sides.AddObject(out);
    }
}

/*
=================
G_BoundBrush

=================
*/
void G_BoundBrush(cbrush_t *b)
{
    b->bounds[0][0] = -b->sides[0].plane->dist;
    b->bounds[1][0] = b->sides[1].plane->dist;

    b->bounds[0][1] = -b->sides[2].plane->dist;
    b->bounds[1][1] = b->sides[3].plane->dist;

    b->bounds[0][2] = -b->sides[4].plane->dist;
    b->bounds[1][2] = b->sides[5].plane->dist;
}

/*
=================
G_LoadBrushes
=================
*/
void G_LoadBrushes(
    const gameLump_c&              lump,
    const Container<cshader_t>&    shaders,
    const Container<cbrushside_t>& sides,
    Container<cbrush_t>&           brushes
)
{
    dbrush_t *in;
    int       i, count;

    in = (dbrush_t *)lump.buffer;
    if (lump.length % sizeof(*in)) {
        throw ScriptException("Invalid brush lump");
    }
    count = lump.length / sizeof(*in);

    brushes.Resize(count);

    for (i = 0; i < count; i++, in++) {
        cbrush_t out;

        out.sides    = &sides.ObjectAt(LittleLong(in->firstSide) + 1);
        out.numsides = LittleLong(in->numSides);

        out.shaderNum = LittleLong(in->shaderNum);
        if (out.shaderNum < 0 || out.shaderNum >= shaders.NumObjects()) {
            throw ScriptException("bad shaderNum: %i", out.shaderNum);
        }
        out.contents = shaders[out.shaderNum].contentFlags;

        G_BoundBrush(&out);

        brushes.AddObject(out);
    }
}

void G_LoadLeafBrushes(const gameLump_c& lump, Container<int>& leafbrushes)
{
    int *in;
    int  i, count;

    in = (int *)lump.buffer;
    if (lump.length % sizeof(*in)) {
        throw ScriptException("Invalid leaf brush lump");
    }
    count = lump.length / sizeof(*in);

    for (i = 0; i < count; i++, in++) {
        leafbrushes.AddObject(LittleLong(*in));
    }
}

void G_LoadSubmodels(const gameLump_c& lump, Container<cmodel_t>& submodels)
{
    dmodel_t *in;
    int       count;
    int       i, j;

    in = (dmodel_t *)lump.buffer;
    if (lump.length % sizeof(*in)) {
        throw ScriptException("Invalid submodels lump");
    }
    count = lump.length / sizeof(*in);

    if (count < 1) {
        Com_Error(ERR_DROP, "Map with no models");
    }

    submodels.Resize(count);

    for (i = 0; i < count; i++, in++) {
        cmodel_t out;

        for (j = 0; j < 3; j++) { // spread the mins / maxs by a pixel
            out.mins[j] = LittleFloat(in->mins[j]) - 1;
            out.maxs[j] = LittleFloat(in->maxs[j]) + 1;
        }

        out.numBrushes = LittleLong(in->numBrushes);
        if (out.numBrushes) {
            out.firstBrush = LittleLong(in->firstBrush);
        } else {
            out.firstBrush = -1;
        }

        out.numSurfaces = LittleLong(in->numSurfaces);
        if (out.numSurfaces) {
            out.firstSurface = LittleLong(in->firstSurface);
        } else {
            out.firstSurface = -1;
        }

        submodels.AddObject(out);
    }
}

#define MAX_INDEXES 1024

/*
============
G_GenerateSideTriangles
============
*/
void G_GenerateSideTriangles(navMap_t& navMap, cbrushside_t& side)
{
    int          i, r, least, rotate, ni;
    int          numIndexes;
    int          a, b, c;
    const vec_t *v1, *v2;
    int          indexes[MAX_INDEXES];
    size_t       baseVertex;

    if (!side.winding) {
        return;
    }

    if (side.surfaceFlags & SURF_SKY) {
        // Ignore sky surfaces
        return;
    }

    G_StripFaceSurface(navMap, side.winding);
}

/*
============
G_GenerateBrushTriangles
============
*/
void G_GenerateBrushTriangles(navMap_t& navMap, const Container<cplane_t>& planes, cbrush_t& brush)
{
    size_t i;
    size_t numSkip = 0;

    if (!(brush.contents & CONTENTS_SOLID) && !(brush.contents & CONTENTS_PLAYERCLIP) && !(brush.contents & CONTENTS_FENCE)) {
        return;
    }

    CreateBrushWindings(planes, brush);

    for (i = 0; i < brush.numsides; i++) {
        cbrushside_t& side = brush.sides[i];

        G_GenerateSideTriangles(navMap, side);
    }
}

/*
============
G_GenerateVerticesFromHull
============
*/
void G_GenerateVerticesFromHull(navMap_t& navMap, bspMap_c& inBspMap)
{
    Container<cshader_t>    shaders;
    Container<cplane_t>     planes;
    Container<cbrushside_t> sides;
    Container<cbrush_t>     brushes;
    Container<cmodel_t>     submodels;
    size_t                  i, j;
    size_t                  baseVertex;

    G_LoadShaders(inBspMap.LoadLump(LUMP_SHADERS), shaders);
    G_LoadPlanes(inBspMap.LoadLump(LUMP_PLANES), planes);
    G_LoadBrushSides(inBspMap.LoadLump(LUMP_BRUSHSIDES), shaders, planes, sides);
    G_LoadBrushes(inBspMap.LoadLump(LUMP_BRUSHES), shaders, sides, brushes);
    G_LoadSubmodels(inBspMap.LoadLump(LUMP_MODELS), submodels);

    const cmodel_t& worldModel = submodels.ObjectAt(1);

    for (i = worldModel.firstBrush + 1; i <= worldModel.firstBrush + worldModel.numBrushes; i++) {
        cbrush_t& brush = brushes.ObjectAt(i);

        G_GenerateBrushTriangles(navMap, planes, brush);
    }
}

/*
============
RenderSurfaceGrid
============
*/
static void RenderSurfaceGrid(navMap_t& navMap, const surfaceGrid_t *grid)
{
    int           i;
    int           baseVertex, baseIndex;
    const Vector *vertices;
    const int    *indexes;

    vertices   = grid->getVertices();
    indexes    = grid->getIndices();
    baseVertex = navMap.vertices.NumObjects();
    baseIndex  = navMap.indices.NumObjects();

    //navMap.vertices.Resize(baseVertex + grid->numVertices);
    //navMap.indices.Resize(baseIndex + grid->numIndexes);

    for (i = 0; i < grid->numVertices; i++) {
        navMap.AddVertice(vertices[i]);
    }

    for (i = 0; i < grid->numIndexes; i++) {
        navMap.AddIndex(baseVertex + indexes[i]);
    }
}

/*
============
ParseMesh
============
*/
static void ParseMesh(navMap_t& navMap, const dsurface_t *ds, const drawVert_t *verts)
{
    int            i, j;
    int            width, height, numPoints;
    Vector         points[MAX_PATCH_SIZE * MAX_PATCH_SIZE];
    surfaceGrid_t *grid;

    width  = LittleLong(ds->patchWidth);
    height = LittleLong(ds->patchHeight);
    verts += LittleLong(ds->firstVert);

    numPoints = width * height;
    for (i = 0; i < numPoints; i++) {
        for (j = 0; j < 3; j++) {
            points[i][j] = LittleFloat(verts[i].xyz[j]);
        }
    }

    grid = G_SubdividePatchToGrid(width, height, 1.0, points);

    // render the grid into vertices
    RenderSurfaceGrid(navMap, grid);

    // destroy the grid
    G_FreeSurfaceGridMesh(grid);
}

/*
============
ParseTriSurf
============
*/
static void ParseTriSurf(navMap_t& navMap, const dsurface_t *ds, const drawVert_t *verts, const int *indexes)
{
    int i, j;
    int numPoints, numIndexes;
    int baseVertex, baseIndex;

    numPoints  = LittleLong(ds->numVerts);
    numIndexes = LittleLong(ds->numIndexes);
    verts += LittleLong(ds->firstVert);
    indexes += LittleLong(ds->firstIndex);
    baseVertex = navMap.vertices.NumObjects();
    baseIndex  = navMap.indices.NumObjects();

    //navMap.vertices.Resize(navMap.vertices.NumObjects() + numPoints);

    for (i = 0; i < numPoints; i++) {
        Vector vec;

        for (j = 0; j < 3; j++) {
            vec[j] = LittleFloat(verts[i].xyz[j]);
        }

        navMap.AddVertice(vec);
    }

    //navMap.indices.Resize(navMap.indices.NumObjects() + numIndexes);

    for (i = 0; i < numIndexes; i++) {
        navMap.AddIndex(baseVertex + LittleLong(indexes[i]));
    }
}

/*
============
ParseFace
============
*/
static void ParseFace(navMap_t& navMap, const dsurface_t *ds, const drawVert_t *verts, const int *indexes)
{
    int i, j;
    int numPoints, numIndexes;
    int baseVertex, baseIndex;

    numPoints  = LittleLong(ds->numVerts);
    numIndexes = LittleLong(ds->numIndexes);
    verts += LittleLong(ds->firstVert);
    indexes += LittleLong(ds->firstIndex);
    baseVertex = navMap.vertices.NumObjects();
    baseIndex  = navMap.indices.NumObjects();

    //navMap.vertices.Resize(navMap.vertices.NumObjects() + numPoints);

    for (i = 0; i < numPoints; i++) {
        Vector vec;

        for (j = 0; j < 3; j++) {
            vec[j] = LittleFloat(verts[i].xyz[j]);
        }

        navMap.AddVertice(vec);
    }

    //navMap.indices.Resize(navMap.indices.NumObjects() + numIndexes);

    for (i = 0; i < numIndexes; i++) {
        navMap.AddIndex(baseVertex + LittleLong(indexes[i]));
    }
}

/*
============
ParseFlare
============
*/
static void ParseFlare(navMap_t& navMap, const dsurface_t *ds, const drawVert_t *verts)
{
    // Nothing to do
}

/*
============
G_LoadSurfaces
============
*/
static void G_LoadSurfaces(navMap_t& navMap, bspMap_c& inBspMap)
{
    const dsurface_t *in;
    const drawVert_t *dv;
    const int        *indexes;
    int               count;
    int               i;
    size_t            totalNumVerts;
    size_t            totalNumIndexes;
    size_t            numVerts;
    gameLump_c        surfs, verts, indexLump;

    // Gather vertices from meshes and surfaces
    surfs     = inBspMap.LoadLump(LUMP_SURFACES);
    verts     = inBspMap.LoadLump(LUMP_DRAWVERTS);
    indexLump = inBspMap.LoadLump(LUMP_DRAWINDEXES);

    if (surfs.length % sizeof(*in)) {
        throw ScriptException("LoadMap: funny lump size in %s", inBspMap.mapname);
    }

    count = surfs.length / sizeof(*in);

    dv = (const drawVert_t *)verts.buffer;
    if (verts.length % sizeof(*dv)) {
        throw ScriptException("LoadMap: funny lump size in %s", inBspMap.mapname);
    }

    indexes = (const int *)indexLump.buffer;
    if (indexLump.length % sizeof(*indexes)) {
        throw ScriptException("LoadMap: funny lump size in %s", inBspMap.mapname);
    }

    //
    // Calculate the number of vertices and indexes
    in = (dsurface_t *)surfs.buffer;

    totalNumVerts   = 0;
    totalNumIndexes = 0;

    for (i = 0; i < count; i++, in++) {
        switch (LittleLong(in->surfaceType)) {
        case MST_PATCH:
            numVerts = in->patchWidth * in->patchHeight * 9;
            totalNumVerts += numVerts;
            totalNumIndexes += numVerts * 4;
            break;
            //         case MST_TRIANGLE_SOUP:
            //             totalNumVerts += in->numVerts;
            //             totalNumIndexes += in->numIndexes;
            //             break;
            //         case MST_PLANAR:
            //             totalNumVerts += in->numVerts;
            //             totalNumIndexes += in->numIndexes;
            //             break;
        case MST_FLARE:
            break;
        default:
            break;
        }
    }

    navMap.vertices.Resize(navMap.vertices.NumObjects() + totalNumVerts);
    navMap.indices.Resize(navMap.indices.NumObjects() + totalNumIndexes);

    in = (dsurface_t *)surfs.buffer;

    for (i = 0; i < count; i++, in++) {
        switch (LittleLong(in->surfaceType)) {
        case MST_PATCH:
            ParseMesh(navMap, in, dv);
            break;
        case MST_TRIANGLE_SOUP:
            //ParseTriSurf(navMap, in, dv, indexes);
            break;
        case MST_PLANAR:
            //ParseFace(navMap, in, dv, indexes);
            break;
        case MST_FLARE:
            ParseFlare(navMap, in, dv);
            break;
        default:
            throw ScriptException("Bad surfaceType");
        }
    }
}

/*
================
R_UnpackTerraPatch
================
*/
void G_UnpackTerraPatch(const cTerraPatch_t *pPacked, cTerraPatchUnpacked_t *pUnpacked)
{
    int i;

    if (pPacked->lmapScale <= 0) {
        throw ScriptException("invalid map: terrain has lmapScale <= 0");
    }

    pUnpacked->x0     = ((int)pPacked->x << 6);
    pUnpacked->y0     = ((int)pPacked->y << 6);
    pUnpacked->z0     = pPacked->iBaseHeight;
    pUnpacked->iNorth = pPacked->iNorth;
    pUnpacked->iEast  = pPacked->iEast;
    pUnpacked->iSouth = pPacked->iSouth;
    pUnpacked->iWest  = pPacked->iWest;

    for (i = 0; i < MAX_TERRAIN_VARNODES; i++) {
        const varnode_t *packedVarTree;

        packedVarTree                      = &pPacked->varTree[0][i];
        pUnpacked->varTree[0][i].fVariance = packedVarTree->flags & 0x7FF;
        pUnpacked->varTree[0][i].flags &= ~0xFF;
        pUnpacked->varTree[0][i].flags |= (packedVarTree->flags >> 12) & 0xFF;

        packedVarTree                      = &pPacked->varTree[1][i];
        pUnpacked->varTree[1][i].fVariance = packedVarTree->flags & 0x7FF;
        pUnpacked->varTree[1][i].flags &= ~0xFF;
        pUnpacked->varTree[1][i].flags |= (packedVarTree->flags >> 12) & 0xFF;
    }

    for (i = 0; i < ARRAY_LEN(pUnpacked->heightmap); i++) {
        pUnpacked->heightmap[i] = pPacked->heightmap[i];
    }

    pUnpacked->zmax  = 0;
    pUnpacked->flags = pPacked->flags;

    for (i = 0; i < ARRAY_LEN(pUnpacked->heightmap); i++) {
        if (pUnpacked->zmax < pUnpacked->heightmap[i]) {
            pUnpacked->zmax = pUnpacked->heightmap[i];
        }
    }

    pUnpacked->zmax += pUnpacked->zmax;
}

/*
====================
G_SwapTerraPatch

Swaps the patch on big-endian
====================
*/
void G_SwapTerraPatch(cTerraPatch_t *pPatch)
{
#ifdef Q3_BIG_ENDIAN
    int i;

    pPatch->iBaseHeight = LittleShort(pPatch->iBaseHeight);
    pPatch->iNorth      = LittleShort(pPatch->iNorth);
    pPatch->iEast       = LittleShort(pPatch->iEast);
    pPatch->iSouth      = LittleShort(pPatch->iSouth);
    pPatch->iWest       = LittleShort(pPatch->iWest);

    for (i = 0; i < MAX_TERRAIN_VARNODES; i++) {
        pPatch->varTree[0][i].flags = LittleUnsignedShort(pPatch->varTree[0][i].flags);
        pPatch->varTree[1][i].flags = LittleUnsignedShort(pPatch->varTree[1][i].flags);
    }
#endif
}

/*
================
G_RenderPatch
================
*/
void G_RenderPatch(navMap_t& navMap, const cTerraPatchUnpacked_t& patch)
{
    terraInt vertNum;
    terraInt triNum;
    terraInt currentVertice = 0;
    size_t   baseVertice    = navMap.vertices.NumObjects();

    for (vertNum = patch.drawinfo.iVertHead; vertNum; vertNum = g_pVert[vertNum].iNext) {
        terrainVert_t& vert = g_pVert[vertNum];

        navMap.AddVertice(vert.xyz);
        vert.iVertArray = currentVertice;

        currentVertice++;
    }

    for (triNum = patch.drawinfo.iTriHead; triNum; triNum = g_pTris[triNum].iNext) {
        const terraTri_t& tri = g_pTris[triNum];

        if (tri.byConstChecks & 4) {
            navMap.AddIndex(baseVertice + g_pVert[tri.iPt[0]].iVertArray);
            navMap.AddIndex(baseVertice + g_pVert[tri.iPt[1]].iVertArray);
            navMap.AddIndex(baseVertice + g_pVert[tri.iPt[2]].iVertArray);
        }
    }
}

/*
================
G_RenderTerrainTris
================
*/
void G_RenderTerrainTris(navMap_t& navMap, cTerraPatchUnpacked_t *terraPatches, size_t numTerraPatches)
{
    size_t   i;
    size_t   numVertices;
    size_t   numIndices;
    terraInt triNum;

    if (!numTerraPatches) {
        return;
    }

    G_PreTessellateTerrain(terraPatches, numTerraPatches);

    G_DoTriSplitting(terraPatches, numTerraPatches);

    // Calculate the number of required tris and vertices
    numVertices = 0;
    numIndices  = 0;
    for (i = 0; i < numTerraPatches; i++) {
        const cTerraPatchUnpacked_t& patch = terraPatches[i];

        numVertices += patch.drawinfo.nVerts;

        for (triNum = patch.drawinfo.iTriHead; triNum; triNum = g_pTris[triNum].iNext) {
            if (g_pTris[triNum].byConstChecks & 4) {
                numIndices += 3;
            }
        }
    }

    navMap.vertices.Resize(navMap.vertices.NumObjects() + numVertices);
    navMap.indices.Resize(navMap.indices.NumObjects() + numIndices);

    for (i = 0; i < numTerraPatches; i++) {
        const cTerraPatchUnpacked_t& patch = terraPatches[i];

        G_RenderPatch(navMap, patch);
    }

    G_TerrainFree();
}

/*
================
G_LoadAndRenderTerrain
================
*/
void G_LoadAndRenderTerrain(navMap_t& navMap, bspMap_c& inBspMap)
{
    int                    i;
    cTerraPatch_t         *in;
    cTerraPatchUnpacked_t *out;
    size_t                 numTerraPatches;
    cTerraPatchUnpacked_t *terraPatches;

    const gameLump_c lump = inBspMap.LoadLump(LUMP_TERRAIN);

    if (!lump.length) {
        return;
    }

    if (lump.length % sizeof(cTerraPatch_t)) {
        Com_Error(ERR_DROP, "R_LoadTerrain: funny lump size");
    }

    numTerraPatches = lump.length / sizeof(cTerraPatch_t);
    terraPatches    = (cTerraPatchUnpacked_t *)gi.Malloc(numTerraPatches * sizeof(cTerraPatchUnpacked_t));

    in  = (cTerraPatch_t *)lump.buffer;
    out = terraPatches;

    for (i = 0; i < numTerraPatches; in++, out++, i++) {
        G_SwapTerraPatch(in);
        G_UnpackTerraPatch(in, out);
    }

    G_RenderTerrainTris(navMap, terraPatches, numTerraPatches);

    gi.Free(terraPatches);
}

/*
================
G_CopyStaticModel
================
*/
void G_CopyStaticModel(const cStaticModel_t *pSM, cStaticModelUnpacked_t *pUnpackedSM)
{
    pUnpackedSM->angles[0] = LittleFloat(pSM->angles[0]);
    pUnpackedSM->angles[1] = LittleFloat(pSM->angles[1]);
    pUnpackedSM->angles[2] = LittleFloat(pSM->angles[2]);
    pUnpackedSM->origin[0] = LittleFloat(pSM->origin[0]);
    pUnpackedSM->origin[1] = LittleFloat(pSM->origin[1]);
    pUnpackedSM->origin[2] = LittleFloat(pSM->origin[2]);
    pUnpackedSM->scale     = LittleFloat(pSM->scale);
    memcpy(pUnpackedSM->model, pSM->model, sizeof(pUnpackedSM->model));
}

/*
================
G_LoadStaticModelDefs
================
*/
void G_LoadStaticModelDefs(const gameLump_c& lump)
{
    int                     i;
    const cStaticModel_t   *in;
    cStaticModelUnpacked_t *out;
    size_t                  numStaticModels;
    cStaticModelUnpacked_t *staticModels;

    if (!lump.length) {
        return;
    }

    if (lump.length % sizeof(cStaticModel_t)) {
        throw ScriptException("G_LoadStaticModelDefs: funny lump size");
    }

    numStaticModels = lump.length / sizeof(cStaticModel_t);
    staticModels    = (cStaticModelUnpacked_t *)gi.Malloc(numStaticModels * sizeof(cStaticModelUnpacked_t));

    in  = (const cStaticModel_t *)lump.buffer;
    out = (cStaticModelUnpacked_t *)staticModels;

    for (i = 0; i < numStaticModels; in++, out++, i++) {
        G_CopyStaticModel(in, out);
    }
}

/*
============
G_Navigation_ProcessBSPForNavigation
============
*/
void G_Navigation_ProcessBSPForNavigation(const char *mapname, navMap_t& outNavigationMap)
{
    int       length;
    int       i;
    qhandle_t handle;

    // load it
    length = gi.FS_FOpenFile(mapname, &handle, qtrue, qtrue);
    if (length <= 0) {
        return;
    }

    bspMap_c bspMap(mapname, handle, length);

    gi.FS_Read(&bspMap.header, sizeof(dheader_t), bspMap.h);

    for (i = 0; i < sizeof(dheader_t) / 4; i++) {
        ((int *)&bspMap.header)[i] = LittleLong(((int *)&bspMap.header)[i]);
    }

    //
    // Create all vertices from brushes
    //

    G_GenerateVerticesFromHull(outNavigationMap, bspMap);

    //
    // Load patches
    //

    G_LoadSurfaces(outNavigationMap, bspMap);

    //
    // Render the whole map terrain
    //

    G_LoadAndRenderTerrain(outNavigationMap, bspMap);
}
