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
#include "../qcommon/qfiles.h"
#include "../qcommon/container.h"
#include "../qcommon/vector.h"
#include "../script/scriptexception.h"

#define MAX_GRID_SIZE  129
#define MAX_PATCH_SIZE 32

struct navMap_t {
    const char       *mapname;
    Container<int>    indices;
    Container<Vector> vertices;
};

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

gameLump_c::gameLump_c(gameLump_c&& other) noexcept
{
    FreeLump();

    buffer       = other.buffer;
    length       = other.length;
    other.buffer = NULL;
}

gameLump_c& gameLump_c::operator=(gameLump_c&& other) noexcept
{
    FreeLump();

    buffer       = other.buffer;
    length       = other.length;
    other.buffer = NULL;

    return *this;
}

gameLump_c::~gameLump_c()
{
    FreeLump();
}

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

void gameLump_c::FreeLump()
{
    if (buffer) {
        gi.Free(buffer);
        buffer = NULL;
        length = 0;
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

    navMap.vertices.Resize(baseVertex + grid->numVertices);
    navMap.indices.Resize(baseIndex + grid->numIndexes);

    for (i = 0; i < grid->numVertices; i++) {
        navMap.vertices.AddObjectAt(baseVertex + i + 1, vertices[i]);
    }

    for (i = 0; i < grid->numIndexes; i++) {
        navMap.indices.AddObjectAt(baseIndex + i + 1, baseVertex + indexes[i]);
    }
}

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

    navMap.vertices.Resize(navMap.vertices.NumObjects() + numPoints);

    for (i = 0; i < numPoints; i++) {
        Vector vec;

        for (j = 0; j < 3; j++) {
            vec[j] = LittleFloat(verts[i].xyz[j]);
        }

        navMap.vertices.AddObject(vec);
    }

    navMap.indices.Resize(navMap.indices.NumObjects() + numIndexes);

    for (i = 0; i < numIndexes; i++) {
        navMap.indices.AddObjectAt(baseIndex + i + 1, baseVertex + LittleLong(indexes[i]));
    }
}

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

    navMap.vertices.Resize(navMap.vertices.NumObjects() + numPoints);

    for (i = 0; i < numPoints; i++) {
        Vector vec;

        for (j = 0; j < 3; j++) {
            vec[j] = LittleFloat(verts[i].xyz[j]);
        }

        navMap.vertices.AddObject(vec);
    }

    navMap.indices.Resize(navMap.indices.NumObjects() + numIndexes);

    for (i = 0; i < numIndexes; i++) {
        navMap.indices.AddObjectAt(baseIndex + i + 1, baseVertex + LittleLong(indexes[i]));
    }
}

static void ParseFlare(navMap_t& navMap, const dsurface_t *ds, const drawVert_t *verts)
{
    // Nothing to do
}

static void
G_LoadSurfaces(navMap_t& navMap, const gameLump_c& surfs, const gameLump_c& verts, const gameLump_c& indexLump)
{
    const dsurface_t *in;
    const drawVert_t *dv;
    const int        *indexes;
    int               count;
    int               i;

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

void G_Navigation_LoadWorldMap(const char *mapname)
{
    dheader_t    header;
    fileHandle_t h;
    int          length;
    int          i;
    gameLump_c   lumps[3];
    navMap_t     navMap;

    if (!sv_maxbots->integer) {
        return;
    }

    // load it
    length = gi.FS_FOpenFile(mapname, &h, qtrue, qtrue);
    if (length <= 0) {
        return;
    }

    gi.FS_Read(&header, sizeof(dheader_t), h);

    for (i = 0; i < sizeof(dheader_t) / 4; i++) {
        ((int *)&header)[i] = LittleLong(((int *)&header)[i]);
    }

    navMap.mapname = mapname;

    try {
        lumps[0] = gameLump_c::LoadLump(h, *Q_GetLumpByVersion(&header, LUMP_SURFACES));
        lumps[1] = gameLump_c::LoadLump(h, *Q_GetLumpByVersion(&header, LUMP_DRAWVERTS));
        lumps[2] = gameLump_c::LoadLump(h, *Q_GetLumpByVersion(&header, LUMP_DRAWINDEXES));
        G_LoadSurfaces(navMap, lumps[0], lumps[1], lumps[2]);

        // FIXME: half implemented
        //  1. Load terrains and render them into vertices
    } catch (const ScriptException& e) {
        gi.Printf("Failed to load BSP for navigation: %s\n", e.string.c_str());
    }

    gi.FS_FCloseFile(h);
}
