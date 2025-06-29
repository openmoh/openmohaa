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
 * @file navigation_bsp_load_curve.cpp
 * @brief Render patches from BSP into triangles and vertices.
 * 
 */

#include "g_local.h"
#include "navigation_bsp.h"

#define WRAP_POINT_EPSILON 0.1

typedef struct {
    int      width;
    int      height;
    qboolean wrapWidth;
    qboolean wrapHeight;
    vec3_t   points[MAX_GRID_SIZE][MAX_GRID_SIZE]; // [width][height]
} cGrid_t;

/*
=================
G_NeedsSubdivision

Returns true if the given quadratic curve is not flat enough for our
collision detection purposes
=================
*/
static qboolean G_NeedsSubdivision(vec3_t a, vec3_t b, vec3_t c, float subdivisions)
{
    vec3_t cmid;
    vec3_t lmid;
    vec3_t delta;
    float  dist;
    int    i;

    // calculate the linear midpoint
    for (i = 0; i < 3; i++) {
        lmid[i] = 0.5 * (a[i] + c[i]);
    }

    // calculate the exact curve midpoint
    for (i = 0; i < 3; i++) {
        cmid[i] = 0.5 * (0.5 * (a[i] + b[i]) + 0.5 * (b[i] + c[i]));
    }

    // see if the curve is far enough away from the linear mid
    VectorSubtract(cmid, lmid, delta);
    dist = VectorLength(delta);

    return dist >= subdivisions;
}

/*
===============
G_Subdivide

a, b, and c are control points.
the subdivided sequence will be: a, out1, out2, out3, c
===============
*/
static void G_Subdivide(vec3_t a, vec3_t b, vec3_t c, vec3_t out1, vec3_t out2, vec3_t out3)
{
    int i;

    for (i = 0; i < 3; i++) {
        out1[i] = 0.5 * (a[i] + b[i]);
        out3[i] = 0.5 * (b[i] + c[i]);
        out2[i] = 0.5 * (out1[i] + out3[i]);
    }
}

/*
=================
G_TransposeGrid

Swaps the rows and columns in place
=================
*/
static void G_TransposeGrid(cGrid_t *grid)
{
    int      i, j, l;
    vec3_t   temp;
    qboolean tempWrap;

    if (grid->width > grid->height) {
        for (i = 0; i < grid->height; i++) {
            for (j = i + 1; j < grid->width; j++) {
                if (j < grid->height) {
                    // swap the value
                    VectorCopy(grid->points[i][j], temp);
                    VectorCopy(grid->points[j][i], grid->points[i][j]);
                    VectorCopy(temp, grid->points[j][i]);
                } else {
                    // just copy
                    VectorCopy(grid->points[j][i], grid->points[i][j]);
                }
            }
        }
    } else {
        for (i = 0; i < grid->width; i++) {
            for (j = i + 1; j < grid->height; j++) {
                if (j < grid->width) {
                    // swap the value
                    VectorCopy(grid->points[j][i], temp);
                    VectorCopy(grid->points[i][j], grid->points[j][i]);
                    VectorCopy(temp, grid->points[i][j]);
                } else {
                    // just copy
                    VectorCopy(grid->points[i][j], grid->points[j][i]);
                }
            }
        }
    }

    l            = grid->width;
    grid->width  = grid->height;
    grid->height = l;

    tempWrap         = grid->wrapWidth;
    grid->wrapWidth  = grid->wrapHeight;
    grid->wrapHeight = tempWrap;
}

/*
===================
G_SetGridWrapWidth

If the left and right columns are exactly equal, set grid->wrapWidth qtrue
===================
*/
static void G_SetGridWrapWidth(cGrid_t *grid)
{
    int   i, j;
    float d;

    for (i = 0; i < grid->height; i++) {
        for (j = 0; j < 3; j++) {
            d = grid->points[0][i][j] - grid->points[grid->width - 1][i][j];
            if (d < -WRAP_POINT_EPSILON || d > WRAP_POINT_EPSILON) {
                break;
            }
        }
        if (j != 3) {
            break;
        }
    }
    if (i == grid->height) {
        grid->wrapWidth = qtrue;
    } else {
        grid->wrapWidth = qfalse;
    }
}

/*
=================
G_SubdivideGridColumns

Adds columns as necessary to the grid until
all the aproximating points are within SUBDIVIDE_DISTANCE
from the true curve
=================
*/
static void G_SubdivideGridColumns(cGrid_t *grid, float subdivisions)
{
    int i, j, k;

    for (i = 0; i < grid->width - 2;) {
        // grid->points[i][x] is an interpolating control point
        // grid->points[i+1][x] is an aproximating control point
        // grid->points[i+2][x] is an interpolating control point

        //
        // first see if we can collapse the aproximating collumn away
        //
        for (j = 0; j < grid->height; j++) {
            if (G_NeedsSubdivision(grid->points[i][j], grid->points[i + 1][j], grid->points[i + 2][j], subdivisions)) {
                break;
            }
        }
        if (j == grid->height) {
            // all of the points were close enough to the linear midpoints
            // that we can collapse the entire column away
            for (j = 0; j < grid->height; j++) {
                // remove the column
                for (k = i + 2; k < grid->width; k++) {
                    VectorCopy(grid->points[k][j], grid->points[k - 1][j]);
                }
            }

            grid->width--;

            // go to the next curve segment
            i++;
            continue;
        }

        //
        // we need to subdivide the curve
        //
        for (j = 0; j < grid->height; j++) {
            vec3_t prev, mid, next;

            // save the control points now
            VectorCopy(grid->points[i][j], prev);
            VectorCopy(grid->points[i + 1][j], mid);
            VectorCopy(grid->points[i + 2][j], next);

            // make room for two additional columns in the grid
            // columns i+1 will be replaced, column i+2 will become i+4
            // i+1, i+2, and i+3 will be generated
            for (k = grid->width - 1; k > i + 1; k--) {
                VectorCopy(grid->points[k][j], grid->points[k + 2][j]);
            }

            // generate the subdivided points
            G_Subdivide(prev, mid, next, grid->points[i + 1][j], grid->points[i + 2][j], grid->points[i + 3][j]);
        }

        grid->width += 2;

        // the new aproximating point at i+1 may need to be removed
        // or subdivided farther, so don't advance i
    }
}

/*
======================
G_ComparePoints
======================
*/
#define POINT_EPSILON 0.1

static qboolean G_ComparePoints(float *a, float *b)
{
    float d;

    d = a[0] - b[0];
    if (d < -POINT_EPSILON || d > POINT_EPSILON) {
        return qfalse;
    }
    d = a[1] - b[1];
    if (d < -POINT_EPSILON || d > POINT_EPSILON) {
        return qfalse;
    }
    d = a[2] - b[2];
    if (d < -POINT_EPSILON || d > POINT_EPSILON) {
        return qfalse;
    }
    return qtrue;
}

/*
=================
G_RemoveDegenerateColumns

If there are any identical columns, remove them
=================
*/
static void G_RemoveDegenerateColumns(cGrid_t *grid)
{
    int i, j, k;

    for (i = 0; i < grid->width - 1; i++) {
        for (j = 0; j < grid->height; j++) {
            if (!G_ComparePoints(grid->points[i][j], grid->points[i + 1][j])) {
                break;
            }
        }

        if (j != grid->height) {
            continue; // not degenerate
        }

        for (j = 0; j < grid->height; j++) {
            // remove the column
            for (k = i + 2; k < grid->width; k++) {
                VectorCopy(grid->points[k][j], grid->points[k - 1][j]);
            }
        }
        grid->width--;

        // check against the next column
        i--;
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
surfaceGrid_t *G_CreateSurfaceGridMesh(
    int width, int height, const vec3_t ctrl[MAX_GRID_SIZE][MAX_GRID_SIZE], int *indexes, int numIndexes
)
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
            *vert = ctrl[i][j];
        }
    }

    for (i = 0; i < numIndexes; i++) {
        grid->getIndices()[i] = indexes[numIndexes - i - 1];
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

#if 1

surfaceGrid_t *
G_SubdividePatchToGrid(int width, int height, Vector points[MAX_PATCH_SIZE * MAX_PATCH_SIZE], float subdivisions)
{
    int     i, j;
    int     indexes[(MAX_GRID_SIZE - 1) * (MAX_GRID_SIZE - 1) * 2 * 3];
    int     numIndexes;
    cGrid_t grid;

    // build a grid
    grid.width      = width;
    grid.height     = height;
    grid.wrapWidth  = qfalse;
    grid.wrapHeight = qfalse;
    for (i = 0; i < width; i++) {
        for (j = 0; j < height; j++) {
            VectorCopy(points[j * width + i], grid.points[i][j]);
        }
    }

    // subdivide the grid
    G_SetGridWrapWidth(&grid);
    G_SubdivideGridColumns(&grid, subdivisions);
    G_RemoveDegenerateColumns(&grid);

    G_TransposeGrid(&grid);

    G_SetGridWrapWidth(&grid);
    G_SubdivideGridColumns(&grid, subdivisions);
    G_RemoveDegenerateColumns(&grid);

    // calculate indexes
    numIndexes = MakeMeshIndexes(grid.width, grid.height, indexes);

    return G_CreateSurfaceGridMesh(grid.width, grid.height, grid.points, indexes, numIndexes);
}

#else

/*
=================
G_SubdividePatchToGrid
=================
*/
surfaceGrid_t *
G_SubdividePatchToGrid(int width, int height, Vector points[MAX_PATCH_SIZE * MAX_PATCH_SIZE], float subdivisions)
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

            errorTable[dir][j + 1] = 1.0f / maxLen;

#    if 1
            if (maxLen <= subdivisions) {
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
#    endif
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

#    if 1
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
#    endif

    // calculate indexes
    numIndexes = MakeMeshIndexes(width, height, indexes);

    return G_CreateSurfaceGridMesh(width, height, ctrl, indexes, numIndexes);
}

#endif
