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

struct navIndice_t {
    int indice;

    navIndice_t() noexcept {}
    navIndice_t(int value) noexcept
        : indice(value)
    {
        assert(value >= 0);
    }

    operator int() const noexcept { return indice; };
    operator int* () noexcept { return &indice; };
    operator const int*() const noexcept { return &indice; };
};

struct navVertice_t {
    Vector xyz;

    navVertice_t();
    navVertice_t(const vec3_t& inXyz);
    navVertice_t(const Vector& inXyz);
};

struct navMap_t {
    Container<navIndice_t>    indices;
    Container<navVertice_t>   vertices;
    Vector bounds[2];

public:
    void AddVertice(const navVertice_t& vert);
    void AddIndex(navIndice_t index);
};

struct cshader_t {
    char shader[64];
    int  surfaceFlags;
    int  contentFlags;
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
    int firstSurface;
    int numSurfaces;
    int firstBrush;
    int numBrushes;
};

struct cStaticModelUnpacked_t {
    char     model[128];
    vec3_t   origin;
    vec3_t   angles;
    vec3_t   axis[3];
    float    scale;
    dtiki_t *tiki;
};

void G_Navigation_ProcessBSPForNavigation(const char* mapname, navMap_t& outNavigationMap);

//
// Brush loading
//
qboolean IsTriangleDegenerate(const vec3_t* points, int a, int b, int c);
void FanFaceSurface(navMap_t& navMap, const cbrushside_t& side);
qboolean CreateBrushWindings(const Container<cplane_t>& planes, cbrush_t& brush);
void G_StripFaceSurface(navMap_t& navMap, const winding_t* winding);

//
// Utilities
//
qboolean FixWinding(winding_t* w);

//
// Curve loading
//

static const unsigned int MAX_GRID_SIZE = 129;
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

surfaceGrid_t* G_SubdividePatchToGrid(int width, int height, float subdivide, Vector points[MAX_PATCH_SIZE * MAX_PATCH_SIZE]);
void G_FreeSurfaceGridMesh(surfaceGrid_t* grid);

//
// LOD Terrain loading
//

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

// Use a 32-bit int because there can be more than 65536 tris
typedef unsigned int terraInt;

struct terrainVert_t {
    vec3_t       xyz;
    vec2_t       texCoords[2];
    float        fVariance;
    float        fHgtAvg;
    float        fHgtAdd;
    unsigned int uiDistRecalc;
    terraInt     nRef;
    terraInt     iVertArray;
    byte        *pHgt;
    terraInt     iNext;
    terraInt     iPrev;
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

extern terraTri_t* g_pTris;
extern terrainVert_t* g_pVert;

extern poolInfo_t g_tri;
extern poolInfo_t g_vert;

void G_PreTessellateTerrain(cTerraPatchUnpacked_t* terraPatches, size_t numTerraPatches);
void G_DoTriSplitting(cTerraPatchUnpacked_t* terraPatches, size_t numTerraPatches);
void G_DoGeomorphs(cTerraPatchUnpacked_t* terraPatches, size_t numTerraPatches);
void G_TerrainFree();
