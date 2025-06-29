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
#include "../qcommon/container.h"
#include "../qcommon/vector.h"
#include "../qcommon/cm_polylib.h"

/**
 * @brief An index to a vertice in the navigation map.
 * 
 */
struct navIndice_t {
    int indice;

    navIndice_t() noexcept {}

    navIndice_t(int value) noexcept
        : indice(value)
    {
        assert(value >= 0);
    }

    operator int() const noexcept { return indice; };

    operator int *() noexcept { return &indice; };

    operator const int *() const noexcept { return &indice; };
};

/**
 * @brief Vertice in the navigation map.
 * 
 */
struct navVertice_t {
    Vector xyz;

    navVertice_t();
    navVertice_t(const vec3_t& inXyz);
    navVertice_t(const Vector& inXyz);
};

/**
 * @brief Navigation surface containing indices and vertices.
 * 
 */
struct navSurface_t {
    Container<navIndice_t>  indices;
    Container<navVertice_t> vertices;
    Vector                  bounds[2];

public:
    void AddVertice(const navVertice_t& vert);
    void AddIndex(navIndice_t index);
};

/**
 * @brief Navigation model containing faces.
 * 
 */
struct navModel_t {
    Container<navSurface_t> surfaces;
    Vector                  bounds[2];

public:
    void CalculateBounds();
};

/**
 * @brief Navigation map.
 * Contains indices and vertices renderer from LOD terrain, brushes and patches.
 *
 * Model 0 is the world map.
 */
struct navMap_t {
    navModel_t            worldMap;
    Container<navModel_t> subModels;

public:
    navModel_t&       GetWorldMap();
    const navModel_t& GetWorldMap() const;

    navModel_t&       CreateModel();
    int               GetNumSubmodels() const;
    const navModel_t& GetSubmodel(int index) const;
};

struct cshader_t {
    char shader[64];
    int  surfaceFlags;
    int  contentFlags;
    int  subdivisions;
};

struct cbrushside_t {
    cplane_t  *plane;
    int        planenum;
    int        shaderNum;
    int        surfaceFlags;
    winding_t *winding;

    dsideequation_t *pEq;

public:
    cbrushside_t();
    ~cbrushside_t();
};

struct cbrush_t {
    int           contents;
    vec3_t        bounds[2];
    int           numsides;
    int           shaderNum;
    cbrushside_t *sides;
};

struct cmodel_t {
    vec3_t mins;
    vec3_t maxs;
    int    firstSurface;
    int    numSurfaces;
    int    firstBrush;
    int    numBrushes;
};

struct cStaticModelUnpacked_t {
    char     model[128];
    vec3_t   origin;
    vec3_t   angles;
    vec3_t   axis[3];
    float    scale;
    dtiki_t *tiki;
};

///
/// Brush loading
///
qboolean IsTriangleDegenerate(const vec3_t *points, int a, int b, int c);
void     FanFaceSurface(navSurface_t& surface, const cbrushside_t& side);
qboolean CreateBrushWindings(const Container<cplane_t>& planes, cbrush_t& brush);
void     G_StripFaceSurface(navSurface_t& surface, const winding_t *winding);

//
// Utilities
//
qboolean FixWinding(winding_t *w);
qboolean G_PlaneFromPoints(vec4_t plane, vec3_t a, vec3_t b, vec3_t c);

///
/// Curve loading
///

static const unsigned int MAX_GRID_SIZE  = 129;
static const unsigned int MAX_PATCH_SIZE = 32;

struct surfaceGrid_t {
    // lod information, which may be different
    // than the culling information to allow for
    // groups of curves that LOD as a unit
    vec3_t lodOrigin;
    float  lodRadius;
    int    lodFixed;
    int    lodStitched;

    // vertexes
    int    width, height;
    float *widthLodError;
    float *heightLodError;
    int    numVertices;
    int    numIndexes;

    Vector *getVertices() { return (Vector *)((byte *)this + sizeof(surfaceGrid_t)); }

    const Vector *getVertices() const { return (Vector *)((byte *)this + sizeof(surfaceGrid_t)); }

    int *getIndices() { return (int *)((byte *)this + sizeof(surfaceGrid_t) + sizeof(Vector) * numVertices); }

    const int *getIndices() const
    {
        return (int *)((byte *)this + sizeof(surfaceGrid_t) + sizeof(Vector) * numVertices);
    }
};

surfaceGrid_t *G_SubdividePatchToGrid(int width, int height, Vector points[MAX_PATCH_SIZE * MAX_PATCH_SIZE], float subdivisions);
void           G_FreeSurfaceGridMesh(surfaceGrid_t *grid);

///
/// LOD Terrain loading
///

typedef union varnodeUnpacked_u {
    float fVariance;
    int   flags;
    /*
    struct {
#if !Q3_BIG_ENDIAN
		byte flags;
        unsigned char unused[3];
#else
        unsigned char unused[3];
        byte flags;
#endif
    } s;
	*/
} varnodeUnpacked_t;

#define TER_QUADS_PER_ROW  8
#define TER_TRIS_PER_PATCH (TER_QUADS_PER_ROW * TER_QUADS_PER_ROW * 2)
#define TER_PLANES_PER_TRI 5

struct terTriangle_t {
    cplane_t planes
        [TER_PLANES_PER_TRI]; // 0 is the surface plane, 3 border planes follow and a cap to give it some finite volume
};

struct terPatchCollide_t {
    vec3_t bounds[2];

    baseshader_t *shader;

    terTriangle_t tris[TER_TRIS_PER_PATCH];
};

struct terrainCollideSquare_t {
    vec4_t     plane[2];
    winding_t *w;
    int        eMode;

public:
    terrainCollideSquare_t();
    ~terrainCollideSquare_t();
};

struct terrainCollide_t {
    vec3_t                 vBounds[2];
    terrainCollideSquare_t squares[8][8];
};

// Use a 32-bit int because there can be more than 65536 tris
typedef unsigned int terraInt;

void     G_PrepareGenerateTerrainCollide(void);
void     G_GenerateTerrainCollide(cTerraPatch_t *patch, terrainCollide_t *tc);
qboolean G_CreateTerPatchWindings(terrainCollide_t& tc);

struct terrainVert_t {
    vec3_t   xyz;
    float    fVariance;
    float    fHgtAvg;
    float    fHgtAdd;
    terraInt nRef;
    terraInt iVertArray;
    byte    *pHgt;
    terraInt iNext;
    terraInt iPrev;
};

struct terraTri_t {
    terraInt                      iPt[3];
    terraInt                      nSplit;
    struct cTerraPatchUnpacked_t *patch;
    varnodeUnpacked_t            *varnode;
    terraInt                      index;
    byte                          lod;
    byte                          byConstChecks;
    terraInt                      iLeft;
    terraInt                      iRight;
    terraInt                      iBase;
    terraInt                      iLeftChild;
    terraInt                      iRightChild;
    terraInt                      iParent;
    terraInt                      iPrev;
    terraInt                      iNext;
};

struct surfaceTerrain_t {
    terraInt iVertHead;
    terraInt iTriHead;
    terraInt iTriTail;
    terraInt iMergeHead;
    int      nVerts;
    int      nTris;
};

struct cTerraPatchUnpacked_t {
    surfaceTerrain_t              drawinfo;
    float                         x0;
    float                         y0;
    float                         z0;
    float                         zmax;
    short int                     iNorth;
    short int                     iEast;
    short int                     iSouth;
    short int                     iWest;
    struct cTerraPatchUnpacked_t *pNextActive;
    varnodeUnpacked_t             varTree[2][63];
    unsigned char                 heightmap[81];
    byte                          flags;
};

struct poolInfo_t {
    terraInt iFreeHead;
    terraInt iCur;
    size_t   nFree;
};

extern terraTri_t    *g_pTris;
extern terrainVert_t *g_pVert;

extern poolInfo_t g_tri;
extern poolInfo_t g_vert;

void G_PreTessellateTerrain(cTerraPatchUnpacked_t *terraPatches, size_t numTerraPatches);
void G_DoTriSplitting(cTerraPatchUnpacked_t *terraPatches, size_t numTerraPatches);
void G_DoGeomorphs(cTerraPatchUnpacked_t *terraPatches, size_t numTerraPatches);
void G_TerrainFree();

///
/// Classes
///

class gameLump_c;
class bspMap_c;

/**
 * @brief Navigation generated by a BSP file.
 * 
 */
class NavigationBSP
{
public:
    /**
     * @brief Process the specified BSP map file and generate surfaces from it.
     * 
     * @param mapname The BSP map to use.
     *
     * Surfaces are stored the navMap member.
     */
    void ProcessBSPForNavigation(const char *mapname);

private:
    void LoadShaders(const gameLump_c& lump, Container<cshader_t>& shaders);
    bool IsValidContentsForPlayer(int contents);
    void LoadStaticModelDefs(const gameLump_c& lump);
    void LoadPlanes(const gameLump_c& lump, Container<cplane_t>& planes);
    void LoadBrushSides(
        const gameLump_c&           lump,
        const Container<cshader_t>& shaders,
        const Container<cplane_t>&  planes,
        Container<cbrushside_t>&    sides
    );
    void BoundBrush(cbrush_t *b);
    void LoadBrushes(
        const gameLump_c&              lump,
        const Container<cshader_t>&    shaders,
        const Container<cbrushside_t>& sides,
        Container<cbrush_t>&           brushes
    );
    void LoadLeafBrushes(const gameLump_c& lump, Container<int>& leafbrushes);
    void LoadSubmodels(const gameLump_c& lump, Container<cmodel_t>& submodels);
    void GenerateSideTriangles(navModel_t& model, const cbrush_t& brush, cbrushside_t& side);
    void GenerateBrushTriangles(navModel_t& model, const Container<cplane_t>& planes, cbrush_t& brush);
    void GenerateVerticesFromHull(bspMap_c& inBspMap, const Container<cshader_t>& shaders);
    void RenderSurfaceGrid(const surfaceGrid_t *grid, navSurface_t& outSurface);
    void ParseMesh(
        const dsurface_t *ds, const drawVert_t *verts, const Container<cshader_t>& shaders, navSurface_t& outSurface
    );
    void ParseTriSurf(const dsurface_t *ds, const drawVert_t *verts, const int *indexes);
    void ParseFace(const dsurface_t *ds, const drawVert_t *verts, const int *indexes);
    void ParseFlare(const dsurface_t *ds, const drawVert_t *verts);
    void LoadSurfaces(bspMap_c& inBspMap, const Container<cshader_t>& shaders);
    void UnpackTerraPatch(const cTerraPatch_t *pPacked, cTerraPatchUnpacked_t *pUnpacked);
    void SwapTerraPatch(cTerraPatch_t *pPatch);
    void GenerateTerrainPatchMesh(const cTerraPatchUnpacked_t& patch);
    void GenerateTerrainMesh(cTerraPatchUnpacked_t *terraPatches, size_t numTerraPatches);
    void LoadAndRenderTerrain(bspMap_c& inBspMap, const Container<cshader_t>& shaders);
    void CopyStaticModel(const cStaticModel_t *pSM, cStaticModelUnpacked_t *pUnpackedSM);

public:
    navMap_t navMap;
};