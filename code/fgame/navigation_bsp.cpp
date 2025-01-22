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
#include "../qcommon/qfiles.h"
#include "../qcommon/container.h"
#include "../qcommon/vector.h"
#include "../qcommon/cm_polylib.h"
#include "../script/scriptexception.h"

#define MAX_GRID_SIZE  129
#define MAX_PATCH_SIZE 32

// to allow boxes to be treated as brush models, we allocate
// some extra indexes along with those needed by the map
#define BOX_BRUSHES 1
#define BOX_SIDES   6
#define BOX_LEAFS   2
#define BOX_PLANES  12

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

class gameLump_c
{
public:
    gameLump_c()
        : buffer(NULL)
        , length(0)
    {}

    gameLump_c(void *inBuffer, int inLength)
        : buffer(inBuffer)
        , length(inLength)
    {}

    gameLump_c(const gameLump_c&)            = delete;
    gameLump_c& operator=(const gameLump_c&) = delete;
    gameLump_c(gameLump_c&& other) noexcept;
    gameLump_c& operator=(gameLump_c&& other) noexcept;
    ~gameLump_c();

    static gameLump_c LoadLump(fileHandle_t handle, const lump_t& lump);
    void              FreeLump();

public:
    void *buffer;
    int   length;
};

typedef struct {
    char shader[64];
    int  surfaceFlags;
    int  contentFlags;
} cshader_t;

typedef struct {
    cplane_t  *plane;
    int        planenum;
    int        surfaceFlags;
    winding_t *winding;

    dsideequation_t *pEq;
} cbrushside_t;

typedef struct {
    int           contents;
    vec3_t        bounds[2];
    int           numsides;
    cbrushside_t *sides;
} cbrush_t;

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

typedef struct terrainVert_s {
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
} terrainVert_t;

typedef struct terraTri_s {
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
} terraTri_t;

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

struct cStaticModelUnpacked_t {
    char     model[128];
    vec3_t   origin;
    vec3_t   angles;
    vec3_t   axis[3];
    float    scale;
    dtiki_t *tiki;
};

static const unsigned int MAX_TERRAIN_LOD = 6;

static size_t g_nTris;
static size_t g_nVerts;
static int    g_nMerge;
static int    g_nSplit;

typedef struct poolInfo_s {
    terraInt iFreeHead;
    terraInt iCur;
    size_t   nFree;
} poolInfo_t;

static int modeTable[8];

terraTri_t    *g_pTris;
terrainVert_t *g_pVert;

poolInfo_t g_tri;
poolInfo_t g_vert;

/*
============
gameLump_c::gameLump_c
============
*/
gameLump_c::gameLump_c(gameLump_c&& other) noexcept
{
    FreeLump();

    buffer       = other.buffer;
    length       = other.length;
    other.buffer = NULL;
}

/*
============
gameLump_c::operator=
============
*/
gameLump_c& gameLump_c::operator=(gameLump_c&& other) noexcept
{
    FreeLump();

    buffer       = other.buffer;
    length       = other.length;
    other.buffer = NULL;

    return *this;
}

/*
============
gameLump_c::~gameLump_c
============
*/
gameLump_c::~gameLump_c()
{
    FreeLump();
}

/*
============
gameLump_c::LoadLump
============
*/
gameLump_c gameLump_c::LoadLump(fileHandle_t handle, const lump_t& lump)
{
    if (lump.filelen) {
        void *buffer = gi.Malloc(lump.filelen);

        if (gi.FS_Seek(handle, lump.fileofs, FS_SEEK_SET) < 0) {
            throw ScriptException("CM_LoadLump: Error seeking to lump.");
        }

        gi.FS_Read(buffer, lump.filelen, handle);

        return gameLump_c(buffer, lump.filelen);
    }

    return gameLump_c();
}

/*
============
gameLump_c::FreeLump
============
*/
void gameLump_c::FreeLump()
{
    if (buffer) {
        gi.Free(buffer);
        buffer = NULL;
        length = 0;
    }
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

    planes.Resize(BOX_PLANES + count);

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
    int           shaderNum;

    in = (dbrushside_t *)lump.buffer;
    if (lump.length % sizeof(*in)) {
        throw ScriptException("Invalid plane lump");
    }

    count = lump.length / sizeof(*in);

    sides.Resize(BOX_SIDES + count);

    for (i = 0; i < count; i++, in++) {
        cbrushside_t out;

        num              = LittleLong(in->planeNum);
        shaderNum        = LittleLong(in->shaderNum);
        out.plane        = &planes[num];
        out.planenum     = num;
        out.surfaceFlags = shaders.ObjectAt(shaderNum + 1).surfaceFlags;

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
    int       shaderNum;

    in = (dbrush_t *)lump.buffer;
    if (lump.length % sizeof(*in)) {
        throw ScriptException("Invalid brush lump");
    }
    count = lump.length / sizeof(*in);

    brushes.Resize(BOX_BRUSHES + count);

    for (i = 0; i < count; i++, in++) {
        cbrush_t out;

        out.sides    = &sides.ObjectAt(LittleLong(in->firstSide) + 1);
        out.numsides = LittleLong(in->numSides);

        shaderNum = LittleLong(in->shaderNum);
        if (shaderNum < 0 || shaderNum >= shaders.NumObjects()) {
            throw ScriptException("bad shaderNum: %i", shaderNum);
        }
        out.contents = shaders[shaderNum].contentFlags;

        G_BoundBrush(&out);

        brushes.AddObject(out);
    }
}

#define SNAP_EPSILON 0.01

void SnapWeldVector(vec3_t a, vec3_t b, vec3_t out)
{
    int   i;
    vec_t ai, bi, outi;

    /* dummy check */
    if (a == NULL || b == NULL || out == NULL) {
        return;
    }

    /* do each element */
    for (i = 0; i < 3; i++) {
        /* round to integer */
        ai = Q_rint(a[i]);
        bi = Q_rint(a[i]);

        /* prefer exact integer */
        if (ai == a[i]) {
            out[i] = a[i];
        } else if (bi == b[i]) {
            out[i] = b[i];
        }

        /* use nearest */
        else if (fabs(ai - a[i]) < fabs(bi < b[i])) {
            out[i] = a[i];
        } else {
            out[i] = b[i];
        }

        /* snap */
        outi = Q_rint(out[i]);
        if (fabs(outi - out[i]) <= SNAP_EPSILON) {
            out[i] = outi;
        }
    }
}

#define DEGENERATE_EPSILON 0.1

qboolean FixWinding(winding_t *w)
{
    qboolean valid = qtrue;
    int      i, j, k;
    vec3_t   vec;
    float    dist;

    /* dummy check */
    if (!w) {
        return qfalse;
    }

    /* check all verts */
    for (i = 0; i < w->numpoints; i++) {
        /* don't remove points if winding is a triangle */
        if (w->numpoints == 3) {
            return valid;
        }

        /* get second point index */
        j = (i + 1) % w->numpoints;

        /* degenerate edge? */
        VectorSubtract(w->p[i], w->p[j], vec);
        dist = VectorLength(vec);
        if (dist < DEGENERATE_EPSILON) {
            valid = qfalse;
            //Sys_FPrintf( SYS_VRB, "WARNING: Degenerate winding edge found, fixing...\n" );

            /* create an average point (ydnar 2002-01-26: using nearest-integer weld preference) */
            SnapWeldVector(w->p[i], w->p[j], vec);
            VectorCopy(vec, w->p[i]);
            //VectorAdd( w->p[ i ], w->p[ j ], vec );
            //VectorScale( vec, 0.5, w->p[ i ] );

            /* move the remaining verts */
            for (k = i + 2; k < w->numpoints; k++) {
                VectorCopy(w->p[k], w->p[k - 1]);
            }
            w->numpoints--;
        }
    }

    /* one last check and return */
    if (w->numpoints < 3) {
        valid = qfalse;
    }
    return valid;
}

qboolean BoundBrush(cbrush_t& brush)
{
    int        i, j;
    winding_t *w;

    ClearBounds(brush.bounds[0], brush.bounds[1]);
    for (i = 0; i < brush.numsides; i++) {
        w = brush.sides[i].winding;
        if (w == NULL) {
            continue;
        }
        for (j = 0; j < w->numpoints; j++) {
            AddPointToBounds(w->p[j], brush.bounds[0], brush.bounds[1]);
        }
    }

    for (i = 0; i < 3; i++) {
        if (brush.bounds[0][i] < MIN_WORLD_COORD || brush.bounds[1][i] > MAX_WORLD_COORD
            || brush.bounds[0][i] >= brush.bounds[1][i]) {
            return qfalse;
        }
    }

    return qtrue;
}

/*
=============
AllocWinding
=============
*/
winding_t *AllocWinding(int points)
{
    winding_t *w;
    int        s;

    s = sizeof(vec_t) * 3 * points + sizeof(int);
    w = (winding_t *)gi.Malloc(s);
    Com_Memset(w, 0, s);
    return w;
}

void FreeWinding(winding_t *w)
{
    if (*(unsigned *)w == 0xdeaddead) {
        Com_Error(ERR_FATAL, "FreeWinding: freed a freed winding");
    }
    *(unsigned *)w = 0xdeaddead;

    gi.Free(w);
}

/*
=================
BaseWindingForPlane
=================
*/
winding_t *BaseWindingForPlane(vec3_t normal, vec_t dist)
{
    int        i, x;
    vec_t      max, v;
    vec3_t     org, vright, vup;
    winding_t *w;

    // find the major axis

    max = -MAP_SIZE;
    x   = -1;
    for (i = 0; i < 3; i++) {
        v = fabs(normal[i]);
        if (v > max) {
            x   = i;
            max = v;
        }
    }
    if (x == -1) {
        Com_Error(ERR_DROP, "BaseWindingForPlane: no axis found");
    }

    VectorCopy(vec3_origin, vup);
    switch (x) {
    case 0:
    case 1:
        vup[2] = 1;
        break;
    case 2:
        vup[0] = 1;
        break;
    }

    v = DotProduct(vup, normal);
    VectorMA(vup, -v, normal, vup);
    VectorNormalize(vup);

    VectorScale(normal, dist, org);

    CrossProduct(vup, normal, vright);

    VectorScale(vup, MAP_SIZE, vup);
    VectorScale(vright, MAP_SIZE, vright);

    // project a really big	axis aligned box onto the plane
    w = AllocWinding(4);

    VectorSubtract(org, vright, w->p[0]);
    VectorAdd(w->p[0], vup, w->p[0]);

    VectorAdd(org, vright, w->p[1]);
    VectorAdd(w->p[1], vup, w->p[1]);

    VectorAdd(org, vright, w->p[2]);
    VectorSubtract(w->p[2], vup, w->p[2]);

    VectorSubtract(org, vright, w->p[3]);
    VectorSubtract(w->p[3], vup, w->p[3]);

    w->numpoints = 4;

    return w;
}

/*
=============
ChopWindingInPlace
=============
*/
void ChopWindingInPlace(winding_t **inout, vec3_t normal, vec_t dist, vec_t epsilon)
{
    winding_t   *in;
    vec_t        dists[MAX_POINTS_ON_WINDING + 4];
    int          sides[MAX_POINTS_ON_WINDING + 4];
    int          counts[3];
    static vec_t dot; // VC 4.2 optimizer bug if not static
    int          i, j;
    vec_t       *p1, *p2;
    vec3_t       mid;
    winding_t   *f;
    int          maxpts;

    in        = *inout;
    counts[0] = counts[1] = counts[2] = 0;

    // determine sides for each point
    for (i = 0; i < in->numpoints; i++) {
        dot = DotProduct(in->p[i], normal);
        dot -= dist;
        dists[i] = dot;
        if ((dot + 0.0005) > epsilon) {
            sides[i] = SIDE_FRONT;
        } else if ((dot - 0.0005) < -epsilon) {
            sides[i] = SIDE_BACK;
        } else {
            sides[i] = SIDE_ON;
        }
        counts[sides[i]]++;
    }
    sides[i] = sides[0];
    dists[i] = dists[0];

    if (!counts[0]) {
        FreeWinding(in);
        *inout = NULL;
        return;
    }
    if (!counts[1]) {
        return; // inout stays the same
    }

    maxpts = in->numpoints + 4; // cant use counts[0]+2 because
    // of fp grouping errors

    f = AllocWinding(maxpts);

    for (i = 0; i < in->numpoints; i++) {
        p1 = in->p[i];

        if (sides[i] == SIDE_ON) {
            VectorCopy(p1, f->p[f->numpoints]);
            f->numpoints++;
            continue;
        }

        if (sides[i] == SIDE_FRONT) {
            VectorCopy(p1, f->p[f->numpoints]);
            f->numpoints++;
        }

        if (sides[i + 1] == SIDE_ON || sides[i + 1] == sides[i]) {
            continue;
        }

        // generate a split point
        p2 = in->p[(i + 1) % in->numpoints];

        dot = dists[i] / (dists[i] - dists[i + 1]);
        for (j = 0; j < 3; j++) { // avoid round off error when possible
            if (normal[j] == 1) {
                mid[j] = dist;
            } else if (normal[j] == -1) {
                mid[j] = -dist;
            } else {
                mid[j] = p1[j] + dot * (p2[j] - p1[j]);
            }
        }

        VectorCopy(mid, f->p[f->numpoints]);
        f->numpoints++;
    }

    if (f->numpoints > maxpts) {
        Com_Error(ERR_DROP, "ClipWinding: points exceeded estimate");
    }
    if (f->numpoints > MAX_POINTS_ON_WINDING) {
        Com_Error(ERR_DROP, "ClipWinding: MAX_POINTS_ON_WINDING");
    }

    FreeWinding(in);
    *inout = f;
}

qboolean CreateBrushWindings(const Container<cplane_t>& planes, cbrush_t& brush)
{
    int           i, j;
    winding_t    *w;
    cbrushside_t *side;
    cplane_t     *plane;

    /* walk the list of brush sides */
    for (i = 0; i < brush.numsides; i++) {
        /* get side and plane */
        side  = &brush.sides[i];
        plane = side->plane;

        /* make huge winding */
        w = BaseWindingForPlane(plane->normal, plane->dist);

        /* walk the list of brush sides */
        for (j = 0; j < brush.numsides && w != NULL; j++) {
            if (i == j) {
                continue;
            }
            if (brush.sides[j].planenum == (brush.sides[i].planenum ^ 1)) {
                continue; /* back side clipaway */
            }
            plane = (cplane_t *)&planes[brush.sides[j].planenum ^ 1];
            ChopWindingInPlace(&w, plane->normal, plane->dist, 0); // CLIP_EPSILON );

            FixWinding(w);
        }

        /* set side winding */
        side->winding = w;
    }

    /* find brush bounds */
    return BoundBrush(brush);
}

#define COLINEAR_AREA 10

static qboolean IsTriangleDegenerate(const vec3_t *points, int a, int b, int c)
{
    vec3_t v1, v2, v3;
    float  d;

    VectorSubtract(points[b], points[a], v1);
    VectorSubtract(points[c], points[a], v2);
    CrossProduct(v1, v2, v3);
    d = VectorLength(v3);

    // assume all very small or backwards triangles will cause problems
    if (d < COLINEAR_AREA) {
        return qtrue;
    }

    return qfalse;
}

#define MAX_INDEXES 1024

/*
============
G_GenerateVerticesFromHull
============
*/
void G_GenerateVerticesFromHull(
    navMap_t&         navMap,
    const gameLump_c& shadersLump,
    const gameLump_c& planesLump,
    const gameLump_c& brushSidesLump,
    const gameLump_c& brushesLump
)
{
    Container<cshader_t>    shaders;
    Container<cplane_t>     planes;
    Container<cbrushside_t> sides;
    Container<cbrush_t>     brushes;
    size_t                  i, j, k;
    size_t                  baseVertex;

    G_LoadShaders(shadersLump, shaders);
    G_LoadPlanes(planesLump, planes);
    G_LoadBrushSides(brushSidesLump, shaders, planes, sides);
    G_LoadBrushes(brushesLump, shaders, sides, brushes);

    for (i = 1; i <= brushes.NumObjects(); i++) {
        cbrush_t& brush = brushes.ObjectAt(i);
        CreateBrushWindings(planes, brush);

        for (j = 0; j < brush.numsides; j++) {
            cbrushside_t& side = brush.sides[j];
            int           indexes[MAX_INDEXES];

            if (!side.winding) {
                continue;
            }

            baseVertex = navMap.vertices.NumObjects();

            for (k = 0; k < side.winding->numpoints; k++) {
                navMap.vertices.AddObject(side.winding->p[k]);
            }

            if (side.winding->numpoints == 3) {
                navMap.indices.AddObject(baseVertex + 0);
                navMap.indices.AddObject(baseVertex + 1);
                navMap.indices.AddObject(baseVertex + 2);
                continue;
            }

            int least = 0;
            for (k = 0; k < side.winding->numpoints; k++) {
                /* get points */
                const vec_t *v1 = side.winding->p[k];
                const vec_t *v2 = side.winding->p[least];

                /* compare */
                if (v1[0] < v2[0] || (v1[0] == v2[0] && v1[1] < v2[1])
                    || (v1[0] == v2[0] && v1[1] == v2[1] && v1[2] < v2[2])) {
                    least = k;
                }
            }

            /* determine the triangle strip order */
            size_t numIndexes = (side.winding->numpoints - 2) * 3;
            size_t ni         = 0;

            /* try all possible orderings of the points looking for a non-degenerate strip order */
            for (int r = 0; r < side.winding->numpoints; r++) {
                /* set rotation */
                int rotate = (r + least) % side.winding->numpoints;

                /* walk the winding in both directions */
                for (ni = 0, k = 0; k < side.winding->numpoints - 2 - k; k++) {
                    /* make indexes */
                    int a = (side.winding->numpoints - 1 - k + rotate) % side.winding->numpoints;
                    int b = (k + rotate) % side.winding->numpoints;
                    int c = (side.winding->numpoints - 2 - k + rotate) % side.winding->numpoints;

                    /* test this triangle */
                    if (side.winding->numpoints > 4 && IsTriangleDegenerate(side.winding->p, a, b, c)) {
                        break;
                    }
                    indexes[ni++] = a;
                    indexes[ni++] = b;
                    indexes[ni++] = c;

                    /* handle end case */
                    if (k + 1 != side.winding->numpoints - 1 - k) {
                        /* make indexes */
                        a = (side.winding->numpoints - 2 - k + rotate) % side.winding->numpoints;
                        b = (k + rotate) % side.winding->numpoints;
                        c = (k + 1 + rotate) % side.winding->numpoints;

                        /* test triangle */
                        if (side.winding->numpoints > 4 && IsTriangleDegenerate(side.winding->p, a, b, c)) {
                            break;
                        }
                        indexes[ni++] = a;
                        indexes[ni++] = b;
                        indexes[ni++] = c;
                    }
                }

                /* valid strip? */
                if (ni == numIndexes) {
                    break;
                }
            }

            /* if any triangle in the strip is degenerate, render from a centered fan point instead */
            if (ni < numIndexes) {
                return;
            }

            for (k = 0; k < numIndexes; k++) {
                navMap.indices.AddObject(baseVertex + indexes[k]);
            }
        }
    }
}

/*
============
LerpDrawVert
============
*/
static void LerpDrawVert(const Vector *a, const Vector *b, Vector *out)
{
    (*out) = 0.5f * (*a + *b);
}

/*
============
Transpose
============
*/
static void Transpose(int width, int height, Vector ctrl[MAX_GRID_SIZE][MAX_GRID_SIZE])
{
    int    i, j;
    Vector temp;

    if (width > height) {
        for (i = 0; i < height; i++) {
            for (j = i + 1; j < width; j++) {
                if (j < height) {
                    // swap the value
                    temp       = ctrl[j][i];
                    ctrl[j][i] = ctrl[i][j];
                    ctrl[i][j] = temp;
                } else {
                    // just copy
                    ctrl[j][i] = ctrl[i][j];
                }
            }
        }
    } else {
        for (i = 0; i < width; i++) {
            for (j = i + 1; j < height; j++) {
                if (j < width) {
                    // swap the value
                    temp       = ctrl[i][j];
                    ctrl[i][j] = ctrl[j][i];
                    ctrl[j][i] = temp;
                } else {
                    // just copy
                    ctrl[i][j] = ctrl[j][i];
                }
            }
        }
    }
}

/*
============
InvertCtrl
============
*/
static void InvertCtrl(int width, int height, Vector ctrl[MAX_GRID_SIZE][MAX_GRID_SIZE])
{
    int    i, j;
    Vector temp;

    for (i = 0; i < height; i++) {
        for (j = 0; j < width / 2; j++) {
            temp                   = ctrl[i][j];
            ctrl[i][j]             = ctrl[i][width - 1 - j];
            ctrl[i][width - 1 - j] = temp;
        }
    }
}

/*
=================
InvertErrorTable
=================
*/
static void InvertErrorTable(float errorTable[2][MAX_GRID_SIZE], int width, int height)
{
    int   i;
    float copy[2][MAX_GRID_SIZE];

    Com_Memcpy(copy, errorTable, sizeof(copy));

    for (i = 0; i < width; i++) {
        errorTable[1][i] = copy[0][i]; //[width-1-i];
    }

    for (i = 0; i < height; i++) {
        errorTable[0][i] = copy[1][height - 1 - i];
    }
}

/*
==================
PutPointsOnCurve
==================
*/
static void PutPointsOnCurve(Vector ctrl[MAX_GRID_SIZE][MAX_GRID_SIZE], int width, int height)
{
    int    i, j;
    Vector prev, next;

    for (i = 0; i < width; i++) {
        for (j = 1; j < height; j += 2) {
            LerpDrawVert(&ctrl[j][i], &ctrl[j + 1][i], &prev);
            LerpDrawVert(&ctrl[j][i], &ctrl[j - 1][i], &next);
            LerpDrawVert(&prev, &next, &ctrl[j][i]);
        }
    }

    for (j = 0; j < height; j++) {
        for (i = 1; i < width; i += 2) {
            LerpDrawVert(&ctrl[j][i], &ctrl[j][i + 1], &prev);
            LerpDrawVert(&ctrl[j][i], &ctrl[j][i - 1], &next);
            LerpDrawVert(&prev, &next, &ctrl[j][i]);
        }
    }
}

/*
=================
G_CreateSurfaceGridMesh
=================
*/
surfaceGrid_t *
G_CreateSurfaceGridMesh(int width, int height, Vector ctrl[MAX_GRID_SIZE][MAX_GRID_SIZE], int *indexes, int numIndexes)
{
    int            i, j, size;
    Vector        *vert;
    surfaceGrid_t *grid;

    // copy the results out to a grid
    size = sizeof(*grid) + sizeof(Vector) * (width * height) + sizeof(*indexes) * numIndexes;

    grid = (surfaceGrid_t *)gi.Malloc(size);
    Com_Memset(grid, 0, size);

    grid->width       = width;
    grid->height      = height;
    grid->numVertices = width * height;
    grid->numIndexes  = numIndexes;

    for (i = 0; i < width; i++) {
        for (j = 0; j < height; j++) {
            vert  = &grid->getVertices()[j * width + i];
            *vert = ctrl[j][i];
        }
    }

    for (i = 0; i < numIndexes; i++) {
        grid->getIndices()[i] = indexes[i];
    }

    return grid;
}

/*
=================
G_FreeSurfaceGridMesh
=================
*/
void G_FreeSurfaceGridMesh(surfaceGrid_t *grid)
{
    gi.Free(grid);
}

static int MakeMeshIndexes(int width, int height, int indexes[(MAX_GRID_SIZE - 1) * (MAX_GRID_SIZE - 1) * 2 * 3])
{
    int i, j;
    int numIndexes;
    int w, h;

    h          = height - 1;
    w          = width - 1;
    numIndexes = 0;
    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            int v1, v2, v3, v4;

            // vertex order to be recognized as tristrips
            v1 = i * width + j + 1;
            v2 = v1 - 1;
            v3 = v2 + width;
            v4 = v3 + 1;

            indexes[numIndexes++] = v2;
            indexes[numIndexes++] = v3;
            indexes[numIndexes++] = v1;

            indexes[numIndexes++] = v1;
            indexes[numIndexes++] = v3;
            indexes[numIndexes++] = v4;
        }
    }

    return numIndexes;
}

/*
=================
G_SubdividePatchToGrid
=================
*/
surfaceGrid_t *
G_SubdividePatchToGrid(int width, int height, float subdivide, Vector points[MAX_PATCH_SIZE * MAX_PATCH_SIZE])
{
    int               i, j, k, l;
    Vector            prev, next, mid;
    float             len, maxLen;
    int               dir;
    int               t;
    MAC_STATIC Vector ctrl[MAX_GRID_SIZE][MAX_GRID_SIZE];
    float             errorTable[2][MAX_GRID_SIZE];
    int               numIndexes;
    int               indexes[(MAX_GRID_SIZE - 1) * (MAX_GRID_SIZE - 1) * 2 * 3];

    for (i = 0; i < width; i++) {
        for (j = 0; j < height; j++) {
            ctrl[j][i] = points[j * width + i];
        }
    }

    for (dir = 0; dir < 2; dir++) {
        for (j = 0; j < MAX_GRID_SIZE; j++) {
            errorTable[dir][j] = 0;
        }

        // horizontal subdivisions
        for (j = 0; j + 2 < width; j += 2) {
            // check subdivided midpoints against control points

            // FIXME: also check midpoints of adjacent patches against the control points
            // this would basically stitch all patches in the same LOD group together.

            maxLen = 0;
            for (i = 0; i < height; i++) {
                vec3_t midxyz;
                vec3_t midxyz2;
                vec3_t dir;
                vec3_t projected;
                float  d;

                // calculate the point on the curve
                for (l = 0; l < 3; l++) {
                    midxyz[l] = (ctrl[i][j][l] + ctrl[i][j + 1][l] * 2 + ctrl[i][j + 2][l]) * 0.25f;
                }

                // see how far off the line it is
                // using dist-from-line will not account for internal
                // texture warping, but it gives a lot less polygons than
                // dist-from-midpoint
                VectorSubtract(midxyz, ctrl[i][j], midxyz);
                VectorSubtract(ctrl[i][j + 2], ctrl[i][j], dir);
                VectorNormalize(dir);

                d = DotProduct(midxyz, dir);
                VectorScale(dir, d, projected);
                VectorSubtract(midxyz, projected, midxyz2);
                len = VectorLengthSquared(midxyz2); // we will do the sqrt later
                if (len > maxLen) {
                    maxLen = len;
                }
            }

            maxLen = sqrt(maxLen);

            // if all the points are on the lines, remove the entire columns
            if (maxLen < 0.1f) {
                errorTable[dir][j + 1] = 999;
                continue;
            }

            // see if we want to insert subdivided columns
            if (width + 2 > MAX_GRID_SIZE) {
                errorTable[dir][j + 1] = 1.0f / maxLen;
                continue; // can't subdivide any more
            }

            if (maxLen <= subdivide) {
                errorTable[dir][j + 1] = 1.0f / maxLen;
                continue; // didn't need subdivision
            }

            errorTable[dir][j + 2] = 1.0f / maxLen;

            // insert two columns and replace the peak
            width += 2;
            for (i = 0; i < height; i++) {
                LerpDrawVert(&ctrl[i][j], &ctrl[i][j + 1], &prev);
                LerpDrawVert(&ctrl[i][j + 1], &ctrl[i][j + 2], &next);
                LerpDrawVert(&prev, &next, &mid);

                for (k = width - 1; k > j + 3; k--) {
                    ctrl[i][k] = ctrl[i][k - 2];
                }
                ctrl[i][j + 1] = prev;
                ctrl[i][j + 2] = mid;
                ctrl[i][j + 3] = next;
            }

            // back up and recheck this set again, it may need more subdivision
            j -= 2;
        }

        Transpose(width, height, ctrl);
        t      = width;
        width  = height;
        height = t;
    }

    // put all the aproximating points on the curve
    PutPointsOnCurve(ctrl, width, height);

    // cull out any rows or columns that are colinear
    for (i = 1; i < width - 1; i++) {
        if (errorTable[0][i] != 999) {
            continue;
        }
        for (j = i + 1; j < width; j++) {
            for (k = 0; k < height; k++) {
                ctrl[k][j - 1] = ctrl[k][j];
            }
            errorTable[0][j - 1] = errorTable[0][j];
        }
        width--;
    }

    for (i = 1; i < height - 1; i++) {
        if (errorTable[1][i] != 999) {
            continue;
        }
        for (j = i + 1; j < height; j++) {
            for (k = 0; k < width; k++) {
                ctrl[j - 1][k] = ctrl[j][k];
            }
            errorTable[1][j - 1] = errorTable[1][j];
        }
        height--;
    }

#if 1
    // flip for longest tristrips as an optimization
    // the results should be visually identical with or
    // without this step
    if (height > width) {
        Transpose(width, height, ctrl);
        InvertErrorTable(errorTable, width, height);
        t      = width;
        width  = height;
        height = t;
        InvertCtrl(width, height, ctrl);
    }
#endif

    // calculate indexes
    numIndexes = MakeMeshIndexes(width, height, indexes);

    return G_CreateSurfaceGridMesh(width, height, ctrl, indexes, numIndexes);
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
        navMap.vertices.AddObject(vertices[i]);
    }

    for (i = 0; i < grid->numIndexes; i++) {
        navMap.indices.AddObject(baseVertex + indexes[i]);
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

        navMap.vertices.AddObject(vec);
    }

    //navMap.indices.Resize(navMap.indices.NumObjects() + numIndexes);

    for (i = 0; i < numIndexes; i++) {
        navMap.indices.AddObject(baseVertex + LittleLong(indexes[i]));
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

        navMap.vertices.AddObject(vec);
    }

    //navMap.indices.Resize(navMap.indices.NumObjects() + numIndexes);

    for (i = 0; i < numIndexes; i++) {
        navMap.indices.AddObject(baseVertex + LittleLong(indexes[i]));
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
static void
G_LoadSurfaces(navMap_t& navMap, const gameLump_c& surfs, const gameLump_c& verts, const gameLump_c& indexLump)
{
    const dsurface_t *in;
    const drawVert_t *dv;
    const int        *indexes;
    int               count;
    int               i;
    size_t            totalNumVerts;
    size_t            totalNumIndexes;
    size_t            numVerts;

    if (surfs.length % sizeof(*in)) {
        throw ScriptException("LoadMap: funny lump size in %s", navMap.mapname);
    }

    count = surfs.length / sizeof(*in);

    dv = (const drawVert_t *)verts.buffer;
    if (verts.length % sizeof(*dv)) {
        throw ScriptException("LoadMap: funny lump size in %s", navMap.mapname);
    }

    indexes = (const int *)indexLump.buffer;
    if (indexLump.length % sizeof(*indexes)) {
        throw ScriptException("LoadMap: funny lump size in %s", navMap.mapname);
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
        case MST_TRIANGLE_SOUP:
            totalNumVerts += in->numVerts;
            totalNumIndexes += in->numIndexes;
            break;
        case MST_PLANAR:
            totalNumVerts += in->numVerts;
            totalNumIndexes += in->numIndexes;
            break;
        case MST_FLARE:
            break;
        default:
            throw ScriptException("Bad surfaceType");
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
            ParseTriSurf(navMap, in, dv, indexes);
            break;
        case MST_PLANAR:
            ParseFace(navMap, in, dv, indexes);
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
G_ValidateHeightmapForVertex
================
*/
static void G_ValidateHeightmapForVertex(terraTri_t *pTri)
{
    for (terraInt i = 0; i < 3; i++) {
        terrainVert_t *pVert = &g_pVert[pTri->iPt[i]];
        if (pVert->pHgt < pTri->patch->heightmap || pVert->pHgt > &pTri->patch->heightmap[80]) {
            pVert->pHgt = pTri->patch->heightmap;
        }
    }
}

/*
================
G_AllocateVert
================
*/
static terraInt G_AllocateVert(cTerraPatchUnpacked_t *patch)
{
    terraInt iVert = g_vert.iFreeHead;

    g_vert.iFreeHead                = g_pVert[g_vert.iFreeHead].iNext;
    g_pVert[g_vert.iFreeHead].iPrev = 0;

    g_pVert[iVert].iPrev                     = 0;
    g_pVert[iVert].iNext                     = patch->drawinfo.iVertHead;
    g_pVert[patch->drawinfo.iVertHead].iPrev = iVert;

    patch->drawinfo.iVertHead = iVert;
    patch->drawinfo.nVerts++;

    assert(g_vert.nFree > 0);
    g_vert.nFree--;

    g_pVert[iVert].nRef         = 0;
    g_pVert[iVert].uiDistRecalc = 0;

    return iVert;
}

/*
================
G_InterpolateVert
================
*/
static void G_InterpolateVert(terraTri_t *pTri, terrainVert_t *pVert)
{
    const terrainVert_t         *pVert0 = &g_pVert[pTri->iPt[0]];
    const terrainVert_t         *pVert1 = &g_pVert[pTri->iPt[1]];
    const cTerraPatchUnpacked_t *pPatch = pTri->patch;
    const byte                  *pMinHeight, *pMaxHeight;

    // Interpolate texture coordinates
    pVert->texCoords[0][0] = (pVert0->texCoords[0][0] + pVert1->texCoords[0][0]) * 0.5f;
    pVert->texCoords[0][1] = (pVert0->texCoords[0][1] + pVert1->texCoords[0][1]) * 0.5f;
    pVert->texCoords[1][0] = (pVert0->texCoords[1][0] + pVert1->texCoords[1][0]) * 0.5f;
    pVert->texCoords[1][1] = (pVert0->texCoords[1][1] + pVert1->texCoords[1][1]) * 0.5f;

    // Fixed in OPM
    //  Use the delta of the two pointers instead of adding them + divide to get the average height,
    //  otherwise the resulting integer would overflow/warp and cause a segmentation fault.
    //  Although rare, the overflow issue can occur in the original game
    pMinHeight  = (byte *)Q_min((uintptr_t)pVert0->pHgt, (uintptr_t)pVert1->pHgt);
    pMaxHeight  = (byte *)Q_max((uintptr_t)pVert0->pHgt, (uintptr_t)pVert1->pHgt);
    pVert->pHgt = (byte *)(pMinHeight + ((pMaxHeight - pMinHeight) >> 1));
    assert(pVert->pHgt >= pMinHeight && pVert->pHgt < pMaxHeight);

    // Calculate the average Z
    pVert->fHgtAvg = (float)(*pVert0->pHgt + *pVert1->pHgt);
    pVert->fHgtAdd = (float)(*pVert->pHgt * 2) - pVert->fHgtAvg;
    pVert->fHgtAvg += pPatch->z0;
    // Calculate the average position
    pVert->xyz[0] = (pVert0->xyz[0] + pVert1->xyz[0]) * 0.5f;
    pVert->xyz[1] = (pVert0->xyz[1] + pVert1->xyz[1]) * 0.5f;
    pVert->xyz[2] = pVert->fHgtAvg;
}

/*
================
G_ReleaseVert
================
*/
static void G_ReleaseVert(cTerraPatchUnpacked_t *patch, int iVert)
{
    terrainVert_t *pVert = &g_pVert[iVert];

    terraInt iPrev       = pVert->iPrev;
    terraInt iNext       = pVert->iNext;
    g_pVert[iPrev].iNext = iNext;
    g_pVert[iNext].iPrev = iPrev;

    assert(patch->drawinfo.nVerts > 0);
    patch->drawinfo.nVerts--;

    if (patch->drawinfo.iVertHead == iVert) {
        patch->drawinfo.iVertHead = iNext;
    }

    pVert->iPrev                    = 0;
    pVert->iNext                    = g_vert.iFreeHead;
    g_pVert[g_vert.iFreeHead].iPrev = iVert;
    g_vert.iFreeHead                = iVert;
    g_vert.nFree++;
}

/*
================
G_AllocateTri
================
*/
terraInt G_AllocateTri(cTerraPatchUnpacked_t *patch, qboolean check)
{
    terraInt iTri = g_tri.iFreeHead;

    g_tri.iFreeHead                = g_pTris[iTri].iNext;
    g_pTris[g_tri.iFreeHead].iPrev = 0;

    g_pTris[iTri].iPrev                     = patch->drawinfo.iTriTail;
    g_pTris[iTri].iNext                     = 0;
    g_pTris[patch->drawinfo.iTriTail].iNext = iTri;
    patch->drawinfo.iTriTail                = iTri;

    if (!patch->drawinfo.iTriHead) {
        patch->drawinfo.iTriHead = iTri;
    }

    patch->drawinfo.nTris++;
    assert(g_tri.nFree > 0);
    g_tri.nFree--;

    g_pTris[iTri].byConstChecks = check ? 0 : 4;
    g_pTris[iTri].iParent       = 0;

    return iTri;
}

/*
================
R_ReleaseTri
================
*/
static void G_ReleaseTri(cTerraPatchUnpacked_t *patch, terraInt iTri)
{
    terraTri_t *pTri = &g_pTris[iTri];

    terraInt iPrev       = pTri->iPrev;
    terraInt iNext       = pTri->iNext;
    g_pTris[iPrev].iNext = iNext;
    g_pTris[iNext].iPrev = iPrev;

    if (g_tri.iCur == iTri) {
        g_tri.iCur = iNext;
    }

    assert(patch->drawinfo.nTris > 0);
    patch->drawinfo.nTris--;

    if (patch->drawinfo.iTriHead == iTri) {
        patch->drawinfo.iTriHead = iNext;
    }

    if (patch->drawinfo.iTriTail == iTri) {
        patch->drawinfo.iTriTail = iPrev;
    }

    pTri->iPrev                    = 0;
    pTri->iNext                    = g_tri.iFreeHead;
    g_pTris[g_tri.iFreeHead].iPrev = iTri;
    g_tri.iFreeHead                = iTri;
    g_tri.nFree++;

    for (int i = 0; i < 3; i++) {
        terraInt ptNum = pTri->iPt[i];

        g_pVert[ptNum].nRef--;
        if (!g_pVert[ptNum].nRef) {
            G_ReleaseVert(patch, ptNum);
        }
    }
}

/*
================
G_ConstChecksForTri
================
*/
static int G_ConstChecksForTri(terraTri_t *pTri)
{
    varnodeUnpacked_t vn;

    if (pTri->lod == MAX_TERRAIN_LOD) {
        return 2;
    }

    vn = *pTri->varnode;
    //vn.s.flags &= 0xF0;
    vn.flags &= 0xFFFFFFF0;

    if (vn.fVariance == 0.0 && !(pTri->varnode->flags & 8)) {
        return 2;
    } else if (pTri->varnode->flags & 8) {
        return 3;
    } else if ((pTri->byConstChecks & 4) && !(pTri->varnode->flags & 4) && pTri->lod < MAX_TERRAIN_LOD) {
        return 0;
    }

    return 2;
}

/*
================
G_DemoteInAncestry
================
*/
static void G_DemoteInAncestry(cTerraPatchUnpacked_t *patch, terraInt iTri)
{
    terraInt iPrev = g_pTris[iTri].iPrev;
    terraInt iNext = g_pTris[iTri].iNext;

    g_pTris[iPrev].iNext = iNext;
    g_pTris[iNext].iPrev = iPrev;

    if (g_tri.iCur == iTri) {
        g_tri.iCur = iNext;
    }

    assert(patch->drawinfo.nTris > 0);
    patch->drawinfo.nTris--;

    if (patch->drawinfo.iTriHead == iTri) {
        patch->drawinfo.iTriHead = iNext;
    }

    if (patch->drawinfo.iTriTail == iTri) {
        patch->drawinfo.iTriTail = iPrev;
    }

    g_pTris[iTri].iPrev                       = 0;
    g_pTris[iTri].iNext                       = patch->drawinfo.iMergeHead;
    g_pTris[patch->drawinfo.iMergeHead].iPrev = iTri;
    patch->drawinfo.iMergeHead                = iTri;
}

/*
================
R_TerrainHeapInit
================
*/
static void G_TerrainHeapInit()
{
    g_tri.iFreeHead  = 1;
    g_tri.nFree      = g_nTris - 1;
    g_vert.iFreeHead = 1;
    g_vert.nFree     = g_nVerts - 1;

    for (size_t i = 0; i < g_nTris; i++) {
        g_pTris[i].iPrev = (terraInt)i - 1;
        g_pTris[i].iNext = (terraInt)i + 1;
    }

    g_pTris[0].iPrev           = 0;
    g_pTris[g_nTris - 1].iNext = 0;

    for (size_t i = 0; i < g_nVerts; i++) {
        g_pVert[i].iPrev = (terraInt)i - 1;
        g_pVert[i].iNext = (terraInt)i + 1;
    }

    g_pVert[0].iPrev            = 0;
    g_pVert[g_nVerts - 1].iNext = 0;
}

/*
================
R_TerrainPatchesInit
================
*/
static void G_TerrainPatchesInit(cTerraPatchUnpacked_t *terraPatches, size_t numTerraPatches)
{
    int i;

    for (i = 0; i < numTerraPatches; i++) {
        cTerraPatchUnpacked_t *patch = &terraPatches[i];
        patch->drawinfo.iTriHead     = 0;
        patch->drawinfo.iTriTail     = 0;
        patch->drawinfo.iMergeHead   = 0;
        patch->drawinfo.iVertHead    = 0;
        patch->drawinfo.nTris        = 0;
        patch->drawinfo.nVerts       = 0;
    }
}

/*
================
G_SplitTri
================
*/

void G_SplitTri(
    terraInt iSplit, terraInt iNewPt, terraInt iLeft, terraInt iRight, terraInt iRightOfLeft, terraInt iLeftOfRight
)
{
    terraTri_t *pSplit = &g_pTris[iSplit];

    terraTri_t *pLeft;
    if (iLeft) {
        pLeft = &g_pTris[iLeft];
    } else {
        pLeft = NULL;
    }

    terraTri_t *pRight;
    if (iRight) {
        pRight = &g_pTris[iRight];
    } else {
        pRight = NULL;
    }

    int                iNextLod = pSplit->lod + 1;
    int                index    = pSplit->index;
    varnodeUnpacked_t *varnode  = &pSplit->varnode[index];

    if (pLeft) {
        pLeft->patch   = pSplit->patch;
        pLeft->index   = index * 2;
        pLeft->varnode = varnode;
        pLeft->lod     = iNextLod;
        pLeft->iLeft   = iRight;
        pLeft->iRight  = iRightOfLeft;
        pLeft->iBase   = pSplit->iLeft;
        pLeft->iPt[0]  = pSplit->iPt[1];
        pLeft->iPt[1]  = pSplit->iPt[2];
        pLeft->iPt[2]  = iNewPt;

        G_ValidateHeightmapForVertex(pLeft);
        pLeft->byConstChecks |= G_ConstChecksForTri(pLeft);

        g_pVert[pLeft->iPt[0]].nRef++;
        g_pVert[pLeft->iPt[1]].nRef++;
        g_pVert[pLeft->iPt[2]].nRef++;
        g_pTris[pSplit->iParent].nSplit++;
        pLeft->nSplit = 0;
    }

    if (pSplit->iLeft) {
        if (g_pTris[pSplit->iLeft].lod == iNextLod) {
            g_pTris[pSplit->iLeft].iBase = iLeft;
        } else {
            g_pTris[pSplit->iLeft].iRight = iLeft;
        }
    }

    if (pRight) {
        pRight->patch   = pSplit->patch;
        pRight->index   = index * 2 + 1;
        pRight->varnode = varnode + 1;
        pRight->lod     = iNextLod;
        pRight->iLeft   = iLeftOfRight;
        pRight->iRight  = iLeft;
        pRight->iBase   = pSplit->iRight;
        pRight->iPt[0]  = pSplit->iPt[2];
        pRight->iPt[1]  = pSplit->iPt[0];
        pRight->iPt[2]  = iNewPt;

        G_ValidateHeightmapForVertex(pRight);
        pRight->byConstChecks |= G_ConstChecksForTri(pRight);

        g_pVert[pRight->iPt[0]].nRef++;
        g_pVert[pRight->iPt[1]].nRef++;
        g_pVert[pRight->iPt[2]].nRef++;
        g_pTris[pSplit->iParent].nSplit++;
        pRight->nSplit = 0;
    }

    if (pSplit->iRight) {
        if (g_pTris[pSplit->iRight].lod == iNextLod) {
            g_pTris[pSplit->iRight].iBase = iRight;
        } else {
            g_pTris[pSplit->iRight].iLeft = iRight;
        }
    }
}

/*
================
G_ForceSplit
================
*/
static void G_ForceSplit(terraInt iTri)
{
    terraTri_t    *pTri = &g_pTris[iTri];
    terraTri_t    *pBase;
    terrainVert_t *pVert;
    terraInt       iBase;
    terraInt       iTriLeft, iTriRight;
    terraInt       iNewPt;
    terraInt       iBaseLeft = 0, iBaseRight = 0;
    terraInt       iNewBasePt;
    uint32_t       flags, flags2;

    g_nSplit++;

    iBase = pTri->iBase;
    pBase = &g_pTris[iBase];
    if (iBase && pBase->lod != pTri->lod) {
        assert(g_pTris[pTri->iBase].iBase != iTri);
        assert(g_tri.nFree >= 8);

        G_ForceSplit(iBase);

        assert(g_tri.nFree >= 4);

        iBase = pTri->iBase;
        pBase = &g_pTris[iBase];
    }

    flags = pTri->varnode->flags;

    iTriLeft  = G_AllocateTri(pTri->patch, (flags & 2));
    iTriRight = G_AllocateTri(pTri->patch, (flags & 1));

    iNewPt = G_AllocateVert(pTri->patch);
    G_InterpolateVert(pTri, &g_pVert[iNewPt]);

    g_pVert[iNewPt].fVariance = pTri->varnode->fVariance;

    iBaseLeft  = 0;
    iBaseRight = 0;

    if (iBase) {
        flags2 = pBase->varnode->flags;
        flags |= flags2;

        iBaseLeft  = G_AllocateTri(pBase->patch, (flags2 & 2));
        iBaseRight = G_AllocateTri(pBase->patch, (flags2 & 1));

        iNewBasePt = iNewPt;
        if (pBase->patch != pTri->patch) {
            iNewBasePt = G_AllocateVert(pBase->patch);
            pVert      = &g_pVert[iNewBasePt];
            G_InterpolateVert(pBase, pVert);

            pVert->fVariance = g_pVert[iNewPt].fVariance;
            if (flags & 8) {
                pVert->fHgtAvg += pVert->fHgtAdd;
                pVert->fHgtAdd   = 0.0;
                pVert->fVariance = 0.0;
                pVert->xyz[2]    = pVert->fHgtAvg;
            }
        }

        G_SplitTri(iBase, iNewBasePt, iBaseLeft, iBaseRight, iTriRight, iTriLeft);

        pBase->iLeftChild           = iBaseLeft;
        pBase->iRightChild          = iBaseRight;
        g_pTris[iBaseLeft].iParent  = iBase;
        g_pTris[iBaseRight].iParent = iBase;
        G_DemoteInAncestry(pBase->patch, iBase);
    }

    if (flags & 8) {
        pVert = &g_pVert[iNewPt];
        pVert->fHgtAvg += pVert->fHgtAdd;
        pVert->fHgtAdd   = 0.0;
        pVert->fVariance = 0.0;
        pVert->xyz[2]    = pVert->fHgtAvg;
    }

    G_SplitTri(iTri, iNewPt, iTriLeft, iTriRight, iBaseRight, iBaseLeft);

    pTri->iLeftChild           = iTriLeft;
    pTri->iRightChild          = iTriRight;
    g_pTris[iTriLeft].iParent  = iTri;
    g_pTris[iTriRight].iParent = iTri;
    G_DemoteInAncestry(pTri->patch, iTri);
}

/*
================
G_ForceMerge
================
*/
static void G_ForceMerge(terraInt iTri)
{
    terraTri_t            *pTri  = &g_pTris[iTri];
    cTerraPatchUnpacked_t *patch = pTri->patch;
    terraInt               iPrev = pTri->iPrev;
    terraInt               iNext = pTri->iNext;

    g_nMerge++;

    if (pTri->iLeftChild) {
        terraInt iLeft = g_pTris[pTri->iLeftChild].iBase;

        pTri->iLeft = iLeft;
        if (iLeft) {
            if (g_pTris[iLeft].lod == pTri->lod) {
                g_pTris[iLeft].iRight = iTri;
            } else {
                g_pTris[iLeft].iBase = iTri;
            }
        }

        G_ReleaseTri(pTri->patch, pTri->iLeftChild);

        pTri->iLeftChild = 0;
        g_pTris[pTri->iParent].nSplit--;
    }

    if (pTri->iRightChild) {
        terraInt iRight = g_pTris[pTri->iRightChild].iBase;

        pTri->iRight = iRight;
        if (iRight) {
            if (g_pTris[iRight].lod == pTri->lod) {
                g_pTris[iRight].iLeft = iTri;
            } else {
                g_pTris[iRight].iBase = iTri;
            }
        }

        G_ReleaseTri(pTri->patch, pTri->iRightChild);

        pTri->iLeftChild = 0;
        g_pTris[pTri->iParent].nSplit--;
    }

    g_pTris[iPrev].iNext = iNext;
    g_pTris[iNext].iPrev = iPrev;

    if (g_tri.iCur == iTri) {
        g_tri.iCur = iNext;
    }

    patch->drawinfo.nTris++;
    if (patch->drawinfo.iMergeHead == iTri) {
        patch->drawinfo.iMergeHead = iNext;
    }

    g_pTris[iTri].iPrev = patch->drawinfo.iTriTail;
    g_pTris[iTri].iNext = 0;

    g_pTris[patch->drawinfo.iTriTail].iNext = iTri;

    patch->drawinfo.iTriTail = iTri;
    if (!patch->drawinfo.iTriHead) {
        patch->drawinfo.iTriHead = iTri;
    }
}

/*
================
G_TerraTriNeighbor
================
*/
static int G_TerraTriNeighbor(cTerraPatchUnpacked_t *terraPatches, int iPatch, int dir)
{
    int iNeighbor;

    if (iPatch < 0) {
        return 0;
    }

    iNeighbor = 2 * iPatch + 1;

    switch (dir) {
    case 0:
        return iNeighbor;
    case 1:
        if (terraPatches[iPatch].flags & 0x80) {
            return iNeighbor;
        } else {
            return iNeighbor + 1;
        }
        break;
    case 2:
        return iNeighbor + 1;
    case 3:
        if (terraPatches[iPatch].flags & 0x80) {
            return iNeighbor + 1;
        } else {
            return iNeighbor;
        }
        break;
    }

    return 0;
}

/*
================
G_PreTessellateTerrain
================
*/
static void G_PreTessellateTerrain(cTerraPatchUnpacked_t *terraPatches, size_t numTerraPatches)
{
    if (!numTerraPatches) {
        return;
    }

    g_nTris  = numTerraPatches * 8 * 8 * 6 * MAX_TERRAIN_LOD + 1;
    g_nVerts = g_nTris;
    g_pTris  = (terraTri_t *)gi.Malloc(g_nTris * sizeof(terraTri_t));
    g_pVert  = (terrainVert_t *)gi.Malloc(g_nVerts * sizeof(terrainVert_t));

    // Init triangles & vertices
    G_TerrainHeapInit();
    G_TerrainPatchesInit(terraPatches, numTerraPatches);

    for (size_t i = 0; i < numTerraPatches; i++) {
        cTerraPatchUnpacked_t *patch = &terraPatches[i];

        patch->drawinfo.nTris     = 0;
        patch->drawinfo.nVerts    = 0;
        patch->drawinfo.iTriHead  = 0;
        patch->drawinfo.iTriTail  = 0;
        patch->drawinfo.iVertHead = 0;

        terraInt iTri0 = G_AllocateTri(patch, qfalse);
        terraInt iTri1 = G_AllocateTri(patch, qfalse);
        terraInt i00   = G_AllocateVert(patch);
        terraInt i01   = G_AllocateVert(patch);
        terraInt i10   = G_AllocateVert(patch);
        terraInt i11   = G_AllocateVert(patch);

        terrainVert_t *pVert;
        pVert            = &g_pVert[i00];
        pVert->xyz[0]    = patch->x0;
        pVert->xyz[1]    = patch->y0;
        pVert->xyz[2]    = (float)(patch->heightmap[0] * 2) + patch->z0;
        pVert->pHgt      = &patch->heightmap[0];
        pVert->fHgtAvg   = pVert->xyz[2];
        pVert->fVariance = 0.0f;
        pVert->nRef      = 4;

        pVert            = &g_pVert[i01];
        pVert->xyz[0]    = patch->x0;
        pVert->xyz[1]    = patch->y0 + 512.0f;
        pVert->xyz[2]    = (float)(patch->heightmap[72] * 2) + patch->z0;
        pVert->pHgt      = &patch->heightmap[72];
        pVert->fHgtAvg   = pVert->xyz[2];
        pVert->fVariance = 0.0f;
        pVert->nRef      = 4;

        pVert            = &g_pVert[i10];
        pVert->xyz[0]    = patch->x0 + 512.0f;
        pVert->xyz[1]    = patch->y0;
        pVert->xyz[2]    = (float)(patch->heightmap[8] * 2) + patch->z0;
        pVert->pHgt      = &patch->heightmap[8];
        pVert->fHgtAvg   = pVert->xyz[2];
        pVert->fVariance = 0.0f;
        pVert->nRef      = 4;

        pVert            = &g_pVert[i11];
        pVert->xyz[0]    = patch->x0 + 512.0f;
        pVert->xyz[1]    = patch->y0 + 512.0f;
        pVert->xyz[2]    = (float)(patch->heightmap[80] * 2) + patch->z0;
        pVert->pHgt      = &patch->heightmap[80];
        pVert->fHgtAvg   = pVert->xyz[2];
        pVert->fVariance = 0.0f;
        pVert->nRef      = 4;

        terraTri_t *pTri = &g_pTris[iTri0];
        pTri->patch      = patch;
        pTri->varnode    = &patch->varTree[0][0];
        pTri->index      = 1;
        pTri->lod        = 0;
        pTri->byConstChecks |= G_ConstChecksForTri(pTri);

        pTri->iBase = iTri1;
        if ((patch->flags & 0x80u) == 0) {
            pTri->iLeft  = G_TerraTriNeighbor(terraPatches, patch->iWest, 1);
            pTri->iRight = G_TerraTriNeighbor(terraPatches, patch->iNorth, 2);
            if (patch->flags & 0x40) {
                pTri->iPt[0] = i00;
                pTri->iPt[1] = i11;
            } else {
                pTri->iPt[0] = i11;
                pTri->iPt[1] = i00;
            }
            pTri->iPt[2] = i01;
        } else {
            pTri->iLeft  = G_TerraTriNeighbor(terraPatches, patch->iNorth, 2);
            pTri->iRight = G_TerraTriNeighbor(terraPatches, patch->iEast, 3);
            if (patch->flags & 0x40) {
                pTri->iPt[0] = i01;
                pTri->iPt[1] = i10;
            } else {
                pTri->iPt[0] = i10;
                pTri->iPt[1] = i01;
            }
            pTri->iPt[2] = i11;
        }

        G_ValidateHeightmapForVertex(pTri);

        pTri          = &g_pTris[iTri1];
        pTri->patch   = patch;
        pTri->varnode = &patch->varTree[1][0];
        pTri->index   = 1;
        pTri->lod     = 0;
        pTri->byConstChecks |= G_ConstChecksForTri(pTri);

        pTri->iBase = iTri0;
        if ((patch->flags & 0x80u) == 0) {
            pTri->iLeft  = G_TerraTriNeighbor(terraPatches, patch->iEast, 3);
            pTri->iRight = G_TerraTriNeighbor(terraPatches, patch->iSouth, 0);
            if (patch->flags & 0x40) {
                pTri->iPt[0] = i11;
                pTri->iPt[1] = i00;
            } else {
                pTri->iPt[0] = i00;
                pTri->iPt[1] = i11;
            }
            pTri->iPt[2] = i10;
        } else {
            pTri->iLeft  = G_TerraTriNeighbor(terraPatches, patch->iSouth, 0);
            pTri->iRight = G_TerraTriNeighbor(terraPatches, patch->iWest, 1);
            if (patch->flags & 0x40) {
                pTri->iPt[0] = i10;
                pTri->iPt[1] = i01;
            } else {
                pTri->iPt[0] = i01;
                pTri->iPt[1] = i10;
            }
            pTri->iPt[2] = i00;
        }

        G_ValidateHeightmapForVertex(pTri);
    }
}

/*
================
G_NeedSplitTri
================
*/
static qboolean G_NeedSplitTri(terraTri_t *pTri)
{
    uint8_t byConstChecks = pTri->byConstChecks;
    if (byConstChecks & 2) {
        return byConstChecks & 1;
    }

    return qtrue;
}

/*
================
G_DoTriSplitting
================
*/
static void G_DoTriSplitting(cTerraPatchUnpacked_t *terraPatches, size_t numTerraPatches)
{
    cTerraPatchUnpacked_t *patch;
    size_t                 i;

    for (i = 0; i < numTerraPatches; i++) {
        patch = &terraPatches[i];

        g_tri.iCur = patch->drawinfo.iTriHead;
        while (g_tri.iCur != 0) {
            terraTri_t *pTri = &g_pTris[g_tri.iCur];

            if (G_NeedSplitTri(pTri)) {
                if (g_tri.nFree < 14 * 2 || g_vert.nFree < 14) {
                    // this shouldn't happen
                    throw("aborting terrain tessellation -- insufficient tris\n");
                }

                G_ForceSplit(g_tri.iCur);

                if (&g_pTris[g_tri.iCur] == pTri) {
                    g_tri.iCur = g_pTris[g_tri.iCur].iNext;
                }
            } else {
                g_tri.iCur = g_pTris[g_tri.iCur].iNext;
            }
        }
    }
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

        navMap.vertices.AddObject(vert.xyz);
        vert.iVertArray = currentVertice;

        currentVertice++;
    }

    for (triNum = patch.drawinfo.iTriHead; triNum; triNum = g_pTris[triNum].iNext) {
        const terraTri_t& tri = g_pTris[triNum];

        if (tri.byConstChecks & 4) {
            navMap.indices.AddObject(baseVertice + g_pVert[tri.iPt[0]].iVertArray);
            navMap.indices.AddObject(baseVertice + g_pVert[tri.iPt[1]].iVertArray);
            navMap.indices.AddObject(baseVertice + g_pVert[tri.iPt[2]].iVertArray);
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
}

/*
================
G_LoadTerrain
================
*/
void G_LoadTerrain(navMap_t& navMap, const gameLump_c& lump)
{
    int                    i;
    cTerraPatch_t         *in;
    cTerraPatchUnpacked_t *out;
    size_t                 numTerraPatches;
    cTerraPatchUnpacked_t *terraPatches;

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
    dheader_t    header;
    fileHandle_t h;
    int          length;
    int          i;
    gameLump_c   lumps[4];

    // load it
    length = gi.FS_FOpenFile(mapname, &h, qtrue, qtrue);
    if (length <= 0) {
        return;
    }

    gi.FS_Read(&header, sizeof(dheader_t), h);

    for (i = 0; i < sizeof(dheader_t) / 4; i++) {
        ((int *)&header)[i] = LittleLong(((int *)&header)[i]);
    }

    outNavigationMap.mapname = mapname;

    try {
        lumps[0] = gameLump_c::LoadLump(h, *Q_GetLumpByVersion(&header, LUMP_SHADERS));
        lumps[1] = gameLump_c::LoadLump(h, *Q_GetLumpByVersion(&header, LUMP_PLANES));
        lumps[2] = gameLump_c::LoadLump(h, *Q_GetLumpByVersion(&header, LUMP_BRUSHSIDES));
        lumps[3] = gameLump_c::LoadLump(h, *Q_GetLumpByVersion(&header, LUMP_BRUSHES));
        //G_GenerateVerticesFromHull(outNavigationMap, lumps[0], lumps[1], lumps[2], lumps[3]);

        // Gather vertices from meshes and surfaces
        lumps[0] = gameLump_c::LoadLump(h, *Q_GetLumpByVersion(&header, LUMP_SURFACES));
        lumps[1] = gameLump_c::LoadLump(h, *Q_GetLumpByVersion(&header, LUMP_DRAWVERTS));
        lumps[2] = gameLump_c::LoadLump(h, *Q_GetLumpByVersion(&header, LUMP_DRAWINDEXES));
        G_LoadSurfaces(outNavigationMap, lumps[0], lumps[1], lumps[2]);

        // Gather vertices from LOD terrain
        lumps[0] = gameLump_c::LoadLump(h, *Q_GetLumpByVersion(&header, LUMP_TERRAIN));
        G_LoadTerrain(outNavigationMap, lumps[0]);
    } catch (const ScriptException&) {
        gi.FS_FCloseFile(h);
        throw;
    }

    gi.FS_FCloseFile(h);
}
